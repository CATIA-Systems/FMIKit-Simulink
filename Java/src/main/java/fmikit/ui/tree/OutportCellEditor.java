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
public class OutportCellEditor extends DefaultCellEditor {
	
	public Map<String, String> outportNames;

	public OutportCellEditor(Map<String, String> outportNames) {
		super(new JTextField());
		this.outportNames = outportNames;
	}
	
	@Override
	public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
		JTextField component = (JTextField) super.getTableCellEditorComponent(table, value, isSelected, row, column);
		
		ScalarVariable sv = (ScalarVariable) value;

		if (outportNames.containsKey(sv.name)) {
			component.setText(outportNames.get(sv.name));
		} else {
			component.setText(sv.name);
		}
		
		return component;
	}
	
}
