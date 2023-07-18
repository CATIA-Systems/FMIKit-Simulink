package fmikit;

import fmikit.BuildDescription.BuildConfiguration;
import fmikit.BuildDescription.PreprocessorDefinition;
import fmikit.BuildDescription.SourceFileSet;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;
import java.io.File;
import java.net.URL;

public class BuildDescriptionReader {


    public static BuildDescription readBuildDescription(String filename) throws Exception {

        URL schemaUrl = BuildDescriptionReader.class.getResource("/schema/fmi3/fmi3BuildDescription.xsd");

        SchemaFactory factory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);

        Schema schema = factory.newSchema(schemaUrl);

        DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();

        dbf.setNamespaceAware(true);

        DocumentBuilder db = dbf.newDocumentBuilder();

        Document doc = db.parse(new File(filename));

        Validator validator = schema.newValidator();

        validator.validate(new DOMSource(doc));

        doc.getDocumentElement().normalize();

        Element root = doc.getDocumentElement();

        BuildDescription buildDescription = new BuildDescription();


        for (Element e : XmlUtil.elementsByTagName(root, "BuildConfiguration")) {

            BuildConfiguration buildConfiguration = new BuildConfiguration();

            buildConfiguration.modelIdentifier = e.getAttribute("modelIdentifier");

            for (Element f : XmlUtil.elementsByTagName(e, "SourceFileSet")) {

                SourceFileSet sourceFileSet = new SourceFileSet();

                sourceFileSet.language = f.getAttribute("language");

                buildConfiguration.sourceFileSets.add(sourceFileSet);

                for (Element g : XmlUtil.elementsByTagName(e, "SourceFile")) {
                    sourceFileSet.sourceFiles.add(g.getAttribute("name"));
                }

                for (Element g : XmlUtil.elementsByTagName(e, "PreprocessorDefinition")) {
                    PreprocessorDefinition preprocessorDefinition = new PreprocessorDefinition();
                    preprocessorDefinition.name = g.getAttribute("name");
                    preprocessorDefinition.value = g.getAttribute("value");
                    sourceFileSet.preprocessorDefinitions.add(preprocessorDefinition);
                }

                for (Element g : XmlUtil.elementsByTagName(e, "IncludeDirectory")) {
                    sourceFileSet.includeDirectories.add(g.getAttribute("name"));
                }

            }

            buildDescription.buildConfigurations.add(buildConfiguration);

        }

        return buildDescription;
    }

}
