/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.util.ArrayList;
import java.util.List;

public abstract class Implementation {

	public String modelIdentifier;
	public List<String> platforms = new ArrayList<String>();
	public List<String> sourceFiles = new ArrayList<String>();

}
