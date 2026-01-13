#include <stdint.h>
#include "stm32f446xx.h"

#define CYCLES_PER_MS 16000
#define STK_CTRL_ENABLE (1U)
#define STK_CTRL_CLKSOURCE (1U << 2)
#define STK_CTRL_COUNTFLAG (1U << 16)

void SysTickDelayMS(int ms) {
    SysTick->LOAD = CYCLES_PER_MS - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = STK_CTRL_ENABLE | STK_CTRL_CLKSOURCE;
    for (int i = 0; i < ms; i++) {
        while ((SysTick->CTRL & STK_CTRL_COUNTFLAG) == 0) {}
    }
    SysTick->CTRL = 0;
}

int main() {
    // gpio a and c enable
    RCC->AHB1ENR |= (1U);
    RCC->AHB1ENR |= (1U << 2);

    // pin 5 general purpose output
    GPIOA->MODER |= (1U << 10);
    GPIOA->MODER &= ~(1U << 11);

    // pin 13 built-in push button
    GPIOC->MODER &= ~(1U << 26);
    GPIOC->MODER &= ~(1U << 27);

    while (1) {
        // XOR
        GPIOA->ODR ^= (1U << 5);
        SysTickDelayMS(1000);
    }
}
