const parameter_graphs = document.getElementsByClassName("parameter_graph");

var parameters = [];

for (let i = 0; i < parameter_graphs.length; i++) {
    var element = parameter_graphs[i];
    console.log(element.id, element.getAttribute("xaxis"), element.getAttribute("yaxis"));
    parameters[parameters.length] = {
        id: element.id,
        xaxis: element.getAttribute("xaxistype"),
        yaxis: element.getAttribute("yaxistype")
    };

    var trace = {
        x: [],
        y: [],
        type: "scatter"
    };

    var layout = {
        title: {text: element.id},
        xaxis: {
            title: {
                text: element.getAttribute("xaxislabel")
            },
        },
        yaxis: {
            title: {
                text: element.getAttribute("yaxislabel")
            },
        }
    };

    Plotly.newPlot(element.id, [trace], layout);
}

const socket = io();

socket.on(("serial_data"), (data) => {
    console.log(data);
    for (let i = 0; i < parameters.length; i++) {
        Plotly.extendTraces(parameters[i].id, {
            x: [data[parameters[i].xaxis]],
            y: [data[parameters[i].yaxis]]
        }, [0]);
    }
});
