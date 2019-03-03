/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui.tree;

import java.awt.Component;

import javax.swing.JTable;
import javax.swing.table.DefaultTableCellRenderer;

import fmikit.ScalarVariable;


@SuppressWarnings("serial")
public class UnitTableCellRenderer extends DefaultTableCellRenderer {

	@Override
	public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus,
			int row, int column) {
		
		UnitTableCellRenderer renderer = (UnitTableCellRenderer) super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
		
		if (value instanceof ScalarVariable) {
			ScalarVariable variable = (ScalarVariable) value;
			
			String unit;
			
			if (variable.declaredType != null) {
				unit = variable.declaredType.unit;
			} else {
				unit = variable.unit;
			}

			if (unit != null) {
				renderer.setText(" " + unit);
				return renderer;
			}
		}
		
		renderer.setText(null);		
		
		return renderer;
	}
	
}