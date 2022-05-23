/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui.tree;

import java.awt.Component;

import javax.swing.ImageIcon;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;

import fmikit.ScalarVariable;
import fmikit.ui.FMUBlockDialog;
import fmikit.ui.Util;

@SuppressWarnings("serial")
public class NameTreeCellRenderer extends DefaultTreeCellRenderer {

	ImageIcon subsystemIcon = Util.getIcon("subsystem");

	ImageIcon floatIcon = Util.getIcon("float");
	ImageIcon realParameterIcon = Util.getIcon("float-parameter");
	ImageIcon floatInputIcon = Util.getIcon("float-input");
	ImageIcon floatOutputIcon = Util.getIcon("float-output");

	ImageIcon integerIcon = Util.getIcon("integer");
	ImageIcon integerParameterIcon = Util.getIcon("integer-parameter");
	ImageIcon integerInputIcon = Util.getIcon("integer-input");
	ImageIcon integerOutputIcon = Util.getIcon("integer-output");

	ImageIcon enumerationIcon = Util.getIcon("enumeration");
	ImageIcon enumerationParameterIcon = Util.getIcon("enumeration-parameter");
	ImageIcon enumerationInputIcon = Util.getIcon("enumeration-input");
	ImageIcon enumerationOutputIcon = Util.getIcon("enumeration");

	ImageIcon booleanIcon = Util.getIcon("boolean");
	ImageIcon booleanParameterIcon = Util.getIcon("boolean-parameter");
	ImageIcon booleanInputIcon = Util.getIcon("boolean-input");
	ImageIcon booleanOutputIcon = Util.getIcon("boolean");

	ImageIcon stringParameterIcon = Util.getIcon("string-parameter");

	@Override
	public Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel, boolean expanded, boolean leaf,
			int row, boolean hasFocus) {

		NameTreeCellRenderer renderer = (NameTreeCellRenderer) super.getTreeCellRendererComponent(tree, value, sel,
				expanded, leaf, row, hasFocus);

		Object userObject = ((DefaultMutableTreeNode) value).getUserObject();

		if (userObject instanceof ScalarVariable) {
			ScalarVariable sv = (ScalarVariable) userObject;

			renderer.setText(getDisplayName(sv));

			if ("Real".equals(sv.type) || sv.type.startsWith("Float")) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(floatInputIcon);
				} else if ("output".equals(sv.causality)) {
					renderer.setIcon(floatOutputIcon);
				} else if ("parameter".equals(sv.causality) || "structuralParameter".equals(sv.causality)) {
					renderer.setIcon(realParameterIcon);
				} else {
					renderer.setIcon(floatIcon);
				}
			} else if (sv.type.startsWith("Int")) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(integerInputIcon);
				} else if ("output".equals(sv.causality)) {
					renderer.setIcon(integerOutputIcon);
				} else if ("parameter".equals(sv.causality) || "structuralParameter".equals(sv.causality)) {
					renderer.setIcon(integerParameterIcon);
				}  else {
					renderer.setIcon(integerIcon);
				}
			} else if ("Enumeration".equals(sv.type)) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(enumerationInputIcon);
				} else if ("output".equals(sv.causality)) {
					renderer.setIcon(enumerationOutputIcon);
				} else if ("parameter".equals(sv.causality) || "structuralParameter".equals(sv.causality)) {
					renderer.setIcon(enumerationParameterIcon);
				} else {
					renderer.setIcon(enumerationIcon);
				}
			} else if ("Boolean".equals(sv.type)) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(booleanInputIcon);
				} else if ("output".equals(sv.causality)) {
					renderer.setIcon(booleanOutputIcon);
				} else if ("parameter".equals(sv.causality) || "structuralParameter".equals(sv.causality)) {
					renderer.setIcon(booleanParameterIcon);
				} else {
					renderer.setIcon(booleanIcon);
				}
			} else if ("String".equals(sv.type)) {
				renderer.setIcon(stringParameterIcon);
			} else {
				// TODO
				// System.out.println(sv.name);
			}

		} /*else if (userObject instanceof String) {
			renderer.setText((String) userObject);
			renderer.setIcon(vectorOutputPortIcon);
		} */ else {
			renderer.setIcon(subsystemIcon);
			renderer.setText(userObject.toString());
		}

		return renderer;
	}

	public String getDisplayName(ScalarVariable sv) {
		String name = sv.name;
		boolean derivative = name.startsWith("der(") && name.endsWith("");
		
		if (derivative) {
			name = name.substring(4, name.length() - 1);
		}
		
		int i = name.lastIndexOf('.');
		
		name = name.substring(i + 1);
		
		if (derivative) {
			name = "der(" + name + ")";
		}
		
		return name;
	}

}