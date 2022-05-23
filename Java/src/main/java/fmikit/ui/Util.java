/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui;

import com.intellij.uiDesigner.core.GridConstraints;
import com.intellij.uiDesigner.core.GridLayoutManager;
import fmikit.ScalarVariable;

import javax.swing.*;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;
import java.awt.*;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class Util {


	public static void resizeButton(JButton button) {

		JPanel parent = (JPanel) button.getParent();

		GridLayoutManager layout = (GridLayoutManager) parent.getLayout();

		GridConstraints constraints = layout.getConstraintsForComponent(button);

		parent.remove(button);

		ImageIcon icon = (ImageIcon) button.getIcon();

		String s = icon.toString();

		String name = s.substring(s.lastIndexOf('/') + 1, s.length() - 4);

		button.setIcon(Util.getIcon(name));

		Dimension size = new Dimension(36, 36);
		parent.add(button, new GridConstraints(constraints.getRow(), constraints.getColumn(), constraints.getRowSpan(), constraints.getColSpan(), constraints.getAnchor(), constraints.getFill(), constraints.getHSizePolicy(), constraints.getVSizePolicy(), size, size, size, constraints.getIndent(), constraints.isUseParentLayout()));

	}

	public static void setTreeExpandedState(JTree tree, boolean expanded) {
		DefaultMutableTreeNode node = (DefaultMutableTreeNode) tree.getModel().getRoot();
		setNodeExpandedState(tree, node, expanded);
	}

	public static void setNodeExpandedState(JTree tree, DefaultMutableTreeNode node, boolean expanded) {
		ArrayList<DefaultMutableTreeNode> list = Collections.list(node.children());
		for (DefaultMutableTreeNode treeNode : list) {
			setNodeExpandedState(tree, treeNode, expanded);
		}
		if (!expanded && node.isRoot()) {
			return;
		}
		TreePath path = new TreePath(node.getPath());
		if (expanded) {
			tree.expandPath(path);
		} else {
			tree.collapsePath(path);
		}
	}


	public static boolean isHiDPI() {
		final int pixelsPerInch = Toolkit.getDefaultToolkit().getScreenResolution();
		return pixelsPerInch > 120;
	}

	public static ImageIcon getIcon(String name) {
		final String suffix = isHiDPI() ? "@2x.png" : ".png";
		return new ImageIcon(Util.class.getResource("/icons/" + name + suffix));
	}

	public static String getRelativePath(String path, String reference) {
		
		if (reference != null && path.startsWith(reference)) {
			return path.substring(reference.length() + 1);
		}
		
		return path;
	}
	
	public static String joinPath(String first, String... more) {
		for (String s : more) {
			first += File.separator + s;
		}
		return first;
	}
	
	public static String join(String separator, String first, String... more) {
		for (String s : more) {
			first += separator + s;
		}
		return first;
	}
	
	public static String join(Iterable<?> items, String delimiter) {
		StringBuilder sb = new StringBuilder();
		String delim = "";
		for (Object item : items) {
		    sb.append(delim).append(item);
		    delim = delimiter;
		}
		return sb.toString();
	}
	
	public static boolean isFile(String first, String... more) {
		String path = joinPath(first, more);
		File file = new File(path);
		return file.isFile();
	}

	public static void unzip(String zipFile, String outputFolder) throws IOException {

		byte[] buffer = new byte[1024];

		File folder = new File(outputFolder);

		if (folder.exists())
			delete(folder);

		folder.mkdir();

		ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFile));

		ZipEntry ze = zis.getNextEntry();
		
		while (ze != null) {
			String fileName = ze.getName();
			File newFile = new File(outputFolder + File.separator + fileName);

			// create all non exists folders
			// else you will hit FileNotFoundException for compressed folder
			new File(newFile.getParent()).mkdirs();
			
			if (!ze.isDirectory()) {				
				FileOutputStream fos = new FileOutputStream(newFile);
	
				int len;
	
				while ((len = zis.read(buffer)) > 0) {
					fos.write(buffer, 0, len);
				}
	
				fos.close();
			}
			
			ze = zis.getNextEntry();
		}

		zis.closeEntry();
		zis.close();
	}

	public static void delete(File f) throws IOException {
		
		if (f.isDirectory()) {
			for (File c : f.listFiles())
				delete(c);
		}
		
		if (!f.delete())
			throw new FileNotFoundException("Failed to delete file: " + f);
	}

	public static int typeEnumForVariable(ScalarVariable variable) {

		final String type = variable.type;
		final boolean discrete = "tunable".equals(variable.variability) || "discrete".equals(variable.variability);

		if ("Float32".equals(type)) {
			return discrete ? 1 : 0;
		} else if ("Float64".equals(type) || "Real".equals(type)) {
			return discrete ? 3 : 2;
		} else if ("Int8".equals(type)) {
			return 4;
		} else if ("UInt8".equals(type)) {
			return 5;
		} else if ("Int16".equals(type)) {
			return 6;
		} else if ("UInt16".equals(type)) {
			return 7;
		} else if ("Int32".equals(type) || "Integer".equals(type) || "Enumeration".equals(type)) {
			return 8;
		} else if ("UInt32".equals(type)) {
			return 9;
		} else if ("Int64".equals(type)) {
			return 10;
		} else if ("UInt64".equals(type)) {
			return 11;
		} else if ("Boolean".equals(type)) {
			return 12;
		} else if ("String".equals(type)) {
			return 13;
		} else if ("Binary".equals(type)) {
			return 14;
		} else if ("Clock".equals(type)) {
			return 15;
		}

		throw new RuntimeException("Unknown variable type: " + type);
	}

}
