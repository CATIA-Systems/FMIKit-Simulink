/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.util.ArrayList;
import java.util.List;

public class ScalarVariable {

	public String name;
	public String type;
	public String valueReference;
	public String startValue;
	public String description;
	public String causality;
    public String variability;
	public String unit;
	public SimpleType declaredType;
	public String derivative;

	/** List of fixed dimensions (Integer) or variables (ScalarVariable) that hold the dimensions */
	public List<Object> dimensions = new ArrayList<Object>();

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