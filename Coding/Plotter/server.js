const express = require("express");
const http = require("node:http");
const fs = require("node:fs");
const multer = require("multer");
const path = require("path");
const {parse} = require("csv-parse/sync")
const { Server } = require("socket.io");
const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");
const { title } = require("node:process");

//////////////////////////////
////////// preliminary
//////////////////////////////

const parameters = [
    // {
    //     title: "Thrust",
    //     xaxis: "Seconds",
    //     yaxis: "Newtons"
    // }, {
    //     title: "Chamber Pressure",
    //     xaxis: "Seconds",
    //     yaxis: "Pascals"
    // }
];

var csv_source = "";
var csv = undefined;
var com = undefined;
var baud_rate = undefined;
var com_parser = undefined;
// spamming plotly leads to lot of lag and just overall bad real time plotting
const com_db = 100;

var com_separator = ",";
const com_parse_parameters = [
    // {
    //     id: "time",
    //     index: 0,
    //     conversion: "timestamp_usec_to_sec"
    // }, {
    //     id: "thrust",
    //     index: 1,
    //     conversion: "loadcell_adc_to_newton"
    // }, {
    //     id: "pressure",
    //     index: 2,
    //     conversion: "pressure_adc_to_pascal"
    // }
];

//////////////////////////////
////////// conversion functions
//////////////////////////////

/*
assuming

MCU ANALOG INPUT PIN MAX: 3.3 V (voltage divider will be used)
MCU 12 BIT ADC: 0-4095
*/
const adc_max = 4095

/*
assuming

TE Connectivity FC2311-0000-1000-L LOAD CELL
Range: 453.59 kg (1000 lbf)
Voltage Output: 10 V dc
*/
const thrust_range_newtons = 453.59 * 9.81 // SI units

/*
assuming

Honeywell TJE Pressure Transducer
RANGE CODE: DT (7500 psi)
VOLTAGE OUTPUT OPTION: 2t (0-10 V dc)
*/
const pressure_range_pascals = 7500 * 6894.76 // SI units

const conversion_funcs = {};
conversion_funcs["none"] = (val) => {
    return val;
}

conversion_funcs["loadcell_adc_to_newton"] = (adc) => {
    return (adc / adc_max) * thrust_range_newtons;
}

conversion_funcs["pressure_adc_to_pascal"] = (adc) => {
    return (adc / adc_max) * pressure_range_pascals;
}

conversion_funcs["timestamp_usec_to_sec"] = (time) => {
    return time / 1000000;
}

conversion_funcs["timestamp_msec_to_sec"] = (time) => {
    return time / 1000;
}

//////////////////////////////
////////// web server
//////////////////////////////

const app = express();
app.set("view engine", "ejs");
app.use(express.static("public"));
app.use(express.urlencoded({ extended: true }));
const server = http.createServer(app);

app.get("/", (req, res) => {
    res.render("index", {parameters: parameters});
});

//////////////////////////////
////////// web socket
//////////////////////////////

const io = new Server(server);

io.on("connection", (socket) => {
    console.log("a client connected");
    socket.on("disconnect", () => {
        console.log("a client disconnected");
    });
});

//////////////////////////////
////////// csv upload
//////////////////////////////

app.get("/csv", (req, res) => {
    res.render("csv");
});

const storage = multer.diskStorage({
    destination: "public/uploads/",
    filename: (req, file, cb) => {
        cb(null, file.originalname);
    }
});

const upload = multer({ storage });

app.post("/csv_upload", upload.single("datafile"), (req, res) => {
    if (req.file.mimetype !== "text/csv") {
        res.send("invalid file type, stop");
        return;
    }
    csv_source = req.file.originalname;
    csv = parse(
        fs.readFileSync(path.join(__dirname, "public", "uploads", req.file.originalname))
    );
    console.log("csv uploaded");
    res.redirect("csv_configuration");
});

//////////////////////////////
////////// csv configuration
//////////////////////////////

app.get("/csv_configuration", (req, res) => {
    res.render("csv_configuration", {
        headers: csv[0]
    });
});

app.post("/csv_configuration", (req, res) => {
    var no_plots = Number(req.body.no_plots);
    for (let i = 0; i < no_plots; i++) {
        parameters[i] = {
            title: req.body[`title_${i + 1}`],
            xaxislabel: req.body[`xaxislabel_${i + 1}`],
            xaxistype: req.body[`xaxistype_${i + 1}`],
            yaxislabel: req.body[`yaxislabel_${i + 1}`],
            yaxistype: req.body[`yaxistype_${i + 1}`],
        }
    }
    console.log(parameters);
    res.redirect("/csv_plot");
});

app.get("/csv_plot", (req, res) => {
    res.render("csv_plot", {parameters: parameters, csv_source: csv_source});
})

//////////////////////////////
////////// com connect
//////////////////////////////

app.get("/com", (req, res) => {
    SerialPort.list()
    .then(ports => {
        console.log(ports);
        res.render("com", { ports: ports.map(port => port.path) });
    })
});

app.post("/com_connect", (req, res) => {
    console.log(req.body);
    res.redirect("/com_parse_configuration");
    com = req.body.port;
    baud_rate = Number(req.body.baud_rate);
});

//////////////////////////////
////////// com parse configuration
//////////////////////////////

app.get("/com_parse_configuration", (req, res) => {
    res.render("com_parse_configuration", { conversion_funcs: Object.keys(conversion_funcs).map( key => key )});
});

app.post("/com_parse_configuration", (req, res) => {
    var no_parameters = Number(req.body.no_parameters);
    com_separator = req.body.separator;
    for (let i = 0; i < no_parameters; i++) {
        com_parse_parameters[i] = {
            id: req.body[`id_${i + 1}`],
            index: req.body[`index_${i + 1}`],
            conversion: req.body[`conversion_${i + 1}`]
        }
    }
    console.log(com_parse_parameters);
    res.redirect("/com_plot_configuration");
});

//////////////////////////////
////////// com plot configuration
//////////////////////////////

app.get("/com_plot_configuration", (req, res) => {
    res.render("com_plot_configuration", { headers: com_parse_parameters.map(parameter => parameter.id) });
});

app.post("/com_plot_configuration", (req, res) => {
    var no_plots = Number(req.body.no_plots);
    for (let i = 0; i < no_plots; i++) {
        parameters[i] = {
            title: req.body[`title_${i + 1}`],
            xaxislabel: req.body[`xaxislabel_${i + 1}`],
            xaxistype: req.body[`xaxistype_${i + 1}`],
            yaxislabel: req.body[`yaxislabel_${i + 1}`],
            yaxistype: req.body[`yaxistype_${i + 1}`],
        }
    }
    console.log(parameters);
    res.redirect("/com_plot");
});

//////////////////////////////
////////// com plot :D
//////////////////////////////

var send = true;

app.get("/com_plot", (req, res) => {
    port = new SerialPort({ 
        path: com, 
        baudRate: baud_rate 
    });
    // The parser looks for a "newline" character to know when a message ends
    com_parser = port.pipe(new ReadlineParser({ delimiter: "\r\n" }));

    com_parser.on("data", (com_data) => {
        // console.log(com_data, send);
        if (!send) {return;}
        // com_data =>
        // microseconds
        // thrust_adc
        // pressure_adc
        const split = com_data.split(com_separator);
        if (split.length != com_parse_parameters.length) {return;}
        send = false;
        const ws_data = {};

        for (let i = 0; i < com_parse_parameters.length; i++) {
            ws_data[com_parse_parameters[i].id] = [
                (conversion_funcs[com_parse_parameters[i].conversion])( Number(split[com_parse_parameters[i].index]) )
            ];
        }

        io.emit("serial_data", ws_data);
        console.log(ws_data);
        setTimeout(() => {
            send = true;
        }, com_db);
    });

    res.render("com_plot", {  parameters: parameters });
})

//////////////////////////////
////////// initialise http server
//////////////////////////////

server.listen(3000, () => {
    console.log("server running at http://localhost:3000");
});
