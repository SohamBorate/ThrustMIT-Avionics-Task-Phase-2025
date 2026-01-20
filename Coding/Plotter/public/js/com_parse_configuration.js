let parameter_count = 1;

const no_parameters = document.getElementById("no_parameters");
no_parameters.value = 1;
const container = document.getElementById("com-parse-configs");
const clone_parent = document.getElementById("com-parse-config-form");

function update_attributes(el) {
    Array.from(el.attributes).forEach(attr => {
        if (attr.value.endsWith("_1")) {
            el.setAttribute(
                attr.name,
                `${attr.value.split("_")[0]}_${parameter_count}`
            );
        }
    });
}

document.getElementById("add-parameter-config").addEventListener("click", () => {
    parameter_count++;
    no_parameters.value = parameter_count;
    const clone = clone_parent.cloneNode(true);

    const heading = clone.querySelector("h3");
    if (heading) {
        heading.textContent = `Paramter ${parameter_count}`;
    }

    clone.querySelectorAll("*").forEach(el => update_attributes(el));
    update_attributes(clone);
    clone.querySelectorAll("input").forEach(input => input.value = "");
    container.appendChild(clone);
});
