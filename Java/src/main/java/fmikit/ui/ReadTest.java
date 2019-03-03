/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui;

import java.io.File;

import fmikit.ModelDescriptionReader;

public class ReadTest {

	static int count = 0;

	public static void iterateFolder(File dir) throws Exception {

		for (File child : dir.listFiles()) {
			if (child.isDirectory()) {
				iterateFolder(child);
			} else {
				String name = child.getName();
				if (name.endsWith(".fmu")) {
					ModelDescriptionReader.readFromFMU(child.getAbsolutePath());
					count++;
				}
			}
		}

	}

	public static void main(String args[]) throws Exception {

		File dir = new File(args[0]);

		iterateFolder(dir);

		System.out.println("Read " + count + " model descriptions");

	}

}