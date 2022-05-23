import fmikit.ModelDescription;
import fmikit.ModelDescriptionReader;
import org.junit.Ignore;
import org.junit.Test;


@Ignore
public class DummyUnitTest {

    @Test
    public void testFMI1ME() throws Exception {
        String modelNames[] = {"BouncingBall", "Dahlquist", "Feedthrough", "Stair", "VanDerPol"};

        for (String modelName : modelNames) {
            System.out.println(modelName);
            String xmlfile = "E:\\Development\\Reference-FMUs\\" + modelName + "\\FMI1ME.xml";
            ModelDescriptionReader modelDescriptionReader = new ModelDescriptionReader(xmlfile);
            ModelDescription modelDescription = modelDescriptionReader.readModelDescription();
        }
    }

    @Test
    public void testFMI1CS() throws Exception {
        String modelNames[] = {"BouncingBall", "Dahlquist", "Feedthrough", "Resource", "Stair", "VanDerPol"};

        for (String modelName : modelNames) {
            System.out.println(modelName);
            String xmlfile = "E:\\Development\\Reference-FMUs\\" + modelName + "\\FMI1CS.xml";
            ModelDescriptionReader modelDescriptionReader = new ModelDescriptionReader(xmlfile);
            ModelDescription modelDescription = modelDescriptionReader.readModelDescription();
        }
    }

    @Test
    public void testFMI2() throws Exception {
        String modelNames[] = {"BouncingBall", "Dahlquist", "Feedthrough", "Resource", "Stair", "VanDerPol"};

        for (String modelName : modelNames) {
            System.out.println(modelName);
            String xmlfile = "E:\\Development\\Reference-FMUs\\" + modelName + "\\FMI2.xml";
            ModelDescriptionReader modelDescriptionReader = new ModelDescriptionReader(xmlfile);
            ModelDescription modelDescription = modelDescriptionReader.readModelDescription();
        }
    }

    @Test
    public void testFMI3() throws Exception {

        String modelNames[] = {"BouncingBall", "Clocks", "Dahlquist", "Feedthrough", "LinearTransform", "Resource", "Stair", "VanDerPol"};

        for (String modelName : modelNames) {
            System.out.println(modelName);
            String xmlfile = "E:\\Development\\Reference-FMUs\\" + modelName + "\\FMI3.xml";
            ModelDescriptionReader modelDescriptionReader = new ModelDescriptionReader(xmlfile);
            ModelDescription modelDescription = modelDescriptionReader.readModelDescription();
        }
    }

}
