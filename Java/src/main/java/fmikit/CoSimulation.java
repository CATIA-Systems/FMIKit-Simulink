/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

public class CoSimulation extends Implementation {

	public boolean canInterpolateInputs;

	@Override
	public String toString() {
		return "CoSimulation {canInterpolateInputs: " + canInterpolateInputs + ", modelIdentifier: " + modelIdentifier
				+ ", platforms: " + platforms + ", sourceFiles: " + sourceFiles + "}";
	}

}
