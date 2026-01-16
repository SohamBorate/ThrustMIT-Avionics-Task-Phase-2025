#include "main.h"
#include "rylr.h"

/*
primary mosfet
redundant mosfet
rylr
micro sd card
pressure transducer
load cell
*/

/*
BOOT => waiting for initialisation of all systems
SAFE => wait for ignition
IGNITE => fire both mosfets
RECOVER => stop
FAILURE => something went wrong
*/

enum MACHINE_STATE {
    BOOT,
    SAFE,
    ARMED,
    IGNITE,
    FAILURE,
    RECOVER
};

MACHINE_STATE machine_state;

class PushButton;
void arm_btn_positive_edge(PushButton* btn);
void fire_btn_positive_edge(PushButton* btn);

//////////

class PushButton {
  public:
    String id;
    int pin;
    unsigned long db;
    int state = 0;
    unsigned long lastChange = 0;

    PushButton(String id2 = "", int pin2 = -1, int db2 = -1) {
      this->id = id2;
      this->pin = pin2;
      this->db = db2;
      pinMode(pin, INPUT);
    };

    void update(unsigned long currentTime) {
      if (currentTime - this->lastChange < this->db) {return;}
      int newState = digitalRead(this->pin);
      if (newState == this->state) {return;}
      int oldState = this->state;
      this->state = newState;
      this->lastChange = currentTime;
      if (newState > oldState) {
        if (positiveTriggered != nullptr) {positiveTriggered(this);}
      } else {
        if (negativeTriggered != nullptr) {negativeTriggered(this);}
      }
    };

    // function to be run when button pressed
    void (*positiveTriggered)(PushButton* btn) = nullptr;
    void (*negativeTriggered)(PushButton* btn) = nullptr;

    void positiveTriggerConnect(void (*invoke)(PushButton* btn)) {
      positiveTriggered = invoke;
    }
    void positiveTriggerDisconnect() {
      positiveTriggered = nullptr;
    }

    void negativeTriggerConnect(void (*invoke)(PushButton* btn)) {
      negativeTriggered = invoke;
    }
    void negativeTriggerDisconnect() {
      negativeTriggered = nullptr;
    }
};

//////////

const int dbTimer = 500;
const int no_of_btns = 2;
PushButton btns[no_of_btns];

//////////

void arm_btn_positive_edge(PushButton* btn) {
  if (machine_state != SAFE) {return;}
  machine_state = ARMED;
  Serial.println("ARMED");
  digitalWrite(yellow_led_pin, HIGH);
  digitalWrite(green_led_pin, LOW);
}

void fire_btn_positive_edge(PushButton* btn) {
  if (machine_state != ARMED) {return;}
  machine_state = IGNITE;
  Serial.println("IGNITE");
  rylr_send("IGNITE");
  digitalWrite(red_led_pin, HIGH);
  digitalWrite(yellow_led_pin, LOW);
}

//////////

void setup() {
    Serial.begin(115200);
    machine_state = BOOT;

    String id = "arm_btn";
    btns[0] = PushButton(id, arm_btn_pin, dbTimer);
    btns[0].positiveTriggerConnect(&arm_btn_positive_edge);

    id = "fire_btn";
    btns[1] = PushButton(id, fire_btn_pin, dbTimer);
    btns[1].positiveTriggerConnect(&fire_btn_positive_edge);

    pinMode(green_led_pin, OUTPUT);
    pinMode(yellow_led_pin, OUTPUT);
    pinMode(red_led_pin, OUTPUT);

    if (!rylr_setup()) {
        // sd_card_component_log("machine", "state change", "failure");
        Serial.println("rylr fail");
        return;
    }

    ////////// SAFE

    while (machine_state != SAFE) {
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        String data = rylr_receive_parse(rylr_receive());
        if (data == "SAFE") {
            machine_state = SAFE;
            // sd_card_component_log("machine", "state change", "ignite");
            Serial.println("test stand ready");
            digitalWrite(green_led_pin, HIGH);
        } else {
            Serial.print("rylr998 unknown data received:"); Serial.println(data);
        };
    }

    while (machine_state != IGNITE) {
        const unsigned long current = millis();
        for (int i = 0; i < no_of_btns; i++) {
            if (current - btns[i].lastChange < btns[i].db) {continue;}
            btns[i].update(current);
        }
    }

    // this pauses until data is received so no need for a loop
    String data = rylr_receive_parse(rylr_receive());
    while (data != "RECOVER" || data != "FAILURE") {
        Serial.print("rylr998 unexpected data received: "); Serial.println(data);
    }
    Serial.print("test stand status: "); Serial.println(data);

    ////////// RECOVER

    digitalWrite(red_led_pin, LOW);
}

void loop() {}
