/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui.tree;

import java.awt.Component;
import java.util.Map;

import javax.swing.JTable;
import javax.swing.table.DefaultTableCellRenderer;

import fmikit.ScalarVariable;

@SuppressWarnings("serial")
public class OutportTableCellRenderer extends DefaultTableCellRenderer {
	
	public Map<String, String> outportsNames;
	
	public OutportTableCellRenderer(Map<String, String> outportsNames) {
		this.outportsNames = outportsNames;
	}

	@Override
	public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus,
			int row, int column) {

		OutportTableCellRenderer renderer = (OutportTableCellRenderer) super.getTableCellRendererComponent(table, value,
				isSelected, hasFocus, row, column);

		if (value instanceof ScalarVariable) {
			ScalarVariable sv = (ScalarVariable) value;
			
			if (outportsNames.containsKey(sv.name)) {
				renderer.setText(outportsNames.get(sv.name));
				return renderer;
			}
		}

		renderer.setText(null);
		return renderer;
	}

}