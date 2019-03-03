/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui.tree;

import java.awt.Component;
import java.awt.Font;
import java.util.Map;

import javax.swing.JTable;
import javax.swing.SwingConstants;
import javax.swing.table.DefaultTableCellRenderer;

import fmikit.ScalarVariable;

@SuppressWarnings("serial")
public class StartTableCellRenderer extends DefaultTableCellRenderer {

	public Map<String, String> startValues;

	public StartTableCellRenderer(Map<String, String> startValues) {
		this.startValues = startValues;
	}

	@Override
	public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus,
			int row, int column) {

		StartTableCellRenderer renderer = (StartTableCellRenderer) super.getTableCellRendererComponent(table, value,
				isSelected, hasFocus, row, column);

		if (value instanceof ScalarVariable) {
			ScalarVariable sv = (ScalarVariable) value;

			if (startValues.containsKey(sv.name)) {
				Font font = getFont();
				Font boldFont = new Font(font.getFontName(), Font.BOLD, font.getSize());
				setFont(boldFont);
				renderer.setText(startValues.get(sv.name));
			} else {
				renderer.setText(sv.startValue);
			}

			renderer.setHorizontalAlignment(SwingConstants.RIGHT);
		} else {
			renderer.setText(null);
		}

		return renderer;
	}

	public static String getDimensions(int[] arr) {
		if (arr.length == 1) {
			return "<1 x " + arr[0] + ">";
		}
		
		String result = "<";
		for (int i = 0; i < arr.length; i++) {
			result += Integer.toString(arr[i]);
			if (i < arr.length - 1) {
				result += " x ";
			}
		}
		result += ">";
				
		return result;
	}

}