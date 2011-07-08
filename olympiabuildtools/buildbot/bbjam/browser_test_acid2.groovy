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
        //Escape out of somewhere/anywhere/nowhere
        scroll(up,3000)
        press(createPoint(40,43))
        sleep(1)
        
        press("http://acid2.acidtests.org")
        press(select)
        sleep(10)
        press(createPoint(200,243))
    }
}
