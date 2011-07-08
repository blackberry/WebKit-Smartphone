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


        //Hit the back button a few times
        for (i in 0..9){
            press(back)
        }

        //click the search icon
        press(createPoint((int)width-30,80))
        
        //go somewhere
        press("http://blackberry.com")
        press(select)
        sleep(5)
        press(select)
        press(select)
       
    }
}
