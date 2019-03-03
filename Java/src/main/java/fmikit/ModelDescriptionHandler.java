/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit;

import java.util.ArrayList;
import java.util.List;

import org.xml.sax.SAXException;
import org.xml.sax.SAXParseException;
import org.xml.sax.helpers.DefaultHandler;

public abstract class ModelDescriptionHandler extends DefaultHandler {

	protected ModelDescription modelDescription = new ModelDescription();

	protected ArrayList<String> messages = new ArrayList<String>();
	
	protected SimpleType simpleType;
	
	public ModelDescriptionHandler() {
		modelDescription.modelStructure = new ModelStructure();
	}

	public List<String> getMessages() {
		return messages;
	}

	public ModelDescription getModelDescription() {
		return modelDescription;
	}

	public void warning(SAXParseException spe) throws SAXException {
		logException(spe);
	}

	public void error(SAXParseException spe) throws SAXException {
		logException(spe);
	}

	public void fatalError(SAXParseException spe) throws SAXException {
		logException(spe);
	}

	protected void logException(SAXParseException spe) {
		String message = "Column " + spe.getColumnNumber() + ", line " + spe.getLineNumber() + ": " + spe.getMessage();
		messages.add(message);
	}

}
