function call(name, ...arguments) {
    const v = {
        name: "print",
        content: arguments,
    };

    window.webkit.messageHandlers.local.postMessage(v);
}

function print(string) {
    call("print", string);
}

//call("print", "My string");

onload = function () {
    print("onload");
    var list = this.document.getElementById("side-bar-list");

    var item = document.createElement("li");
    item.innerText = "custom generated baby";
    list.appendChild(item);
};
