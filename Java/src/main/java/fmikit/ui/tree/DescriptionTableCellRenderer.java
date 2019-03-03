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
public class DescriptionTableCellRenderer extends DefaultTableCellRenderer {

	@Override
	public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus,
			int row, int column) {
		
		DescriptionTableCellRenderer renderer = (DescriptionTableCellRenderer) super.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
		
		if (value instanceof ScalarVariable) {
			renderer.setText(((ScalarVariable) value).description);
		} else {
			renderer.setText(null);			
		}
		
		return renderer;
	}
	
}