public class TestClass extends net.rim.performance.test.execution.SimpleTest {

    // Needed by BB JAM internally - do not touch
    public TestClass(net.rim.performance.usecase.UseCase uc) {
        super(uc);
    }

    // executeTest() is called once per iteration and is the body of the test.
    // If this method completes, the test is a pass.  If an exception is thrown
    // anywhere in the script, the test is a fail.
    public void executeTest() {

        press(select);

    }
}
