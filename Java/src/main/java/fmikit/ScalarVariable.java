/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

public class ScalarVariable {

	public String name;
	public String type;
	public String valueReference;
	public String startValue;
	public String description;
	public String causality;
	public String unit;
	public SimpleType declaredType;

	@Override
	public String toString() {
		return "ScalarVariable {" + 
				"\n          name: " + name + 
				"\n          type: " + type + 
				"\nvalueReference: " + valueReference +
				"\n    startValue: " + startValue + 
				"\n   description: " + description + 
				"\n     causality: " + causality +
				"\n          unit: " + unit + 
				"\n  declaredType: " + declaredType + 
				"\n}";
	}

}