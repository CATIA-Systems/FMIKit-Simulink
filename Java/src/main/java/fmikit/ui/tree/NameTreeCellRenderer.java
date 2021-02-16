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

@SuppressWarnings("serial")
public class NameTreeCellRenderer extends DefaultTreeCellRenderer {

	ImageIcon subsystemIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/subsystem.png"));

	ImageIcon realIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/float.png"));
	ImageIcon realParameterIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/float-parameter.png"));
	ImageIcon realInputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/float-input.png"));
	ImageIcon realOutputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/float-output.png"));

	ImageIcon integerIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/integer.png"));
	ImageIcon integerInputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/integer-input.png"));
	ImageIcon integerOutputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/integer.png"));

	ImageIcon enumerationIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/enumeration.png"));
	ImageIcon enumerationInputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/enumeration-input.png"));
	ImageIcon enumerationOutputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/enumeration.png"));

	ImageIcon booleanIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/boolean.png"));
	ImageIcon booleanInputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/boolean-input.png"));
	ImageIcon booleanOutputIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/boolean.png"));

	ImageIcon stringIcon = new ImageIcon(FMUBlockDialog.class.getResource("/icons/string.png"));

	@Override
	public Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel, boolean expanded, boolean leaf,
			int row, boolean hasFocus) {

		NameTreeCellRenderer renderer = (NameTreeCellRenderer) super.getTreeCellRendererComponent(tree, value, sel,
				expanded, leaf, row, hasFocus);

		Object userObject = ((DefaultMutableTreeNode) value).getUserObject();

		if (userObject instanceof ScalarVariable) {
			ScalarVariable sv = (ScalarVariable) userObject;

			renderer.setText(getDisplayName(sv));

			if ("Real".equals(sv.type)) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(realInputIcon);
				} else if ("input".equals(sv.causality)) {
					renderer.setIcon(realInputIcon);
				} else if ("parameter".equals(sv.causality)) {
					renderer.setIcon(realParameterIcon);
				} else {
					renderer.setIcon(realIcon);
				}
			} else if ("Integer".equals(sv.type)) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(integerInputIcon);
				} else if ("output".equals(sv.causality)) {
					renderer.setIcon(integerOutputIcon);
				} else {
					renderer.setIcon(integerIcon);
				}
			} else if ("Enumeration".equals(sv.type)) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(enumerationInputIcon);
				} else if ("output".equals(sv.causality)) {
					renderer.setIcon(enumerationOutputIcon);
				} else {
					renderer.setIcon(enumerationIcon);
				}
			} else if ("Boolean".equals(sv.type)) {
				if ("input".equals(sv.causality)) {
					renderer.setIcon(booleanInputIcon);
				} else if ("output".equals(sv.causality)) {
					renderer.setIcon(booleanOutputIcon);
				} else {
					renderer.setIcon(booleanIcon);
				}
			} else if ("String".equals(sv.type)) {
				renderer.setIcon(stringIcon);
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