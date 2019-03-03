/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui.tree;

import java.awt.Component;
import java.util.Map;

import javax.swing.DefaultCellEditor;
import javax.swing.JTable;
import javax.swing.JTextField;

import fmikit.ScalarVariable;

@SuppressWarnings("serial")
public class StartCellEditor extends DefaultCellEditor {
	
	public Map<String, String> startValues;

	public StartCellEditor(Map<String, String> startValues) {
		super(new JTextField());
		this.startValues = startValues;
	}
	
	@Override
	public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
		JTextField component = (JTextField) super.getTableCellEditorComponent(table, value, isSelected, row, column);
		
		ScalarVariable sv = (ScalarVariable) value;

		if (startValues.containsKey(sv.name)) {
			component.setText(startValues.get(sv.name));
		} else {
			component.setText(sv.startValue);				
		}
		
		return component;
	}
	
}
