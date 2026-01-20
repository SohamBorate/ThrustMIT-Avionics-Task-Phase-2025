let plot_count = 1;

const no_plots = document.getElementById("no_plots");
no_plots.value = 1;
const container = document.getElementById("com-plot-configs");
const clone_parent = document.getElementById("com-plot-config-form");

function update_attributes(el) {
    Array.from(el.attributes).forEach(attr => {
        if (attr.value.endsWith("_1")) {
            el.setAttribute(
                attr.name,
                `${attr.value.split("_")[0]}_${plot_count}`
            );
        }
    });
}

document.getElementById("add-plot-config").addEventListener("click", () => {
    plot_count++;
    no_plots.value = plot_count;
    const clone = clone_parent.cloneNode(true);

    const heading = clone.querySelector("h3");
    if (heading) {
        heading.textContent = `Plot ${plot_count}`;
    }

    clone.querySelectorAll("*").forEach(el => update_attributes(el));
    update_attributes(clone);
    clone.querySelectorAll("input").forEach(input => input.value = "");
    container.appendChild(clone);
});
