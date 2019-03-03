/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ModelDescription {

	public String fmiVersion;
	public String modelName;
	public String guid;

	public String description;
	public String author;
	public String version;
	public String generationTool;
	public String generationDateAndTime;

	public String variableNamingConvention;

	public int numberOfContinuousStates;
	public int numberOfEventIndicators;

	public List<ScalarVariable> scalarVariables = new ArrayList<ScalarVariable>();

	public CoSimulation coSimulation;
	public ModelExchange modelExchange;
	
	public Map<String, SimpleType> typeDefinitions = new HashMap<String, SimpleType>();
	
	public ModelStructure modelStructure;
	
	public ScalarVariable getScalarVariable(String variableName) {
		
		if (variableName == null) {
			return null;
		}
		
		for (ScalarVariable variable : scalarVariables) {
			if (variableName.equals(variable.name)) {
				return variable;
			}
		}
		
		return null;
	}

	@Override
	public String toString() {
		return 	"ModelDescription {" +
				"\n              fmiVersion: " + fmiVersion + 
				"\n               modelName: " + modelName + 
				"\n                    guid: " + guid +
				"\n             description: " + description + 
				"\n                  author: " + author + 
				"\n                 version: " + version + 
				"\n          generationTool: "	+ generationTool + 
				"\n   generationDateAndTime: " + generationDateAndTime + 
				"\nvariableNamingConvention: " + variableNamingConvention + 
				"\nnumberOfContinuousStates: " + numberOfContinuousStates +
				"\n numberOfEventIndicators: " + numberOfEventIndicators + 
				"\n         scalarVariables: <" + scalarVariables.size() + ">"+
				"\n            coSimulation: " + coSimulation + 
				"\n           modelExchange: " + modelExchange + 
				"\n         typeDefinitions: <" + typeDefinitions.size() + ">" + 
				"\n          modelStructure: " + modelStructure + 
				"\n}";
	}
	
}
