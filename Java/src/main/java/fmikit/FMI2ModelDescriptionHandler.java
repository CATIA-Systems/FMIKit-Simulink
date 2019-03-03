/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

import fmikit.Dependency.DependencyKind;

public class FMI2ModelDescriptionHandler extends ModelDescriptionHandler {

	ScalarVariable sv;
	Map<ScalarVariable, List<Dependency>> unknowns;
	boolean skip = false;
	Implementation implementation;
	
	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		
		if (skip)
			return;

		if ("UnitDefinitions".equals(qName)) {
			
			skip = true;
			
		} else if (qName == "SimpleType") {

			simpleType = new SimpleType();

			simpleType.name = attributes.getValue("name");
			simpleType.description = attributes.getValue("description");

			modelDescription.typeDefinitions.put(simpleType.name, simpleType);

		} else if ("ScalarVariable".equals(qName)) {
			
			sv = new ScalarVariable();
			sv.name = attributes.getValue("name");
			modelDescription.scalarVariables.add(sv);
			sv.description = attributes.getValue("description");
			sv.causality = attributes.getValue("causality");
			sv.valueReference = attributes.getValue("valueReference");
			
			if ("output".equals(sv.causality)) {
				modelDescription.modelStructure.outputs.put(sv, null);
			}
			
		} else if ("Real".equals(qName) || "Integer".equals(qName) || "Enumeration".equals(qName) || "Boolean".equals(qName) || "String".equals(qName)) {
			
			if (sv != null) {
			
				sv.type = qName;
				
				String startValue = attributes.getValue("start");
				
				if (!"".equals(startValue)) {
					sv.startValue = startValue;
				}
				
				sv.unit = attributes.getValue("unit");
				
				String declaredType = attributes.getValue("declaredType");
				
				if (modelDescription.typeDefinitions.containsKey(declaredType)) {
					sv.declaredType = modelDescription.typeDefinitions.get(declaredType);
				}
				
			} else {
				
				simpleType.quantity = attributes.getValue("quantity");
				simpleType.unit = attributes.getValue("unit");
				simpleType.displayUnit = attributes.getValue("displayUnit");
				
			}
			
		} else if ("fmiModelDescription".equals(qName)) {
			
			modelDescription.fmiVersion = attributes.getValue("fmiVersion");
			modelDescription.modelName = attributes.getValue("modelName");
			modelDescription.guid = attributes.getValue("guid");
			modelDescription.description = attributes.getValue("description");
			modelDescription.author = attributes.getValue("author");
			modelDescription.version = attributes.getValue("version");
			modelDescription.generationTool = attributes.getValue("generationTool");
			modelDescription.generationDateAndTime = attributes.getValue("generationDateAndTime");
			modelDescription.variableNamingConvention = attributes.getValue("variableNamingConvention");
			
			String numberOfEventIndicators = attributes.getValue("numberOfEventIndicators");
			
			if (numberOfEventIndicators != null) {
				modelDescription.numberOfEventIndicators = Integer.parseInt(numberOfEventIndicators);
			}
			
		} else if ("ModelExchange".equals(qName)) {
			
			implementation = modelDescription.modelExchange = new ModelExchange();
			modelDescription.modelExchange.modelIdentifier = attributes.getValue("modelIdentifier");
			
		} else if ("CoSimulation".equals(qName)) {
			
			implementation = modelDescription.coSimulation = new CoSimulation();
			modelDescription.coSimulation.modelIdentifier = attributes.getValue("modelIdentifier");
			modelDescription.coSimulation.canInterpolateInputs = "true".equals(attributes.getValue("canInterpolateInputs"));
			
		} else if ("File".equals(qName)) {
			
			implementation.sourceFiles.add(attributes.getValue("name"));
			
		} else if ("ModelStructure".equals(qName)) {
			
			// do nothing
			
		} else if (qName == "Outputs") {
			
			unknowns = modelDescription.modelStructure.outputs;
			
		} else if (qName == "Derivatives") {
			
			unknowns = modelDescription.modelStructure.derivatives;
			
		} else if (qName == "InitialUnknowns") {
			
			unknowns = modelDescription.modelStructure.initialUnknowns;
			
		} else if (qName == "Unknown") {

			int index = Integer.parseInt(attributes.getValue("index"));
			
			ScalarVariable variable = modelDescription.scalarVariables.get(index - 1);
			ArrayList<Dependency> dependencies = null;
			
			String dependenciesValue = attributes.getValue("dependencies");
			String dependencyKinds = attributes.getValue("dependenciesKind");

			if (dependenciesValue != null && dependencyKinds != null) {

				dependencies = new ArrayList<Dependency>();
				String[] indexes = dependenciesValue.split(" ");
				String[] kinds = dependencyKinds.split(" ");

				for (int i = 0; i < indexes.length; i++) {

					if (indexes[i].isEmpty()) {
						continue;
					}

					int idx = Integer.parseInt(indexes[i]);
					String kind = kinds[i];

					Dependency dependency = new Dependency();
					dependency.variable = modelDescription.scalarVariables.get(idx - 1);

					if (kind.equals("dependent")) {
						dependency.kind = DependencyKind.DEPENDENT;
					} else if (kind.equals("constant")) {
						dependency.kind = DependencyKind.CONSTANT;
					} else if (kind.equals("fixed")) {
						dependency.kind = DependencyKind.FIXED;
					} else if (kind.equals("tunable")) {
						dependency.kind = DependencyKind.TUNABLE;
					} else if (kind.equals("discrete")) {
						dependency.kind = DependencyKind.DISCRETE;
					}

					dependencies.add(dependency);
				}
			}

			unknowns.put(variable, dependencies);
		}

	}

	@Override
	public void endElement(String uri, String localName, String qName) throws SAXException {
		
		if ("UnitDefinitions".equals(qName)) {
			skip = false;
		} else if ("ScalarVariable".equals(qName)) {
			sv = null;
		} else if ("SimpleType".equals(qName)) {
			simpleType = null;
		}
		
	}

}
