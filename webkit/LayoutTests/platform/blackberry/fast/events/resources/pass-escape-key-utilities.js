if (window.layoutTestController)
    layoutTestController.dumpAsText();

function testEscapeKeyCaptured(element)
{
    var escapeCharCode = 27;
    element.onkeydown = function(e) {
        if (e.keyCode === escapeCharCode) // Note, the key code is equal to the char code for the ASCII escape character.
            log("PASS keyCode is " + escapeCharCode + ".");
        else
            log("FAIL keyCode should be " + escapeCharCode + ". Was " + e.keyCode + ".");
    }
    if (window.eventSender) {
        eventSender.keyDown(String.fromCharCode(escapeCharCode));
        eventSender.leapForward(300); // From observation, a reasonable wait time.
    }
}

function log(message)
{
    document.getElementById("console").appendChild(document.createTextNode(message + "\n"));
}
