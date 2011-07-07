description("Tests that when Geolocation permission has been denied prior to a call to a Geolocation method, the error callback is invoked with code PERMISSION_DENIED, when the Geolocation service encounters an error.");

// Prime the Geolocation instance by denying permission.
window.layoutTestController.setGeolocationPermission(false);
window.layoutTestController.setMockGeolocationPosition(51.478, -0.166, 100);

var error;
navigator.geolocation.getCurrentPosition(function(p) {
    testFailed('Success callback invoked unexpectedly');
    finishJSTest();
}, function(e) {
    error = e
    shouldBe('error.code', 'error.PERMISSION_DENIED');
    shouldBe('error.message', '"User denied Geolocation"');
    debug('');
    continueTest();
});

function continueTest()
{
    // Make another request, with permission already denied.
    window.layoutTestController.setMockGeolocationError(0, 'test');

    navigator.geolocation.getCurrentPosition(function(p) {
        testFailed('Success callback invoked unexpectedly');
        finishJSTest();
    }, function(e) {
        error = e
        shouldBe('error.code', 'error.PERMISSION_DENIED');
        shouldBe('error.message', '"User denied Geolocation"');
        finishJSTest();
    });
}
window.layoutTestController.waitUntilDone();

window.jsTestIsAsync = true;
window.successfullyParsed = true;