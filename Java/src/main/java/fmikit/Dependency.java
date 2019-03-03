/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

public class Dependency {

	public enum DependencyKind {
		DEPENDENT, // no particular structure, f(v)
		CONSTANT, // constant factor, c*v (only for Real variables)
		FIXED, // fixed factor, p*v (only for Real variables)
		TUNABLE, // tunable factor, p*v (only for Real variables)
		DISCRETE // discrete factor, d*v (only for Real variables)
	}

	public ScalarVariable variable;
	public DependencyKind kind;

	@Override
	public String toString() {
		return "Dependency {variable: " + (variable != null ? variable.name : null) + ", kind: " + kind + "}";
	}

}
