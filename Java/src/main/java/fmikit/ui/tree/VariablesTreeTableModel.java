/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui.tree;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;

import org.jdesktop.swingx.treetable.AbstractTreeTableModel;

import fmikit.ScalarVariable;

public class VariablesTreeTableModel extends AbstractTreeTableModel {

	public static final String[] COLUMN_NAMES = { "Name", "Start", "Unit", "Input", "Description"};

	public static final int NAME_COLUMN = 0;
	public static final int START_COLUMN = 1;
	public static final int UNIT_COLUMN = 2;
	public static final int INPUT_COLUMN = 3;
	public static final int DESCRIPTION_COLUMN = 4;

	public Map<String, String> startValues;

	public Set<String> inputVariables;

	public VariablesTreeTableModel(DefaultMutableTreeNode root, Map<String, String> startValues, Set<String> inputVariables) {
		super(root);
		this.startValues = startValues;
		this.inputVariables = inputVariables;
	}

	@Override
	public int getColumnCount() {
		return COLUMN_NAMES.length;
	}

	@Override
	public Class<?> getColumnClass(int column) {
		if (column == INPUT_COLUMN)
			return Boolean.class;
		else
			return super.getColumnClass(column);
	}

	@Override
	public Object getValueAt(Object node, int column) {
		Object userObject = ((DefaultMutableTreeNode) node).getUserObject();

//		if (column == INPUT_COLUMN) {
//			return true;
//
////			if (userObject instanceof ScalarVariable) {
////				ScalarVariable variable = (ScalarVariable) userObject;
////				return inputVariables.contains(variable.name);
////			}
////
////			return false;
//		}

		return userObject;
	}

	public Object getChild(Object object, int index) {
		return ((DefaultMutableTreeNode) object).getChildAt(index);
	}

	public int getChildCount(Object object) {
		return ((DefaultMutableTreeNode) object).getChildCount();
	}

	public int getIndexOfChild(Object parent, Object child) {
		return ((DefaultMutableTreeNode) parent).getIndex((TreeNode) child);
	}

	public String getColumnName(int column) {
		return COLUMN_NAMES[column];
	}
	
	public boolean isCellEditable(Object node, int column) {

		Object userObject = ((DefaultMutableTreeNode)node).getUserObject();

		if (userObject instanceof ScalarVariable) {

			ScalarVariable variable = (ScalarVariable) userObject;

			if (column == START_COLUMN) {
				return true;
			} else if (column == INPUT_COLUMN) {

				if ("String".equals(variable.type)) {
					return false;
				}

				return "tunable".equals(variable.variability);
			}
		}

		return false;
	}
	
	public void setValueAt(Object value, Object node, int column) {

		Object userObject = ((DefaultMutableTreeNode) node).getUserObject();

		if (!(userObject instanceof ScalarVariable)) {
			return;
		}

		ScalarVariable sv = (ScalarVariable) userObject;

		if (column == START_COLUMN) {
			String text = (String) value;

			if ("".equals(text)) {
				startValues.remove(sv.name);
			} else {
				startValues.put(sv.name, text);
			}
		} else if (column == INPUT_COLUMN) {
			if (inputVariables.contains(sv.name)) {
				inputVariables.remove(sv.name);
			} else {
				inputVariables.add(sv.name);
			}
		}

	}

}
