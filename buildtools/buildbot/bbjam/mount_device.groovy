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
        for (i in 0..9){
            press(back)
        }

        //Open the options
        press(createPoint(0,0))
        sleep(3)
        scroll(down, 30);
        press(select)

        //Select Device
        sleep(3)
        scroll(down,4)
        press(select)

        //Select Storage
        sleep(3)
        scroll(down,7)
        press(select)

        //select [Enable USB Mass Storage]
        sleep(3)
        press(menu)
        scroll(up,3)
        press(select)
    }
}
