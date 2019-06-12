/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;


public class UserData {

	public String fmiKitVersion;
	
	public String fmuFile;
	
	public long fmuLastModified;
	
	public String unzipDirectory;
	
	public int runAsKind;

	public String sampleTime;

	public String relativeTolerance;

	public ArrayList<Port> inputPorts = new ArrayList<Port>();

	public ArrayList<Port> outputPorts = new ArrayList<Port>();
	
	public HashMap<String, String> startValues = new HashMap<String, String>();
	
	public boolean debugLogging = false;
	
	public String errorDiagnostics = "error";
	
	public boolean useSourceCode = false;
	
	public boolean setBlockName = true;
	
	public String parameters;
	
	public String functionName;

	public boolean directInput = false;
	
	public static class Port {

	    public Port() {}

		public Port(String label, String... variables) {
			this.label = label;
			this.variables = new ArrayList<String>(Arrays.asList(variables));
		}

		public String label;

		public ArrayList<String> variables = new ArrayList<String>();
		
	}

}
