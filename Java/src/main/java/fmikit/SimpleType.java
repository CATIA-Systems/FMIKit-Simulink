/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

public class SimpleType {

	public String name;
	public String description;
	public String quantity;
	public String unit;
	public String displayUnit;
	public String relativeQuantity;
	public String min;
	public String max;
	public String nominal;
	
	@Override
	public String toString() {
		return "SimpleType {name: " + name + ", description: " + description + ", quantity: " + quantity + ", unit: " + unit
				+ ", displayUnit: " + displayUnit + ", relativeQuantity: " + relativeQuantity + ", min: " + min + ", max: "
				+ max + ", nominal: " + nominal + "}";
	}

}
