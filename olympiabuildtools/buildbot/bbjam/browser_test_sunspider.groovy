public class TestClass extends net.rim.performance.test.execution.SimpleTest {

    // Needed by BB JAM internally - do not touch
    public TestClass(net.rim.performance.usecase.UseCase uc) {
        super(uc);
    }

    // setup() is called only once per test case and is called first.
    public void setup() {

    }

    // executeTest() is called once per iteration and is the body of the test.
    // If this method completes, the test is a pass.  If an exception is thrown
    // anywhere in the script, the test is a fail.
    public void executeTest() {
        scroll(up,3000)
        press(createPoint(40,43))
        press("http://www2.webkit.org/perf/sunspider-0.9/sunspider.html")
        press(select)
        sleep(10)
        press(createPoint(30,height))  //click somewhere on the screen
        scroll(down,1000)
        press(createPoint(30, height-90))
    }
}
