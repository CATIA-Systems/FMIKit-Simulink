/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui;

import java.util.ArrayList;
import java.util.Arrays;


public class UserData {

	public String fmiKitVersion;
	
	public String fmuFile;
	
	public long fmuLastModified;

	public int runAsKind;

	public String unzipDirectory;

	public boolean debugLogging = false;

	public boolean logFMICalls = false;

	public int logLevel = 0;

	public String logFile;

	public boolean logToFile = false;

	public String relativeTolerance;

	public String sampleTime;

	public ArrayList<Port> inputPorts = new ArrayList<Port>();

	public ArrayList<Port> outputPorts = new ArrayList<Port>();

	public ArrayList<DialogParameter> startValues = new ArrayList<DialogParameter>();

	public boolean useSourceCode = false;

	public String parameters;
	
	public String functionName;

    public boolean resettable;

    public static class Port {

	    public Port() {}

		public Port(String label, String... variables) {
			this.label = label;
			this.variables = new ArrayList<String>(Arrays.asList(variables));
		}

		public String label;

		public ArrayList<String> variables = new ArrayList<String>();
		
	}

	public static class DialogParameter {

    	public String name;
    	public String prompt;
    	public String value;

		public DialogParameter(String name, String prompt, String value) {
			this.name = name;
			this.prompt = prompt;
			this.value = value;
		}
	}

}
