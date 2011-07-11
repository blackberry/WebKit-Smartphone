function getDeviceWidth() {
    switch(window.orientation) {
    case 0:
    case 180:
        return screen.width;
    case -90:
    case 90:
        // The height of the screen in landscape mode will be
        // equal to the width of the screen in portrait mode.
        return screen.height;
    default:
        return -1;
    }
}

function getDeviceHeight() {
    switch(window.orientation) {
    case 0:
    case 180:
        return screen.height;
    case -90:
    case 90:
        return screen.width;
    default:
        return -1;
    }
}

function getScale() {
    var scale = screen.width / window.innerWidth;
    return Math.round(scale * 1000) / 1000;
}

function logStart(logElement) {
    logElement.innerHTML = "";
}

function log(logElement, message) {
    logElement.innerHTML += "<p>" + message + "</p>";
}

function logResult(logElement, result) {
    logElement.innerHTML += "<ul>" + (result ? "<li>PASSED</li>" : "<li>FAILED</li>") + "</ul>";
}

function notifyDone() {
    if (window.layoutTestController)
        layoutTestController.notifyDone();
}

function logResultAndNotifyDone(logElement, result) {
    logResult(logElement, result);
    notifyDone();
}

function logResultAndNotifyDoneOnSuccess(logElement, result) {
    logResult(logElement, result);
    if (result)
        notifyDone();
}

function periodicUpdateDisplayForPreserveZoom() {
    var scale = getScale();

    var square = document.getElementById("square");
    square.innerHTML = "innerWidth: " + window.innerWidth;
    square.innerHTML += "<br>scale: " + scale;
    setTimeout(periodicUpdateDisplayForPreserveZoom, 500);
}
