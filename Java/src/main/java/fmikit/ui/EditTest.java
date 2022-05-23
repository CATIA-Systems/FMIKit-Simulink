/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui;

import java.awt.EventQueue;
import java.util.ArrayList;
import java.util.List;

import javax.swing.UIManager;

import fmikit.ui.UserData.Port;
import fmikit.ui.UserData.DialogParameter;

@SuppressWarnings("all")
public class EditTest {

	public static void main(String[] args) {

		EventQueue.invokeLater(new Runnable() {

			public void run() {

				try {
					UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
					//FMUBlockDialog dialog = FMUBlockDialog.getDialog(1);

					FMUBlockDialog dialog = FMUBlockDialog.getDialog(1);

					dialog.mdlDirectory = "C:\\Temp";
					
					UserData userData = new UserData();
										
					userData.fmiKitVersion = "2.3";
					userData.fmuFile = "C:\\Temp\\ControlledTemperaturev.fmu";
					userData.fmuLastModified = 1L;
					userData.unzipDirectory = "C:\\Temp\\BooleanNetwork1";
					userData.runAsKind = 1;
					userData.sampleTime = "-1";
					userData.startValues = new ArrayList<DialogParameter>();
					userData.startValues.add(new DialogParameter("f", "f", "55"));

                    userData.inputPorts.add(new Port("a", "b"));
                    userData.outputPorts.add(new Port("Losses", "Losses"));
                    userData.outputPorts.add(new Port("y", "y1", "y2", "y3"));
					
					userData.functionName = "sfun_fmurun";					
					userData.parameters = "";
					userData.debugLogging = true;
					userData.useSourceCode = false;

					dialog.setUserData(userData);
					
					//dialog.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
					
					dialog.checkFMUModified();
					dialog.loadModelDescription();

					dialog.setTitle("cc/FMU");
					dialog.setLocationRelativeTo(null);
					dialog.setVisible(true);

//					List<List<ScalarVariable>> inputPortVariables = dialog.getInputPorts();
//					List<List<ScalarVariable>> outputPortVariables = dialog.getOutputPorts();
//					List<Boolean> inputPortDirectFeedThrough = dialog.getInputPortDirectFeedThrough(inputPortVariables, outputPortVariables);
					
					String params = dialog.getSFunctionParameters();
					System.out.println(params);
//					
//					dialog.restoreOutportTree();
//					
//					UserData userData2 = dialog.getUserData();
//					
//					System.out.println(userData2.toJSON());
//					
					dialog.generateSourceSFunction();
					
					List<String> sourceFiles = dialog.getSourceFiles();
					
					System.out.println(sourceFiles);
					
				} catch (Exception e) {
					e.printStackTrace();
				}

			}

		});
	}
	
}
