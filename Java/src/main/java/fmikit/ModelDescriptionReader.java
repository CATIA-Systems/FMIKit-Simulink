/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import fmikit.Dependency.DependencyKind;
import fmikit.ui.Util;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.net.URL;
import java.util.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class ModelDescriptionReader {

	public List<String> messages = new ArrayList<String>();

	private String filename;

	public ModelDescriptionReader(String filename) {
		this.filename = filename;
	}

	public static class FMIVersionHandler extends DefaultHandler {

		public String fmiVersion = null;

		public void startElement(String uri, String localName, String qName, Attributes attributes)
				throws SAXException {

			if ("fmiModelDescription".equals(qName)) {
				fmiVersion = attributes.getValue("fmiVersion");
				throw new SAXException("done");
			}
		}

	}

	protected InputSource createInputSource() throws Exception {
		return new InputSource(new FileReader(filename));
	}
	
	protected boolean containsFile(String first, String... more) {
		File modelDescriptionFile = new File(filename);
		String dir = Util.joinPath(modelDescriptionFile.getParent(), first);
		return Util.isFile(dir, more);
	}

	public ModelDescription readModelDescription() throws Exception {

		SAXParserFactory factory = SAXParserFactory.newInstance();
		SAXParser parser = factory.newSAXParser();

		FMIVersionHandler fmiVersionHandler = new FMIVersionHandler();

		try {
			parser.parse(createInputSource(), fmiVersionHandler);
		} catch (SAXException e) {
			// do nothing
		}

		URL schemaUrl = null;
		ModelDescriptionHandler handler = null;
		ModelDescription modelDescription = null;

		if ("1.0".equals(fmiVersionHandler.fmiVersion)) {
			schemaUrl = ModelDescription.class.getResource("/schema/fmi1/fmiModelDescription.xsd");
			handler = new FMI1ModelDescriptionHandler();
		} else if ("2.0".equals(fmiVersionHandler.fmiVersion)) {
			schemaUrl = ModelDescription.class.getResource("/schema/fmi2/fmi2ModelDescription.xsd");
			handler = new FMI2ModelDescriptionHandler();
		} else if ("3.0-beta.5".equals(fmiVersionHandler.fmiVersion)) {
			modelDescription = readModelDescription3(filename);
		} else {
			throw new Exception("Unsupported FMI version");
		}

		if (modelDescription == null) {
			SchemaFactory schemaFactory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
			Schema schema = schemaFactory.newSchema(schemaUrl);
			factory.setSchema(schema);
			factory.setNamespaceAware(true);
			SAXParser saxParser = factory.newSAXParser();
			saxParser.parse(createInputSource(), handler);
			messages = handler.getMessages();
			modelDescription = handler.getModelDescription();
		}

		if ("1.0".equals(modelDescription.fmiVersion)) {

			// build the model structure
			FMI1ModelDescriptionHandler h = (FMI1ModelDescriptionHandler) handler;

			for (Map.Entry<ScalarVariable, List<String>> entry : h.directDependencies.entrySet()) {

				ScalarVariable variable = entry.getKey();
				List<Dependency> dependencies = null;

				List<String> inputNames = entry.getValue();

				if (inputNames != null) {

					dependencies = new ArrayList<Dependency>();

					for (String name : inputNames) {
						Dependency dependency = new Dependency();
						dependency.variable = h.variables.get(name);
						dependency.kind = DependencyKind.DEPENDENT;
						dependencies.add(dependency);
					}
				}

				modelDescription.modelStructure.outputs.put(variable, dependencies);
			}
			
			// for FMI 1.0 c-code is only supported for Dymola
			if (modelDescription.generationTool != null && modelDescription.generationTool.startsWith("Dymola")) {	
				
				if (containsFile("sources", "all.c")) {
					
					if (modelDescription.coSimulation != null) {
						modelDescription.coSimulation.sourceFiles.add("all.c");
					}
					
					if (modelDescription.modelExchange != null) {
						modelDescription.modelExchange.sourceFiles.add("all.c");
					}
				}
				
			}

		} else if ("2.0".equals(modelDescription.fmiVersion)) {

			modelDescription.numberOfContinuousStates = modelDescription.modelStructure.derivatives.size();

		}
		
		if (modelDescription.coSimulation != null) {
			checkPlatforms(modelDescription.coSimulation);
		}

		if (modelDescription.modelExchange != null) {
			checkPlatforms(modelDescription.modelExchange);
		}

		HashSet<Character> c1 = new HashSet<Character>();

		c1.add('_');
		for (char c = 'A'; c < 'Z'; c++) c1.add(c);
		for (char c = 'a'; c < 'z'; c++) c1.add(c);

		HashSet<Character> c2 = new HashSet<Character>(c1);
		for (char c = '0'; c < '9'; c++) c2.add(c);

		HashSet<String> dialogParameters = new HashSet<String>();

		// generate dialog parameter names
		for (ScalarVariable variable : modelDescription.scalarVariables) {

			char[] chars = variable.name.toCharArray();

			// replace special characters with '_'
			for (int i = 0; i < chars.length; i++) {
				if ((i == 0 && !c1.contains(chars[i])) || (i > 0 && !c2.contains(chars[i]))) {
					chars[i] = '_';
				}
			}

			String dialogParameter = new String(chars);

			// stay under MATLAB's variable length limit of 63 characters
			if (dialogParameter.length() > 55) {
				dialogParameter = dialogParameter.substring(0, 55);
			}

			// make name unique
			if (dialogParameters.contains(dialogParameter)) {
				int i = 2;
				while (dialogParameters.contains(dialogParameter + "_" + Integer.toString(i))) {
					i++;
				}
				dialogParameter = dialogParameter + "_"  + Integer.toString(i);
			}

			variable.dialogParameter = dialogParameter;
			dialogParameters.add(dialogParameter);
		}

		return modelDescription;
	}

	public static ModelDescription readFromFMU(String filename) throws Exception {

		final ZipFile zipFile = new ZipFile(filename);

		final ZipEntry entry = zipFile.getEntry("modelDescription.xml");

		ModelDescription modelDescription = null;

		if (entry != null) {

			ModelDescriptionReader modelDescriptionReader = new ModelDescriptionReader(null) {

				protected InputSource createInputSource() throws IOException {
					return new InputSource(zipFile.getInputStream(entry));
				}

				protected boolean containsFile(String first, String... more) {
					return zipFile.getEntry(Util.join("/", first, more)) != null || zipFile.getEntry(Util.join("\\", first, more)) != null;
				}
				
			};

			modelDescription = modelDescriptionReader.readModelDescription();

		}

		zipFile.close();

		return modelDescription;
	}

	private void checkPlatforms(Implementation implementation) {
		
		if (!implementation.sourceFiles.isEmpty()) {
			
			boolean hasSources = true;
			
//			for (String sourceFile : implementation.sourceFiles) {
//				if (!containsFile("sources", sourceFile)) {
//					hasSources = false;
//					break;
//				}
//			}
			
			if (hasSources) {
				implementation.platforms.add("c-code");
			}
			
		}

		if (containsFile("binaries", "darwin64", implementation.modelIdentifier + ".dylib")) {
			implementation.platforms.add("darwin64");
		}
		
		if (containsFile("binaries", "linux32", implementation.modelIdentifier + ".so")) {
			implementation.platforms.add("linux32");
		}
		
		if (containsFile("binaries", "linux64", implementation.modelIdentifier + ".so")) {
			implementation.platforms.add("linux64");
		}
		
		if (containsFile("binaries", "win32", implementation.modelIdentifier + ".dll")) {
			implementation.platforms.add("win32");
		}
		
		if (containsFile("binaries", "win64", implementation.modelIdentifier + ".dll")) {
			implementation.platforms.add("win64");
		}
		
	}

	ModelDescription readModelDescription3(final String xmlfile) throws Exception {

		SchemaFactory factory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);

		URL schemaUrl = ModelDescription.class.getResource("/schema/fmi3/fmi3ModelDescription.xsd");

		Schema schema = factory.newSchema(schemaUrl);

		File file = new File(xmlfile);

		DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();

		dbf.setNamespaceAware(true);

		DocumentBuilder db = dbf.newDocumentBuilder();

		Document doc = db.parse(file);

		Validator validator = schema.newValidator();

		validator.validate(new DOMSource(doc));

		doc.getDocumentElement().normalize();

		ModelDescription modelDescription = new ModelDescription();

		Element documentElement = doc.getDocumentElement();

		modelDescription.fmiVersion = documentElement.getAttribute("fmiVersion");
		modelDescription.modelName = documentElement.getAttribute("modelName");
		modelDescription.guid = documentElement.getAttribute("instantiationToken");
		modelDescription.description = documentElement.getAttribute("description");
		modelDescription.author = documentElement.getAttribute("author");
		modelDescription.version = documentElement.getAttribute("version");
		modelDescription.generationTool = documentElement.getAttribute("generationTool");
		modelDescription.generationDateAndTime = documentElement.getAttribute("generationDateAndTime");

		// Model Exchange
		Element modelExchange = XmlUtil.getChildElement(documentElement, "ModelExchange");

		if (modelExchange != null) {
			modelDescription.modelExchange = new ModelExchange();
			modelDescription.modelExchange.modelIdentifier = modelExchange.getAttribute("modelIdentifier");
		}

		// Co-Simulation
		Element coSimulation = XmlUtil.getChildElement(documentElement, "CoSimulation");

		if (coSimulation != null) {
			modelDescription.coSimulation = new CoSimulation();
			modelDescription.coSimulation.modelIdentifier = coSimulation.getAttribute("modelIdentifier");
		}

		// DefaultExperiment
		Element item = XmlUtil.getChildElement(documentElement, "DefaultExperiment");

		if (item != null) {
			DefaultExperiment experiment = new DefaultExperiment();

			experiment.startTime = item.getAttribute("startTime");
			experiment.stopTime = item.getAttribute("stopTime");
			experiment.tolerance = item.getAttribute("tolerance");

			modelDescription.defaultExperiment = experiment;
		}

		// type definitions
		Element typeDefinitions = XmlUtil.getChildElement(documentElement, "TypeDefinitions");

		if (typeDefinitions != null) {

			for (Element element : XmlUtil.asElementList(typeDefinitions.getChildNodes())) {

				SimpleType simpleType = new SimpleType();

				simpleType.name = element.getAttribute("name");
				simpleType.description = element.getAttribute("description");
				simpleType.quantity = element.getAttribute("quantity");
				simpleType.unit = element.getAttribute("unit");
				simpleType.displayUnit = element.getAttribute("displayUnit");
				simpleType.relativeQuantity = element.getAttribute("relativeQuantity");
				simpleType.min = element.getAttribute("min");
				simpleType.max = element.getAttribute("max");
				simpleType.nominal = element.getAttribute("nominal");

				modelDescription.typeDefinitions.put(simpleType.name, simpleType);
			}
		}

		// Model Variables
		Element modelVariablesElement = XmlUtil.getChildElement(documentElement, "ModelVariables");

		HashMap<String, ScalarVariable> variables = new HashMap<String, ScalarVariable>();

		for (Element element : XmlUtil.asElementList(modelVariablesElement.getChildNodes())) {

			ScalarVariable scalarVariable = new ScalarVariable();
			scalarVariable.name = element.getAttribute("name");
			scalarVariable.type = element.getTagName();
			scalarVariable.valueReference = element.getAttribute("valueReference");
			scalarVariable.startValue = element.getAttribute("start");
			scalarVariable.description = element.getAttribute("description");
			scalarVariable.causality = element.getAttribute("causality");
			scalarVariable.unit = element.getAttribute("unit");
			scalarVariable.derivative = element.getAttribute("derivative");

			if (element.hasAttribute("declaredType")) {
				String declaredType = element.getAttribute("declaredType");
				scalarVariable.declaredType = modelDescription.typeDefinitions.get(declaredType);
			}

			modelDescription.scalarVariables.add(scalarVariable);

			for (Element dimensionElement : XmlUtil.asElementList(element.getChildNodes())) {
				if (dimensionElement.hasAttribute("start")) {
					String start = dimensionElement.getAttribute("start");
					scalarVariable.dimensions.add(new Integer(start));
				} else {
					scalarVariable.dimensions.add(dimensionElement.getAttribute("valueReference"));
				}
			}

			variables.put(scalarVariable.valueReference, scalarVariable);
		}

		// Dimensions
		for (ScalarVariable variable : variables.values()) {
			for (int i = 0; i < variable.dimensions.size(); i++) {
				Object value = variable.dimensions.get(i);
				if (value instanceof String) {
					ScalarVariable v = variables.get(value);
					variable.dimensions.set(i, v);
				}
			}
		}

		// model structure
		Element modelStructureElement = XmlUtil.getChildElement(documentElement, "ModelStructure");

		for (Element element : XmlUtil.asElementList(modelStructureElement.getChildNodes())) {

			String valueReference = element.getAttribute("valueReference");
			ScalarVariable variable = modelDescription.getScalarVariableByValueReference(valueReference);

			String tagName = element.getTagName();

			List<Dependency> dependencies = Collections.<Dependency>emptyList();

			if ("Output".equals(tagName)) {
				modelDescription.modelStructure.outputs.put(variable, dependencies);
			} else if ("ContinuousStateDerivative".equals(tagName)) {
				modelDescription.modelStructure.derivatives.put(variable, dependencies);
				ScalarVariable continuousState = variables.get(variable.derivative);
				modelDescription.numberOfContinuousStates += calculateInitialSize(continuousState);
			} else if ("InitialUnknown".equals(tagName)) {
				modelDescription.modelStructure.initialUnknowns.put(variable, dependencies);
			} else if ("EventIndicator".equals(tagName)) {
				ScalarVariable eventIndicator = variables.get(variable.derivative);
				modelDescription.numberOfEventIndicators += calculateInitialSize(eventIndicator);;
			}

		}

		return modelDescription;
	}

	private int calculateInitialSize(ScalarVariable continuousState) {
		int size = 1;  // initial size

		for (Object dimension : continuousState.dimensions) {
			if (dimension instanceof Integer) {
				size *= (Integer)dimension;
			} else {
				ScalarVariable dimensionVariable = (ScalarVariable) dimension;
				size *= Integer.parseInt(dimensionVariable.startValue);
			}
		}
		return size;
	}

}
