/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui.tree;

import fmikit.ScalarVariable;
import org.jdesktop.swingx.treetable.AbstractTreeTableModel;

import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;
import java.util.Map;

public class VariablesTreeTableModel extends AbstractTreeTableModel {

	public static final String[] COLUMN_NAMES = { "Name", "Start", "Unit", "Description"};

	public static final int NAME_COLUMN = 0;
	public static final int START_COLUMN = 1;
	public static final int UNIT_COLUMN = 2;
	public static final int DESCRIPTION_COLUMN = 3;

	public Map<String, String> startValues;

	public VariablesTreeTableModel(DefaultMutableTreeNode root, Map<String, String> startValues) {
		super(root);
		this.startValues = startValues;
	}

	@Override
	public int getColumnCount() {
		return COLUMN_NAMES.length;
	}

	@Override
	public Object getValueAt(Object node, int column) {
		return ((DefaultMutableTreeNode) node).getUserObject();
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

		if (userObject instanceof ScalarVariable && column == START_COLUMN) {
			return true;
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
		}
	}

}
