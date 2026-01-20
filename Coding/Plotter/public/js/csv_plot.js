const parameter_graphs = document.getElementsByClassName("parameter_graph");

d3.csv(`/uploads/${document.getElementById("csv-source").innerHTML}`).then(rows => {
    for (let i = 0; i < parameter_graphs.length; i++) {
        var element = parameter_graphs[i];
        console.log(
            element.id,
            element.getAttribute("xaxistype"),
            element.getAttribute("xaxislabel"),
            element.getAttribute("yaxistype"),
            element.getAttribute("yaxislabel")
        );

        const x = rows.map(r => Number(r[element.getAttribute("xaxistype")]));
        const y = rows.map(r => Number(r[element.getAttribute("yaxistype")]));

        const trace = {
            x,
            y,
            type: "scatter",
            mode: "lines",
        };

        Plotly.newPlot(element.id, [trace], {
            title: { text: element.id },
            xaxis: { title: { text: element.getAttribute("xaxislabel") } },
            yaxis: { title: { text: element.getAttribute("yaxislabel") } }
        });
    }
});
