/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

public class ModelExchange extends Implementation {

	@Override
	public String toString() {
		return "ModelExchange {modelIdentifier: " + modelIdentifier + ", platforms: "
				+ platforms + ", sourceFiles: " + sourceFiles + "}";
	}
}
