/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ModelStructure {

	/**
	 * Holds the dependencies of the outputs variables. If dependencies is <code>null</code>
	 * the output variables depends on all inputs.
	 */
	public Map<ScalarVariable, List<Dependency>> outputs = new HashMap<ScalarVariable, List<Dependency>>();

	public Map<ScalarVariable, List<Dependency>> derivatives = new HashMap<ScalarVariable, List<Dependency>>();

	public Map<ScalarVariable, List<Dependency>> initialUnknowns = new HashMap<ScalarVariable, List<Dependency>>();

	@Override
	public String toString() {
		return "ModelStructure {outputs: <" + outputs.size() + ">, derivatives: <" + derivatives.size() + ">, initialUnknowns: <"
				+ initialUnknowns.size() + ">}";
	}

}
