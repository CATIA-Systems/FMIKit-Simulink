/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;

public class FMI1ModelDescriptionHandler extends ModelDescriptionHandler {

	ScalarVariable sv;
	int unknowns = 0;
	StringBuffer text = new StringBuffer(1024);
	
	Map<ScalarVariable, List<String>> directDependencies = new HashMap<ScalarVariable, List<String>>();
	Map<String, ScalarVariable> variables = new HashMap<String, ScalarVariable>();
	
	@Override
	public void characters (char ch[], int start, int length) throws SAXException {
	     //already synchronized
	    text.append(ch, start, length);
	}

	public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
		
		if ("TypeDefinitions".equals(qName)) {

			// do nothing

		} else if ("Type".equals(qName)) {

			simpleType = new SimpleType();

			simpleType.name = attributes.getValue("name");
			simpleType.description = attributes.getValue("description");

			modelDescription.typeDefinitions.put(simpleType.name, simpleType);

			
		} else if ("RealType".equals(qName)) {
			
			simpleType.quantity = attributes.getValue("quantity");
			simpleType.unit = attributes.getValue("unit");
			simpleType.displayUnit = attributes.getValue("displayUnit");
			simpleType.relativeQuantity = attributes.getValue("relativeQuantity");
			simpleType.min = attributes.getValue("min");
			simpleType.max = attributes.getValue("max");
			simpleType.nominal = attributes.getValue("nominal");

		} else if ("IntegerType".equals(qName) || "EnumerationType".equals(qName)) {
			
			simpleType.quantity = attributes.getValue("quantity");
			simpleType.min = attributes.getValue("min");
			simpleType.max = attributes.getValue("max");

		} else if ("ScalarVariable".equals(qName)) {
			
			sv = new ScalarVariable();
			sv.name = attributes.getValue("name");
			modelDescription.scalarVariables.add(sv);
			sv.description = attributes.getValue("description");
			sv.causality = attributes.getValue("causality");
			sv.valueReference = attributes.getValue("valueReference");
			
			if ("output".equals(sv.causality)) {
				directDependencies.put(sv, null);
			}
			
			variables.put(sv.name, sv);
			
		} else if ("Real".equals(qName) || "Integer".equals(qName) || "Enumeration".equals(qName) || "Boolean".equals(qName) || "String".equals(qName)) {
			
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
			
		} else if ("DirectDependency".equals(qName)) {
			
			ScalarVariable variable = modelDescription.scalarVariables.get(modelDescription.scalarVariables.size() - 1);
			directDependencies.put(variable, new ArrayList<String>());
			
		} else if ("Name".equals(qName)) {
			
			// do nothing
			
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
			modelDescription.numberOfContinuousStates = Integer.parseInt(attributes.getValue("numberOfContinuousStates"));
			modelDescription.numberOfEventIndicators = Integer.parseInt(attributes.getValue("numberOfEventIndicators"));

			modelDescription.modelExchange = new ModelExchange();
			modelDescription.modelExchange.modelIdentifier = attributes.getValue("modelIdentifier");
		
		} else if ("Unknown".equals(qName)) {
		
			unknowns++;
		
		} else if ("Derivatives".equals(qName)) {
		
			modelDescription.numberOfContinuousStates = unknowns;
			unknowns = 0;
		
		} else if ("Outputs".equals(qName) || "InitialUnknowns".equals(qName)) {
			
			unknowns = 0;
			
		} else if ("Implementation".equals(qName)) {
			
			modelDescription.coSimulation = new CoSimulation();
			modelDescription.coSimulation.modelIdentifier = modelDescription.modelExchange.modelIdentifier;
			modelDescription.modelExchange = null;
			
		} else if ("Capabilities".equals(qName)) {
			
			modelDescription.coSimulation.canInterpolateInputs = "true".equals(attributes.getValue("canInterpolateInputs"));
			
		} else if ("Capabilities".equals(qName)) {
			
			modelDescription.coSimulation.canInterpolateInputs = "true".equals(attributes.getValue("canInterpolateInputs"));
			
		}

	}
	
	@Override
	public void endElement (String uri, String localName, String qName) throws SAXException {
	    if(qName.equals("Name")){
	        String name = text.toString();
	        
	        ScalarVariable variable = modelDescription.scalarVariables.get(modelDescription.scalarVariables.size() - 1);
			//List<Dependency> dependencies = modelDescription.modelStructure.outputs.get(variable);
			
	        List<String> list = directDependencies.get(variable);
			
	        list.add(name);
	        
			// clear the buffer
			text.setLength(0);
	    }
	}
	
}