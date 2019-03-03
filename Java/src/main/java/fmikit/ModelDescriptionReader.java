/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import javax.xml.XMLConstants;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;

import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import fmikit.Dependency.DependencyKind;
import fmikit.ui.Util;

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
		ModelDescriptionHandler handler;

		if ("1.0".equals(fmiVersionHandler.fmiVersion)) {
			schemaUrl = ModelDescription.class.getResource("/schema/fmi1/fmiModelDescription.xsd");
			handler = new FMI1ModelDescriptionHandler();
		} else if ("2.0".equals(fmiVersionHandler.fmiVersion)) {
			schemaUrl = ModelDescription.class.getResource("/schema/fmi2/fmi2ModelDescription.xsd");
			handler = new FMI2ModelDescriptionHandler();
		} else {
			throw new Exception("Unsupported FMI version");
		}

		SchemaFactory schemaFactory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);

		Schema schema = schemaFactory.newSchema(schemaUrl);

		factory.setSchema(schema);
		factory.setNamespaceAware(true);

		SAXParser saxParser = factory.newSAXParser();

		saxParser.parse(createInputSource(), handler);

		messages = handler.getMessages();

		ModelDescription modelDescription = handler.getModelDescription();

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

		} else {

			modelDescription.numberOfContinuousStates = modelDescription.modelStructure.derivatives.size();

		}
		
		if (modelDescription.coSimulation != null) {
			checkPlatforms(modelDescription.coSimulation);
		}

		if (modelDescription.modelExchange != null) {
			checkPlatforms(modelDescription.modelExchange);
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

}
