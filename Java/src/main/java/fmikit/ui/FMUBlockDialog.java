/*
 * Copyright (c) Dassault Systemes. All rights reserved.
 * This file is part of FMIKit. See LICENSE.txt in the project root for license information.
 */

package fmikit.ui;

import com.intellij.uiDesigner.core.GridConstraints;
import com.intellij.uiDesigner.core.GridLayoutManager;
import com.intellij.uiDesigner.core.Spacer;
import fmikit.*;
import fmikit.ui.tree.*;
import org.jdesktop.swingx.JXTreeTable;

import javax.imageio.ImageIO;
import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.List;
import java.util.*;


public class FMUBlockDialog extends JDialog {

    private JPanel contentPane;
    public JButton btnOK;
    private JButton cancelButton;
    private JTabbedPane tabbedPane;
    public JTextField txtFMUPath;
    private JButton loadButton;
    private JButton reloadButton;
    private JComboBox cmbbxRunAsKind;
    private JLabel lblFmiVersion;
    private JLabel lblPlatforms;
    private JLabel lblModelName;
    private JTextPane txtpnDescription;
    private JLabel lblGenerationTool;
    private JLabel lblGenerationDate;
    private JLabel lblContinuousStates;
    private JLabel lblEventIndicators;
    private JLabel lblVariables;
    public JButton btnApply;
    private JButton moveUpButton;
    private JButton moveDownButton;
    private JButton editButton;
    private JButton removeOutputPortButton;
    private JButton resetOutputsButton;
    private JButton addOutputButton;
    private JTree variablesTree;
    private JTree outportsTree;
    private JTextField txtUnzipDirectory;
    private JTextField txtSampleTime;
    private JCheckBox chckbxDebugLogging;
    private JComboBox cmbbxLogLevel;
    private JCheckBox chckbxUseSourceCode;
    private JLabel lblMessageIcon;
    private JLabel lblMessage;
    private JXTreeTable treeTable;
    private JButton addScalarOutputPortButton;
    private JTextField txtRelativeTolerance;
    private JTextField txtLogFile;
    private JCheckBox chckbxLogToFile;
    private JCheckBox chckbxLogFMICalls;
    public JButton btnHelp;
    public JLabel lblDocumentation;
    private JLabel lblModelImage;
    private JButton treeTableExpandAllButton;
    private JButton treeTableCollapseAllButton;
    private JButton variablesTreeExpandAllButton;
    private JButton variablesTreeCollapseAllButton;

    public static boolean debugLogging = false;
    public static final String FMI_KIT_VERSION = "2.7";
    public static final int MODEL_EXCHANGE = 0;
    public static final int CO_SIMULATION = 1;
    public static HashMap<Double, FMUBlockDialog> dialogs = new HashMap<Double, FMUBlockDialog>();
    public ModelDescription modelDescription;
    public DefaultMutableTreeNode outportRoot; // TODO: access via model
    public DefaultTreeModel outportTreeModel; // TODO: access via tree
    public HashMap<String, String> startValues = new HashMap<String, String>();
    private UserData userData;
    public String mdlDirectory;
    public double blockHandle;
    public File htmlFile;

    private ImageIcon outportIcon = Util.getIcon("outport");


    public FMUBlockDialog() {

        if (Util.isHiDPI()) {
            setMinimumSize(new Dimension(1200, 900));
            treeTable.setRowHeight(34);
            variablesTree.setRowHeight(34);
            outportsTree.setRowHeight(34);

            Util.resizeButton(treeTableExpandAllButton);
            Util.resizeButton(treeTableCollapseAllButton);

            Util.resizeButton(variablesTreeExpandAllButton);
            Util.resizeButton(variablesTreeCollapseAllButton);

            Util.resizeButton(moveUpButton);
            Util.resizeButton(moveDownButton);
            Util.resizeButton(editButton);
            Util.resizeButton(removeOutputPortButton);
            Util.resizeButton(resetOutputsButton);

        } else {
            setMinimumSize(new Dimension(850, 650));
        }

        setContentPane(contentPane);

//        setModal(true);

        getRootPane().setDefaultButton(btnOK);

        btnOK.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dispose();
            }
        });

        cancelButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dispose();
            }
        });

        // call onCancel() when cross is clicked
        setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
        addWindowListener(new WindowAdapter() {

            public void windowClosing(WindowEvent e) {
                dispose();
            }

        });

        // call onCancel() on ESCAPE
        contentPane.registerKeyboardAction(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                dispose();
            }

        }, KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);

        // TODO: center the window on screen
        setBounds(200, 200, 852, 619);

        addActionListeners();
        addScalarOutputPortButton.addActionListener(new ActionListener() {

            //@Override
            public void actionPerformed(ActionEvent e) {
                addSelectionToOuputs();
            }

        });

        loadButton.addActionListener(new ActionListener() {
            //@Override
            public void actionPerformed(ActionEvent actionEvent) {
                loadFMU(true);
            }
        });

        reloadButton.addActionListener(new ActionListener() {
            //@Override
            public void actionPerformed(ActionEvent actionEvent) {
                loadFMU(false);
            }
        });

        editButton.addActionListener(new ActionListener() {

            //@Override
            public void actionPerformed(ActionEvent e) {
                TreePath path = outportsTree.getSelectionPath();
                outportsTree.startEditingAtPath(path);
            }

        });

        // get rid of the "colors, food, sports" default model
        variablesTree.setModel(null);
        outportsTree.setModel(null);

        // disable the "Relative Tolerance" when "Model Exchange" is selected
        cmbbxRunAsKind.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                boolean isCoSimulation = cmbbxRunAsKind.getSelectedIndex() == 1;
                txtRelativeTolerance.setEnabled(isCoSimulation);
            }
        });

        // enable "Log file" when "Log to file" is selected
        chckbxLogToFile.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                txtLogFile.setEnabled(chckbxLogToFile.isSelected());
            }
        });

        treeTableExpandAllButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                treeTable.expandAll();
            }
        });

        treeTableCollapseAllButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                treeTable.collapseAll();
            }
        });


        variablesTreeExpandAllButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Util.setTreeExpandedState(variablesTree, true);
            }
        });

        variablesTreeCollapseAllButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                Util.setTreeExpandedState(variablesTree, false);
            }
        });

    }

    public void showAsync() {

        EventQueue.invokeLater(new Runnable() {

            //@Override
            public void run() {
                setVisible(true);
                toFront();
                requestFocus();
            }

        });

    }

    private void addSelectionToOuputs() {

        ArrayList<ScalarVariable> scalarVariables = getSelectedVariables();

        if (!scalarVariables.isEmpty()) {

            for (ScalarVariable sv : scalarVariables) {
                DefaultMutableTreeNode outputPortNode = new DefaultMutableTreeNode();
                outputPortNode.setUserObject(sv.name);
                outputPortNode.add(new DefaultMutableTreeNode(sv));
                outportTreeModel.insertNodeInto(outputPortNode, outportRoot, outportRoot.getChildCount());
            }

            // workaround: JTree does not update if the root node was previously empty
            outportTreeModel.reload();
        }
    }

    public static FMUBlockDialog getDialog(double blockHandle) {

        if (dialogs.containsKey(blockHandle)) {
            return dialogs.get(blockHandle);
        }

        FMUBlockDialog dialog = new FMUBlockDialog();
        dialog.blockHandle = blockHandle;
        dialogs.put(blockHandle, dialog);
        return dialog;
    }

    public void setBlockPath(String blockPath) {

        final String[] segments = blockPath.split("/");

        setTitle("Block Parameters: " + segments[segments.length - 1]);

        if (txtLogFile.getText().isEmpty()) {
            // replace non-alphanumeric characters with underscores
            String logFileName = blockPath.replaceAll("[^A-Za-z0-9]", "_");
            txtLogFile.setText(logFileName + ".txt");
        }
    }

    public static void closeDialog(double blockHandle) {
        if (dialogs.containsKey(blockHandle)) {
            FMUBlockDialog dialog = dialogs.get(blockHandle);
            dialog.setVisible(false);
            dialogs.remove(blockHandle);
            // TODO: dispose?
        }
    }

    public void close() {
        setVisible(false);
        for (double blockHandle : dialogs.keySet()) {
            if (dialogs.get(blockHandle) == this) {
                dialogs.remove(blockHandle);
                break;
            }
        }
        // TODO: dispose?
    }


    public static void main(String[] args) {

        try {
            UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
        } catch (Exception e) {
            // ignore
        }

        final FMUBlockDialog dialog = new FMUBlockDialog();
        dialog.pack();
        dialog.setVisible(true);
        dialog.btnApply.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                // Util.setTreeExpandedState(dialog.treeTable.ex, true);
                dialog.treeTable.expandAll();
                System.out.println(dialog.getSFunctionParameters());
                System.out.println(dialog.getSourceFiles());
            }
        });
        //System.exit(0);
    }

//    public static void main(String[] args) throws ClassNotFoundException, UnsupportedLookAndFeelException, InstantiationException, IllegalAccessException {
//
//        UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
//
//        FMUBlockDialog dialog = new FMUBlockDialog();
//        dialog.pack();
//
//        UserData userData = new UserData();
//
//        userData.fmiKitVersion = "2.3";
//        userData.fmuFile = "C:\\Temp\\ControlledTemperaturev.fmu";
//        userData.fmuLastModified = 1L;
//        userData.unzipDirectory = "C:\\Temp\\BooleanNetwork1";
//        userData.runAsKind = 1;
//        userData.sampleTime = "-1";
//        userData.startValues = new HashMap<String, String>();
//        userData.startValues.put("f", "55");
//        userData.errorDiagnostics = "ignore";
//
//        UserData.Port inputPort = new UserData.Port("a", false, "b");
//        userData.inputPorts.add(inputPort);
//
//        UserData.Port outport = new UserData.Port("Losses", false, "Losses");
//        userData.outputPorts.add(outport);
//
//        outport = new UserData.Port("y", false, "y1", "y2", "y3");
//        userData.outputPorts.add(outport);
//
//        userData.functionName = "sfun_fmurun";
//        userData.parameters = "";
//        userData.debugLogging = true;
//        userData.useSourceCode = false;
//
//        dialog.setUserData(userData);
//
//        dialog.loadModelDescription();
//
//        dialog.setVisible(true);
//
//        System.exit(0);
//    }

    private void createUIComponents() {
        // TODO: place custom component creation code here
    }

    public void addActionListeners() {

        addWindowListener(new WindowAdapter() {

            @Override
            public void windowClosing(WindowEvent windowEvent) {
                for (double blockHandle : dialogs.keySet()) {
                    if (dialogs.get(blockHandle) == FMUBlockDialog.this) {
                        dialogs.remove(blockHandle);
                        break;
                    }
                }
            }

        });

        addOutputButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent arg0) {
                addSelectionAsOutput();
            }

        });

        removeOutputPortButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent arg0) {

                TreePath[] selectionPaths = outportsTree.getSelectionPaths();

                if (selectionPaths == null) return;

                for (TreePath treePath : selectionPaths) {

                    DefaultMutableTreeNode treeNode = (DefaultMutableTreeNode) treePath.getLastPathComponent();

                    if (treeNode.getUserObject() instanceof ScalarVariable && treeNode.getParent().getChildCount() < 2) {
                        continue; // don't remove the last variable
                    }

                    outportTreeModel.removeNodeFromParent(treeNode);
                }

            }

        });

        moveUpButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent arg0) {
                TreePath selectionPath = outportsTree.getSelectionPath();

                if (selectionPath == null) return;

                DefaultMutableTreeNode node = (DefaultMutableTreeNode) selectionPath.getLastPathComponent();
                DefaultMutableTreeNode parent = (DefaultMutableTreeNode) node.getParent();
                int index = parent.getIndex(node);

                if (index > 0) {
                    outportTreeModel.removeNodeFromParent(node);
                    outportTreeModel.insertNodeInto(node, parent, index - 1);
                    outportsTree.setSelectionPath(selectionPath);
                }
            }

        });

        moveDownButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent arg0) {

                TreePath selectionPath = outportsTree.getSelectionPath();

                if (selectionPath == null) return;

                DefaultMutableTreeNode node = (DefaultMutableTreeNode) selectionPath.getLastPathComponent();
                DefaultMutableTreeNode parent = (DefaultMutableTreeNode) node.getParent();
                int index = parent.getIndex(node);

                if (index < parent.getChildCount() - 1) {
                    outportTreeModel.removeNodeFromParent(node);
                    outportTreeModel.insertNodeInto(node, parent, index + 1);
                    outportsTree.setSelectionPath(selectionPath);
                }
            }

        });

        resetOutputsButton.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent event) {
                if (modelDescription == null) return;
                outportRoot = createOutportTree(modelDescription.scalarVariables);
                outportTreeModel.setRoot(outportRoot);
            }

        });

        variablesTree.addMouseListener(new MouseAdapter() {

            public void mousePressed(MouseEvent e) {

                int row = variablesTree.getRowForLocation(e.getX(), e.getY());

                if (row != -1 && e.getClickCount() == 2) {
                    addSelectionToOuputs();
                }
            }

        });
    }

    private void addSelectionAsOutput() {

        ArrayList<ScalarVariable> scalarVariables = getSelectedVariables();

        if (!scalarVariables.isEmpty()) {

            DefaultMutableTreeNode outputPortNode = new DefaultMutableTreeNode();

            // use the first segment of the first variable as port label
            String name = scalarVariables.get(0).name;
            String[] split = name.split("\\.");
            outputPortNode.setUserObject(split[0]);

            for (ScalarVariable sv : scalarVariables) {
                outputPortNode.add(new DefaultMutableTreeNode(sv));
            }

            outportTreeModel.insertNodeInto(outputPortNode, outportRoot, outportRoot.getChildCount());

            // workaround: JTree does not update if the root node was previously empty
            outportTreeModel.reload();
        }
    }

    private ArrayList<ScalarVariable> getSelectedVariables() {
        TreePath[] selectionPaths = variablesTree.getSelectionPaths();

        if (selectionPaths == null) return null;

        ArrayList<ScalarVariable> scalarVariables = new ArrayList<ScalarVariable>();

        for (TreePath treePath : selectionPaths) {
            collectScalarVariables(scalarVariables, (DefaultMutableTreeNode) treePath.getLastPathComponent());
        }

        return scalarVariables;
    }

    void collectScalarVariables(List<ScalarVariable> scalarVariables, DefaultMutableTreeNode treeNode) {

        Object userObject = treeNode.getUserObject();

        if (userObject instanceof ScalarVariable) {
            ScalarVariable sv = (ScalarVariable) userObject;
            if (!scalarVariables.contains(sv)) {
                scalarVariables.add(sv);
            }
        }

        Enumeration children = treeNode.children();

        while (children.hasMoreElements()) {
            Object element = children.nextElement();
            collectScalarVariables(scalarVariables, (DefaultMutableTreeNode) element);
        }

    }

    public static String getName(Object userObject) {
        if (userObject instanceof ScalarVariable) {
            return ((ScalarVariable) userObject).name;
        } else {
            return userObject.toString();
        }
    }

    public UserData getUserData() {

        UserData userData = new UserData();

        userData.fmiKitVersion = FMI_KIT_VERSION;
        userData.fmuFile = txtFMUPath.getText();
        userData.fmuLastModified = getFMULastModified();
        userData.runAsKind = cmbbxRunAsKind.getSelectedIndex();
        userData.unzipDirectory = txtUnzipDirectory.getText();
        userData.debugLogging = chckbxDebugLogging.isSelected();
        userData.logFMICalls = chckbxLogFMICalls.isSelected();
        userData.logLevel = cmbbxLogLevel.getSelectedIndex();
        userData.logFile = txtLogFile.getText();
        userData.logToFile = chckbxLogToFile.isSelected();
        userData.sampleTime = txtSampleTime.getText();
        userData.relativeTolerance = txtRelativeTolerance.getText();

        if (modelDescription != null) {

            for (List<ScalarVariable> variables : getInputPorts()) {

                ScalarVariable variable = variables.get(0);

                String label = variable.name;

                if (variables.size() > 1) {
                    // use the only the part before the array subscripts
                    label = label.substring(0, label.lastIndexOf("["));
                }

                userData.inputPorts.add(new UserData.Port(label, variable.name));
            }

        }

        for (int i = 0; i < outportRoot.getChildCount(); i++) {

            DefaultMutableTreeNode outputPortNode = (DefaultMutableTreeNode) outportRoot.getChildAt(i);

            String portLabel = (String) outputPortNode.getUserObject();

            UserData.Port outputPort = new UserData.Port(portLabel);

            for (int j = 0; j < outputPortNode.getChildCount(); j++) {
                DefaultMutableTreeNode scalarVariableNode = (DefaultMutableTreeNode) outputPortNode.getChildAt(j);
                ScalarVariable sv = (ScalarVariable) scalarVariableNode.getUserObject();
                outputPort.variables.add(sv.name);
            }

            userData.outputPorts.add(outputPort);
        }

        userData.startValues = new HashMap<String, String>(startValues);

        userData.useSourceCode = chckbxUseSourceCode.isSelected();

        userData.functionName = userData.useSourceCode ? "sfun_" + getModelIdentifier() : "sfun_fmurun";
        userData.parameters = getSFunctionParameters();

        return userData;
    }

    public long getFMULastModified() {
        if (userData != null) {
            return (Long) userData.fmuLastModified;
        } else {
            return new File(getAbsolutePath(txtFMUPath.getText())).lastModified();
        }
    }

    public void setUserData(UserData userData) {

        // TODO: check, deep copy?
        this.userData = userData;

        // check the format version
        if (!userData.fmiKitVersion.startsWith(FMI_KIT_VERSION)) {
            throw new RuntimeException("Wrong FMI Kit version. Expected " + FMI_KIT_VERSION + " but was " + userData.fmiKitVersion);
        }

        // Overview tab
        txtFMUPath.setText(userData.fmuFile);
        cmbbxRunAsKind.setSelectedIndex(userData.runAsKind);

        startValues.putAll(userData.startValues);

        // Advanced tab
        txtUnzipDirectory.setText(userData.unzipDirectory);
        txtSampleTime.setText(userData.sampleTime);
        txtRelativeTolerance.setText(userData.relativeTolerance);
        txtLogFile.setText(userData.logFile);
        txtLogFile.setEnabled(userData.logToFile);
        chckbxLogToFile.setSelected(userData.logToFile);
        cmbbxLogLevel.setSelectedIndex(userData.logLevel);
        chckbxDebugLogging.setSelected(userData.debugLogging);
        chckbxLogFMICalls.setSelected(userData.logFMICalls);
        chckbxUseSourceCode.setSelected(userData.useSourceCode);

        // TODO: restore outports?
    }

    public String getModelIdentifier() {
        return getImplementation().modelIdentifier;
    }

    public List<List<ScalarVariable>> getInputPorts() {

        List<List<ScalarVariable>> inputPorts = new ArrayList<List<ScalarVariable>>();
        ScalarVariable previousVariable = null;

        for (ScalarVariable variable : modelDescription.scalarVariables) {

            if ("Binary".equals(variable.type) || "Clock".equals(variable.type) || "String".equals(variable.type)) continue;

            if ("input".equals(variable.causality)) {

                if (belongsToSameArray(variable, previousVariable)) {
                    // add the variable to the previous input port
                    List<ScalarVariable> lastInputPort = inputPorts.get(inputPorts.size() - 1);
                    lastInputPort.add(variable);
                } else {
                    // add a new input port
                    List<ScalarVariable> inputPort = new ArrayList<ScalarVariable>();
                    inputPort.add(variable);
                    inputPorts.add(inputPort);
                }

                previousVariable = variable;
            }
        }

        return inputPorts;
    }


    public List<List<ScalarVariable>> getOutputPorts() {

        List<List<ScalarVariable>> outputPorts = new ArrayList<List<ScalarVariable>>();

        for (int i = 0; i < outportRoot.getChildCount(); i++) {

            DefaultMutableTreeNode outportNode = (DefaultMutableTreeNode) outportRoot.getChildAt(i);

            List<ScalarVariable> outputPort = new ArrayList<ScalarVariable>();

            for (int j = 0; j < outportNode.getChildCount(); j++) {
                DefaultMutableTreeNode node = (DefaultMutableTreeNode) outportNode.getChildAt(j);
                ScalarVariable variable = (ScalarVariable) node.getUserObject();
                outputPort.add(variable);
            }

            outputPorts.add(outputPort);
        }

        return outputPorts;
    }

    /**
     * Calculate the direct feed through flag for each input port
     *
     * @param inputPorts the variables for each input port
     * @return the direct feed through flags for each input port
     */
    public List<Boolean> getInputPortDirectFeedThrough(List<List<ScalarVariable>> inputPorts, List<List<ScalarVariable>> outputPorts) {

        ArrayList<Boolean> directFeedthrough = new ArrayList<Boolean>();

        Map<ScalarVariable, List<Dependency>> outputDependencies = modelDescription.modelStructure.outputs;

        HashSet<ScalarVariable> allDependencies = new HashSet<ScalarVariable>();

        boolean allDependent = false;

        // collect all dependencies of the outputs
        outer:
        for (List<ScalarVariable> outputPort : outputPorts) {

            for (ScalarVariable outputVariable : outputPort) {

                if (outputDependencies.containsKey(outputVariable)) {

                    List<Dependency> dependencies = outputDependencies.get(outputVariable);

                    if (dependencies == null) {
                        // there is an output variable that depends on all inputs
                        allDependent = true;
                        break outer;
                    }

                    for (Dependency dependency : dependencies) {
                        allDependencies.add(dependency.variable);
                    }

                } else {
                    // there are non-output variables in the output ports
                    // so we assume the outputs depend on all inputs
                    allDependent = true;
                    break outer;
                }

            }

        }

        // determine the output dependencies for each input port
        for (List<ScalarVariable> inputPort : inputPorts) {

            boolean directFeedThrough = allDependent;

            for (ScalarVariable inputVariable : inputPort) {

                if (allDependencies.contains(inputVariable)) {
                    directFeedThrough = true;
                    break;
                }

            }

            directFeedthrough.add(directFeedThrough);
        }

        return directFeedthrough;
    }

    public String getSFunctionParameters() {

        final boolean isFMI1 = "1.0".equals(modelDescription.fmiVersion);
        final boolean isFMI2 = "2.0".equals(modelDescription.fmiVersion);
        final boolean isFMI3 = !(isFMI1 || isFMI2);

        // structural parameters
        ArrayList<Integer> structuralParameterTypes = new ArrayList<Integer>();
        ArrayList<String> structuralParameterVRs = new ArrayList<String>();
        ArrayList<String> structuralParameterValues = new ArrayList<String>();

        // scalar start values
        ArrayList<Integer> scalarStartSizes = new ArrayList<Integer>();
        ArrayList<Integer> scalarStartTypes = new ArrayList<Integer>();
        ArrayList<String> scalarStartVRs = new ArrayList<String>();
        ArrayList<String> scalarStartValues = new ArrayList<String>();

        // string start values
        ArrayList<Integer> stringStartSizes = new ArrayList<Integer>();
        ArrayList<String> stringStartVRs = new ArrayList<String>();
        ArrayList<String> stringStartValues = new ArrayList<String>();

        for (ScalarVariable variable : modelDescription.scalarVariables) {

            if (!startValues.containsKey(variable.name)) {
                continue;
            }

            int variableSize = getVariableSize(variable);
            String startValue = startValues.get(variable.name);

            if ("structuralParameter".equals(variable.causality)) {
                structuralParameterTypes.add(Util.typeEnumForName(variable.type));
                structuralParameterVRs.add(variable.valueReference);
                structuralParameterValues.add(startValue);
            } else if ("String".equals(variable.type)) {
                stringStartSizes.add(variableSize);
                stringStartVRs.add(variable.valueReference);
                stringStartValues.add("'" + startValue + "'");
            } else {
                scalarStartSizes.add(variableSize);
                scalarStartTypes.add(Util.typeEnumForName(variable.type));
                scalarStartVRs.add(variable.valueReference);
                scalarStartValues.add(startValue);
            }
        }

        // input ports
        ArrayList<Integer> inputPortWidths = new ArrayList<Integer>();
        ArrayList<Integer> inputPortTypes = new ArrayList<Integer>();
        ArrayList<String> inputPortVariableVRs = new ArrayList<String>();

        List<List<ScalarVariable>> inputPorts = getInputPorts();

        for (List<ScalarVariable> inputPort : inputPorts) {

            ScalarVariable firstVariable = inputPort.get(0);

            int inputPortWidth;

            if (isFMI1 || isFMI2) {
                inputPortWidth = inputPort.size();
            } else {
                inputPortWidth = getVariableSize(firstVariable);
            }

            inputPortWidths.add(inputPortWidth);

            inputPortTypes.add(Util.typeEnumForName(firstVariable.type));

            for (ScalarVariable variable : inputPort) {
                inputPortVariableVRs.add(variable.valueReference);
            }
        }

        // output ports
        List<List<ScalarVariable>> outputPorts = getOutputPorts();
        ArrayList<Integer> outportWidths = new ArrayList<Integer>();
        ArrayList<Integer> outportPortTypes = new ArrayList<Integer>();
        ArrayList<String> outputPortVariableVRs = new ArrayList<String>();

        for (List<ScalarVariable> outputPort : outputPorts) {

            ScalarVariable firstVariable = outputPort.get(0);
            int portWidth;

            if (isFMI1 || isFMI2) {
                portWidth = outputPort.size();
            } else {
                portWidth = getVariableSize(firstVariable);
            }

            outportWidths.add(portWidth);

            outportPortTypes.add(Util.typeEnumForName(firstVariable.type));

            for (ScalarVariable variable : outputPort) {
                outputPortVariableVRs.add(variable.valueReference);
            }
        }

        int runAsKind = cmbbxRunAsKind.getSelectedIndex();
        boolean isModelExchange = runAsKind == 0;

        ArrayList<String> params = new ArrayList<String>();

        // FMI version
        params.add(modelDescription.fmiVersion.substring(0, 3));

        // interface type
        params.add(Integer.toString(runAsKind));

        // GUID
        params.add("'" + modelDescription.guid + "'");

        // model identifier
        params.add("'" + getModelIdentifier() + "'");

        // unzip directory
        params.add("FMIKit.getUnzipDirectory(gcb)");

        // debug logging
        params.add(chckbxDebugLogging.isSelected() ? "1" : "0");

        // log FMI calls
        params.add(chckbxLogFMICalls.isSelected() ? "1" : "0");

        // log level
        params.add(Integer.toString(cmbbxLogLevel.getSelectedIndex()));

        // log file
        if (chckbxLogToFile.isSelected()) {
            params.add("'" + txtLogFile.getText() + "'");
        } else {
            params.add("''");
        }

        // relative tolerance
        if (isModelExchange) {
            params.add("FMIKit.getSolverRelativeTolerance(bdroot(gcb))");
        } else {
            params.add(txtRelativeTolerance.getText());
        }

        // sample time
        params.add(txtSampleTime.getText());

        // offset time
        params.add("0");

        params.add(Integer.toString(modelDescription.numberOfContinuousStates));
        params.add(Integer.toString(modelDescription.numberOfEventIndicators));

        // structural parameters
        params.add("[" + Util.join(structuralParameterTypes, " ") + "]");
        params.add("[" + Util.join(structuralParameterVRs, " ") + "]");
        params.add("[" + Util.join(structuralParameterValues, " ") + "]");

        // scalar start values
        params.add("[" + Util.join(scalarStartSizes, " ") + "]");
        params.add("[" + Util.join(scalarStartTypes, " ") + "]");
        params.add("[" + Util.join(scalarStartVRs, " ") + "]");
        params.add("[" + Util.join(scalarStartValues, " ") + "]");

        // string start values
        params.add("[" + Util.join(stringStartSizes, " ") + "]");
        params.add("[" + Util.join(stringStartVRs, " ") + "]");
        params.add("char({" + Util.join(stringStartValues, " ") + "})");

        List<Boolean> inputPortDirectFeedThrough = getInputPortDirectFeedThrough(inputPorts, outputPorts);

        ArrayList<Double> inputPortDirectFeedThroughDouble = new ArrayList<Double>();
        for (Boolean value : inputPortDirectFeedThrough) {
            inputPortDirectFeedThroughDouble.add(value ? 1.0 : 0.0);
        }

        // input port widths
        params.add("[" + Util.join(inputPortWidths, " ") + "]");

        // input port direct feed through
        if (isModelExchange) {
            params.add("[" + Util.join(inputPortDirectFeedThroughDouble, " ") + "]");
        } else {
            params.add("[" + Util.join(Collections.nCopies(inputPorts.size(), "0"), " ") + "]");
        }

        // input port types
        params.add("[" + Util.join(inputPortTypes, " ") + "]");

        // input port variable VRs
        params.add("[" + Util.join(inputPortVariableVRs, " ") + "]");

        // output port widths
        params.add("[" + Util.join(outportWidths, " ") + "]");

        // output port types
        params.add("[" + Util.join(outportPortTypes, " ") + "]");

        // output port variable VRs
        params.add("[" + Util.join(outputPortVariableVRs, " ") + "]");

        return Util.join(params, " ");
    }

    /**
     * Calculate the variable size for an array variable in FMI 3.0
     */
    private int getVariableSize(ScalarVariable variable) {

        int size = 1;

        for (Object object : variable.dimensions) {
            int dimension;
            if (object instanceof Integer) {
                dimension = (Integer) object;
            } else {
                ScalarVariable dimensionVariable = (ScalarVariable) object;
                if (startValues.containsKey(dimensionVariable.name)) {
                    dimension = Integer.parseInt(startValues.get(dimensionVariable.name));
                } else {
                    dimension = Integer.parseInt(dimensionVariable.startValue);
                }
            }
            size *= dimension;
        }

        return size;
    }

    /**
     * Determine based on the variable's names if they belong to the same array. Example:
     * <p>
     * a.b[1] & a.b[2] --> true
     * a.b[1] & a.c[2] --> false
     *
     * @param variable
     * @param other    may be null
     * @return true if variable belongs to the same array as other, false otherwise
     */
    public static boolean belongsToSameArray(ScalarVariable variable, ScalarVariable other) {

        // TODO: add support for der(...)?

        if (variable.name.endsWith("]")) {
            String name = variable.name.substring(0, variable.name.lastIndexOf("["));
            if (other != null && other.name.startsWith(name)) {
                return true;
            }
        }
        return false;
    }

    public static void insert(DefaultMutableTreeNode parent, DefaultMutableTreeNode child) {

        int i = 0;

        for (; i < parent.getChildCount(); i++) {
            DefaultMutableTreeNode sibling = (DefaultMutableTreeNode) parent.getChildAt(i);
            Object siblingUserObject = sibling.getUserObject();

            if (child.getUserObject() instanceof ScalarVariable && siblingUserObject instanceof String) {
                break;
            } else if (child.getUserObject() instanceof String && siblingUserObject instanceof ScalarVariable) {
                continue;
            }

            String siblingName = getName(siblingUserObject);
            String childName = getName(child.getUserObject());
            if (childName.compareToIgnoreCase(siblingName) < 0)
                break;
        }

        parent.insert(child, i);
    }

    public static DefaultMutableTreeNode createOutportTree(List<ScalarVariable> scalarVariables) {

        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Root");
        DefaultMutableTreeNode lastOutport = null;

        for (ScalarVariable variable : scalarVariables) {

            if ("Binary".equals(variable.type) || "Clock".equals(variable.type) || "String".equals(variable.type)) continue;

            if ("output".equals(variable.causality)) {
                if (variable.name.endsWith("]")) {
                    String name = variable.name.substring(0, variable.name.lastIndexOf("["));
                    if (lastOutport == null || !name.equals(lastOutport.getUserObject())) {
                        lastOutport = new DefaultMutableTreeNode(name);
                        root.add(lastOutport);
                    }
                    lastOutport.add(new DefaultMutableTreeNode(variable));
                } else {
                    DefaultMutableTreeNode outputPortNode = new DefaultMutableTreeNode(variable.name);
                    root.add(outputPortNode);
                    outputPortNode.add(new DefaultMutableTreeNode(variable));
                }
            }
        }

        return root;
    }

    public static DefaultMutableTreeNode createTree(List<ScalarVariable> scalarVariables) {

        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Root");

        for (ScalarVariable sv : scalarVariables) {

            String path = sv.name;

            if (path.startsWith("der(") && path.endsWith(")")) {
                path = path.substring(4, path.length() - 1);
            }

            String[] segments = path.split("\\.");

            if (segments.length == 1) {
                insert(root, new DefaultMutableTreeNode(sv));
                continue;
            }

            DefaultMutableTreeNode parent = root;

            for (int i = 0; i < segments.length - 1; i++) {

                String segment = segments[i];

                DefaultMutableTreeNode group = null;

                for (int k = 0; k < parent.getChildCount(); k++) {
                    DefaultMutableTreeNode childAt = (DefaultMutableTreeNode) parent.getChildAt(k);
                    String name = getName(childAt.getUserObject());
                    if (name.equals(segment)) {
                        group = childAt;
                        break;
                    }
                }

                if (group == null) {
                    group = new DefaultMutableTreeNode(segment);
                    insert(parent, group);
                }

                parent = group;
            }

            insert(parent, new DefaultMutableTreeNode(sv));

        }

        return root;
    }

    public void loadFMU(boolean showDialog) {

        logDebug("Loading FMU...");

        File fmuFile;

        if (showDialog) {
            JFileChooser fc = new JFileChooser();

            if (mdlDirectory != null) {
                fc.setCurrentDirectory(new File(mdlDirectory));
            }

            fc.setFileFilter(new FileNameExtensionFilter("FMUs (*.fmu)", "fmu"));

            if (fc.showOpenDialog(this) != JFileChooser.APPROVE_OPTION) {
                return;
            }

            fmuFile = fc.getSelectedFile();

            String absoluteFMUPath = fmuFile.getAbsolutePath();
            String relativeFMUPath = Util.getRelativePath(absoluteFMUPath, mdlDirectory);

            txtFMUPath.setText(relativeFMUPath);
        } else {
            fmuFile = new File(txtFMUPath.getText());
        }

        logDebug("FMU file is " + fmuFile);

        // if the unzip directory is empty, extract the FMU next to the model
        if ("".equals(txtUnzipDirectory.getText().trim())) {
            String modelName = fmuFile.getName();
            modelName = modelName.substring(0, modelName.lastIndexOf('.'));
            txtUnzipDirectory.setText(modelName);

            logDebug("Unzip directory not set. Setting unzip directory to " + modelName);
        }

        String unzipdir = getUnzipDirectory();

        logDebug("Unzip directory is " + unzipdir);

        try {
            String absoluteFMUPath = getAbsolutePath(txtFMUPath.getText());
            logDebug("Unzipping " + absoluteFMUPath + " to " + unzipdir);
            Util.unzip(absoluteFMUPath, unzipdir);
        } catch (IOException e) {
            JOptionPane.showMessageDialog(this, "The FMU could not be un-zipped. See MATLAB console for details.", "Failed to unzip FMU",
                    JOptionPane.ERROR_MESSAGE);
            FMUBlockDialog.logError("The FMU could not be unzipped. " + e.getMessage());
            return;
        }

        // show model.png
        try {
            File modelImageFile = new File(unzipdir, "model.png").getCanonicalFile();
            BufferedImage modelImage = ImageIO.read(modelImageFile);
            ImageIcon modelImageIcon = scaleImage(new ImageIcon(modelImage), 270, 270);
            lblModelImage.setEnabled(true);
            lblModelImage.setIcon(modelImageIcon);
            lblModelImage.setText(null);
            String url = modelImageFile.toURI().toURL().toString();
            lblModelImage.setToolTipText("<html><img src=\"" + url + "\"/></html>");
        } catch (IOException e) {
            lblModelImage.setEnabled(false);
            lblModelImage.setIcon(null);
            lblModelImage.setText("no image available");
            lblModelImage.setToolTipText(null);
        }

        loadModelDescription();
    }

    private static ImageIcon scaleImage(ImageIcon icon, int w, int h) {

        int nw = icon.getIconWidth();
        int nh = icon.getIconHeight();

        if (icon.getIconWidth() > w) {
            nw = w;
            nh = (nw * icon.getIconHeight()) / icon.getIconWidth();
        }

        if (nh > h) {
            nh = h;
            nw = (icon.getIconWidth() * nh) / icon.getIconHeight();
        }

        return new ImageIcon(icon.getImage().getScaledInstance(nw, nh, Image.SCALE_SMOOTH));
    }

    private List<String> getPlatforms() {

        String unzipdir = getUnzipDirectory();

        ArrayList<String> platforms = new ArrayList<String>();

        Implementation implementation = modelDescription.coSimulation != null ? modelDescription.coSimulation : modelDescription.modelExchange;

        if (!implementation.sourceFiles.isEmpty()) {

            boolean hasSources = true;

            for (String sourceFile : implementation.sourceFiles) {
                if (!Util.isFile(unzipdir, "sources", sourceFile)) {
                    hasSources = false;
                    break;
                }
            }

            if (hasSources) {
                platforms.add("c-code");
            }

        }

        String modelIdentifier = implementation.modelIdentifier;

        if (Util.isFile(unzipdir, "binaries", "darwin64", modelIdentifier + ".dylib")) {
            platforms.add("darwin64");
        }

        if (Util.isFile(unzipdir, "binaries", "linux32", modelIdentifier + ".so")) {
            platforms.add("linux32");
        }

        if (Util.isFile(unzipdir, "binaries", "linux64", modelIdentifier + ".so")) {
            platforms.add("linux64");
        }

        if (Util.isFile(unzipdir, "binaries", "win32", modelIdentifier + ".dll")) {
            platforms.add("win32");
        }

        if (Util.isFile(unzipdir, "binaries", "win64", modelIdentifier + ".dll")) {
            platforms.add("win64");
        }

        return platforms;
    }

    public void logDebug(String message) {
        if (debugLogging) {
            System.out.println("DEBUG: " + message);
        }
    }

    public void warning(String message) {
        System.out.println("WARNING: " + message);
        lblMessageIcon.setIcon(new ImageIcon(FMUBlockDialog.class.getResource("/icons/warning-sign.png")));
        lblMessage.setText("See MATLAB console for warnings");
    }

    public static void logError(String message) {
        System.out.println("ERROR: " + message);
    }

    public String getAbsolutePath(String path) {

        if (new File(path).isAbsolute()) {
            return path;
        }

        return mdlDirectory + File.separator + path;
    }

    public String getUnzipDirectory() {
        return getAbsolutePath(txtUnzipDirectory.getText());
    }

    public void loadModelDescription() {

        String xmlfile = Util.joinPath(getUnzipDirectory(), "modelDescription.xml");

        logDebug("Reading model description: " + xmlfile);

        ModelDescriptionReader modelDescriptionReader = new ModelDescriptionReader(xmlfile);

        try {
            modelDescription = modelDescriptionReader.readModelDescription();
        } catch (Exception e) {
            warning("Failed to read model description. " + e.getMessage());
            return;
        }

        for (String message : modelDescriptionReader.messages) {
            warning(message);
        }

        lblFmiVersion.setText(modelDescription.fmiVersion);
        lblModelName.setText(modelDescription.modelName);
        lblGenerationTool.setText(modelDescription.generationTool);
        lblGenerationDate.setText(modelDescription.generationDateAndTime);
        lblContinuousStates.setText(Integer.toString(modelDescription.numberOfContinuousStates));
        lblEventIndicators.setText(Integer.toString(modelDescription.numberOfEventIndicators));
        lblVariables.setText(Integer.toString(modelDescription.scalarVariables.size()));
        txtpnDescription.setText(modelDescription.description);

        if (userData != null) {

            if (userData.runAsKind == MODEL_EXCHANGE && modelDescription.modelExchange == null) {
                warning("Selected interface 'Model Exchange' is not supported by the FMU");
                cmbbxRunAsKind.setSelectedIndex(1);
            } else if (userData.runAsKind == CO_SIMULATION && modelDescription.coSimulation == null) {
                warning("Selected interface 'Co-Simulation' is not supported by the FMU");
                cmbbxRunAsKind.setSelectedIndex(0);
            }
        } else {
            cmbbxRunAsKind.setSelectedIndex(modelDescription.modelExchange != null ? 0 : 1);
        }

        cmbbxRunAsKind.setEnabled(modelDescription.coSimulation != null && modelDescription.modelExchange != null);

        DefaultMutableTreeNode root = FMUBlockDialog.createTree(modelDescription.scalarVariables);

        VariablesTreeTableModel myTreeTableModel = new VariablesTreeTableModel(root, startValues);

        treeTable.setTreeTableModel(myTreeTableModel);

        treeTable.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);

        if (Util.isHiDPI()) {
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.NAME_COLUMN).setPreferredWidth(280);
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.START_COLUMN).setPreferredWidth(70);
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.UNIT_COLUMN).setPreferredWidth(70);
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.DESCRIPTION_COLUMN).setPreferredWidth(600);
        } else {
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.NAME_COLUMN).setPreferredWidth(200);
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.START_COLUMN).setPreferredWidth(50);
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.UNIT_COLUMN).setPreferredWidth(50);
            treeTable.getColumnModel().getColumn(VariablesTreeTableModel.DESCRIPTION_COLUMN).setPreferredWidth(473);
        }

        treeTable.setTreeCellRenderer(new NameTreeCellRenderer());
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.START_COLUMN).setCellRenderer(new StartTableCellRenderer(startValues));
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.START_COLUMN).setCellEditor(new StartCellEditor(startValues));
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.UNIT_COLUMN).setCellRenderer(new UnitTableCellRenderer());
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.DESCRIPTION_COLUMN)
                .setCellRenderer(new DescriptionTableCellRenderer());

        // Outputs
        variablesTree.setCellRenderer(new NameTreeCellRenderer());
        variablesTree.setModel(new DefaultTreeModel(root));
        variablesTree.getSelectionModel().setSelectionMode(TreeSelectionModel.DISCONTIGUOUS_TREE_SELECTION);

        if (userData != null) {
            outportRoot = restoreOutportTree();
        } else {
            outportRoot = createOutportTree(modelDescription.scalarVariables);
        }

        outportsTree.setCellRenderer(new NameTreeCellRenderer() {

            @Override
            public Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel, boolean expanded, boolean leaf, int row,
                                                          boolean hasFocus) {
                Component component = super.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);

                NameTreeCellRenderer renderer = (NameTreeCellRenderer) super.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);

                Object userObject = ((DefaultMutableTreeNode) value).getUserObject();

                if (userObject instanceof String) {
                    renderer.setIcon(outportIcon);
                }

                return component;
            }

        });

        outportTreeModel = new DefaultTreeModel(outportRoot);
        outportsTree.setModel(outportTreeModel);
        outportsTree.setEditable(true);
        outportsTree.setCellEditor(new DefaultTreeCellEditor(outportsTree, (DefaultTreeCellRenderer) outportsTree.getCellRenderer()) {

            @Override
            public boolean isCellEditable(EventObject event) {
                if (event instanceof MouseEvent) {
                    return false; // don't edit on click
                }
                TreePath selectionPath = outportsTree.getSelectionPath();
                DefaultMutableTreeNode treeNode = (DefaultMutableTreeNode) selectionPath.getLastPathComponent();
                return treeNode.getUserObject() instanceof String;
            }

        });

        // set the default icons (visible when editing the output port names)
        DefaultTreeCellRenderer renderer = (DefaultTreeCellRenderer) outportsTree.getCellRenderer();
        renderer.setOpenIcon(outportIcon);
        renderer.setClosedIcon(outportIcon);

        chckbxUseSourceCode.setEnabled(canUseSourceCode());

        // documentation
        htmlFile = new File(Util.joinPath(getUnzipDirectory(), "documentation",
                "1.0".equals(modelDescription.fmiVersion) ? "_main.html" : "index.html"));

        if (htmlFile.isFile()) {
            lblDocumentation.setText("<html><a href=\"#\">Open in browser...</a></html>");
            lblDocumentation.setToolTipText(htmlFile.getAbsolutePath());
            lblDocumentation.setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
        } else {
            lblDocumentation.setText("not available");
            lblDocumentation.setToolTipText(null);
            lblDocumentation.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }

        // advanced
        if (userData != null) {
            txtRelativeTolerance.setText(userData.relativeTolerance);
        } else if (modelDescription.defaultExperiment != null && modelDescription.defaultExperiment.tolerance != null) {
            txtRelativeTolerance.setText(modelDescription.defaultExperiment.tolerance);
        }

        // update the platforms
        lblPlatforms.setText(Util.join(getPlatforms(), ", "));

        // enable the dialog buttons
        btnApply.setEnabled(true);
        btnOK.setEnabled(true);
    }

    public boolean canUseSourceCode() {
        return !getImplementation().sourceFiles.isEmpty();
    }

    private Implementation getImplementation() {
        return cmbbxRunAsKind.getSelectedIndex() == MODEL_EXCHANGE ? modelDescription.modelExchange : modelDescription.coSimulation;
    }

    public void checkFMUModified() {
        if (userData != null) {
            if (getFMULastModified() > userData.fmuLastModified) {
                int result = JOptionPane.showConfirmDialog(this, "The FMU has changed on disk. Do you want to reload?", "Reload FMU?",
                        JOptionPane.YES_NO_OPTION);
                if (result == JOptionPane.YES_OPTION) {
                    loadFMU(false);
                }
            }
        }
    }

    public void generateSourceSFunction() throws FileNotFoundException {

        Implementation implementation = getImplementation();

        String modelIdentifier = implementation.modelIdentifier;

        PrintWriter w = new PrintWriter(mdlDirectory + File.separator + "sfun_" + modelIdentifier + ".c");

        final String fmiMajorVersion = modelDescription.fmiVersion.substring(0, 1);

        w.println("#define FMI_VERSION " + fmiMajorVersion);

        if (cmbbxRunAsKind.getSelectedIndex() == MODEL_EXCHANGE) {
            w.println("#define MODEL_EXCHANGE");
        } else {
            w.println("#define CO_SIMULATION");
        }

        w.println("#define FMI" + fmiMajorVersion + "_FUNCTION_PREFIX " + modelIdentifier + "_");

        w.println("#define S_FUNCTION_NAME sfun_" + modelIdentifier);
        w.println();
        w.println("#include \"sfun_fmurun.c\"");

        w.close();
    }

    public List<String> getSourceFiles() {

        ArrayList<String> sourceFiles = new ArrayList<String>();

        Implementation implementation = getImplementation();

        for (String sourceFile : implementation.sourceFiles) {

            if ("all.c".equals(sourceFile)) {

                    final File oldFile = new File(Util.joinPath(getUnzipDirectory(), "sources", "all.c"));
                    final String newFilename = "all_" + implementation.modelIdentifier + ".c";
                    final File newFile = new File(Util.joinPath(getUnzipDirectory(), "sources", newFilename));

                    // rename all.c to avoid name clashes
                    if (oldFile.exists() && !newFile.exists()) {
                        oldFile.renameTo(newFile);
                    }

                    sourceFile = newFilename;
            }

            sourceFiles.add(sourceFile);
        }

        return sourceFiles;
    }

    public DefaultMutableTreeNode restoreOutportTree() {

        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Root");

        HashMap<String, ScalarVariable> scalarVariables = new HashMap<String, ScalarVariable>();

        for (ScalarVariable sv : modelDescription.scalarVariables) {
            scalarVariables.put(sv.name, sv);
        }

        for (UserData.Port outputPort : userData.outputPorts) {

            DefaultMutableTreeNode outputPortNode = new DefaultMutableTreeNode(outputPort.label);

            for (String variableName : outputPort.variables) {
                if (scalarVariables.containsKey(variableName)) {
                    ScalarVariable sv = scalarVariables.get(variableName);
                    outputPortNode.add(new DefaultMutableTreeNode(sv));
                } else {
                    warning("The variable " + variableName + " has been removed from the output ports because is does not exist in the FMU");
                }
            }

            if (outputPortNode.getChildCount() > 0) {
                root.add(outputPortNode);
            }
        }

        return root;
    }

    public String getStartValue(String variableName) {

        ScalarVariable variable = modelDescription.getScalarVariable(variableName);

        if (variable == null) {
            throw new RuntimeException("Variable '" + variableName + "' does not exist");
        }

        if (startValues.containsKey(variableName)) {
            return startValues.get(variableName);
        } else {
            return variable.startValue;
        }
    }

    public void setStartValue(String variableName, String startValue) {

        ScalarVariable variable = modelDescription.getScalarVariable(variableName);

        if (variable == null) {
            throw new RuntimeException("Variable '" + variableName + "' does not exist");
        }

        if (startValue == null) {
            startValues.remove(variableName);
        } else {
            startValues.put(variableName, startValue);
        }
    }


    {
// GUI initializer generated by IntelliJ IDEA GUI Designer
// >>> IMPORTANT!! <<<
// DO NOT EDIT OR ADD ANY CODE HERE!
        $$$setupUI$$$();
    }

    /**
     * Method generated by IntelliJ IDEA GUI Designer
     * >>> IMPORTANT!! <<<
     * DO NOT edit this method OR call it in your code!
     *
     * @noinspection ALL
     */
    private void $$$setupUI$$$() {
        contentPane = new JPanel();
        contentPane.setLayout(new GridLayoutManager(2, 1, new Insets(10, 10, 10, 10), -1, 10));
        final JPanel panel1 = new JPanel();
        panel1.setLayout(new GridLayoutManager(1, 4, new Insets(0, 0, 0, 0), -1, -1));
        contentPane.add(panel1, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, 1, null, null, null, 0, false));
        final Spacer spacer1 = new Spacer();
        panel1.add(spacer1, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        final JPanel panel2 = new JPanel();
        panel2.setLayout(new GridLayoutManager(1, 4, new Insets(0, 0, 0, 0), -1, -1));
        panel1.add(panel2, new GridConstraints(0, 3, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        cancelButton = new JButton();
        cancelButton.setText("Cancel");
        panel2.add(cancelButton, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(90, 25), null, 0, false));
        btnApply = new JButton();
        btnApply.setText("Apply");
        panel2.add(btnApply, new GridConstraints(0, 3, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(90, 25), null, 0, false));
        btnOK = new JButton();
        btnOK.setText("OK");
        panel2.add(btnOK, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(90, 25), null, 0, false));
        btnHelp = new JButton();
        btnHelp.setText("Help");
        panel2.add(btnHelp, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(90, 25), null, 0, false));
        lblMessageIcon = new JLabel();
        lblMessageIcon.setText("");
        panel1.add(lblMessageIcon, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblMessage = new JLabel();
        lblMessage.setText("");
        panel1.add(lblMessage, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JPanel panel3 = new JPanel();
        panel3.setLayout(new GridLayoutManager(1, 1, new Insets(0, 0, 0, 0), -1, -1));
        contentPane.add(panel3, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        tabbedPane = new JTabbedPane();
        tabbedPane.setOpaque(false);
        panel3.add(tabbedPane, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, new Dimension(200, 200), null, 0, false));
        final JPanel panel4 = new JPanel();
        panel4.setLayout(new GridLayoutManager(12, 3, new Insets(15, 15, 15, 15), 15, 15));
        panel4.setOpaque(false);
        tabbedPane.addTab("Overview", panel4);
        final JLabel label1 = new JLabel();
        label1.setText("FMU:");
        panel4.add(label1, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(18, 38), null, 0, false));
        final JPanel panel5 = new JPanel();
        panel5.setLayout(new GridLayoutManager(1, 3, new Insets(0, 0, 0, 0), 5, 0));
        panel5.setOpaque(false);
        panel4.add(panel5, new GridConstraints(0, 1, 1, 2, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, new Dimension(296, 38), null, 0, false));
        txtFMUPath = new JTextField();
        panel5.add(txtFMUPath, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(150, -1), null, 0, false));
        loadButton = new JButton();
        loadButton.setText("Load");
        panel5.add(loadButton, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(65, -1), null, 0, false));
        reloadButton = new JButton();
        reloadButton.setText("Reload");
        panel5.add(reloadButton, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(65, -1), null, 0, false));
        final JLabel label2 = new JLabel();
        label2.setText("Interface:");
        panel4.add(label2, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(18, 16), null, 0, false));
        final JLabel label3 = new JLabel();
        label3.setText("FMI version:");
        panel4.add(label3, new GridConstraints(2, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label4 = new JLabel();
        label4.setText("Platforms:");
        panel4.add(label4, new GridConstraints(3, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label5 = new JLabel();
        label5.setText("Model name:");
        panel4.add(label5, new GridConstraints(4, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label6 = new JLabel();
        label6.setText("Continuous states:");
        panel4.add(label6, new GridConstraints(6, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label7 = new JLabel();
        label7.setText("Event indicators:");
        panel4.add(label7, new GridConstraints(7, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label8 = new JLabel();
        label8.setText("Variables:");
        panel4.add(label8, new GridConstraints(8, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblFmiVersion = new JLabel();
        lblFmiVersion.setText("");
        panel4.add(lblFmiVersion, new GridConstraints(2, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblPlatforms = new JLabel();
        lblPlatforms.setText("");
        panel4.add(lblPlatforms, new GridConstraints(3, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblModelName = new JLabel();
        lblModelName.setText("");
        panel4.add(lblModelName, new GridConstraints(4, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblContinuousStates = new JLabel();
        lblContinuousStates.setText("");
        panel4.add(lblContinuousStates, new GridConstraints(6, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblEventIndicators = new JLabel();
        lblEventIndicators.setText("");
        panel4.add(lblEventIndicators, new GridConstraints(7, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblVariables = new JLabel();
        lblVariables.setText("");
        panel4.add(lblVariables, new GridConstraints(8, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JPanel panel6 = new JPanel();
        panel6.setLayout(new GridLayoutManager(1, 2, new Insets(0, 0, 0, 0), -1, -1));
        panel6.setOpaque(false);
        panel4.add(panel6, new GridConstraints(1, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        cmbbxRunAsKind = new JComboBox();
        cmbbxRunAsKind.setEnabled(false);
        final DefaultComboBoxModel defaultComboBoxModel1 = new DefaultComboBoxModel();
        defaultComboBoxModel1.addElement("Model Exchange");
        defaultComboBoxModel1.addElement("Co-Simulation");
        cmbbxRunAsKind.setModel(defaultComboBoxModel1);
        panel6.add(cmbbxRunAsKind, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final Spacer spacer2 = new Spacer();
        panel6.add(spacer2, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        final JLabel label9 = new JLabel();
        label9.setText("Documentation:");
        panel4.add(label9, new GridConstraints(5, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblDocumentation = new JLabel();
        lblDocumentation.setText("");
        panel4.add(lblDocumentation, new GridConstraints(5, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        txtpnDescription = new JTextPane();
        txtpnDescription.setEditable(false);
        txtpnDescription.setOpaque(false);
        txtpnDescription.setText("");
        panel4.add(txtpnDescription, new GridConstraints(11, 1, 1, 2, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_WANT_GROW, null, new Dimension(150, 50), null, 0, false));
        final JLabel label10 = new JLabel();
        label10.setText("Description:");
        panel4.add(label10, new GridConstraints(11, 0, 1, 1, GridConstraints.ANCHOR_NORTHWEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JPanel panel7 = new JPanel();
        panel7.setLayout(new GridLayoutManager(1, 1, new Insets(0, 0, 0, 0), -1, -1));
        panel7.setOpaque(false);
        panel4.add(panel7, new GridConstraints(1, 2, 9, 1, GridConstraints.ANCHOR_EAST, GridConstraints.FILL_VERTICAL, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(280, 280), new Dimension(280, 280), new Dimension(280, 280), 0, false));
        panel7.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(), null));
        lblModelImage = new JLabel();
        lblModelImage.setEnabled(false);
        lblModelImage.setText("no image available");
        panel7.add(lblModelImage, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label11 = new JLabel();
        label11.setText("Generation tool:");
        panel4.add(label11, new GridConstraints(10, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblGenerationTool = new JLabel();
        panel4.add(lblGenerationTool, new GridConstraints(10, 1, 1, 2, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label12 = new JLabel();
        label12.setText("Generation date:");
        panel4.add(label12, new GridConstraints(9, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblGenerationDate = new JLabel();
        panel4.add(lblGenerationDate, new GridConstraints(9, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JPanel panel8 = new JPanel();
        panel8.setLayout(new GridLayoutManager(2, 1, new Insets(10, 10, 10, 10), -1, -1));
        panel8.setOpaque(false);
        tabbedPane.addTab("Variables", panel8);
        final JScrollPane scrollPane1 = new JScrollPane();
        panel8.add(scrollPane1, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        treeTable = new JXTreeTable();
        treeTable.putClientProperty("terminateEditOnFocusLost", Boolean.TRUE);
        scrollPane1.setViewportView(treeTable);
        final JPanel panel9 = new JPanel();
        panel9.setLayout(new GridLayoutManager(1, 3, new Insets(0, 0, 0, 0), -1, -1));
        panel9.setOpaque(false);
        panel8.add(panel9, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        treeTableExpandAllButton = new JButton();
        treeTableExpandAllButton.setIcon(new ImageIcon(getClass().getResource("/icons/expand.png")));
        treeTableExpandAllButton.setToolTipText("Expand all");
        panel9.add(treeTableExpandAllButton, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        final Spacer spacer3 = new Spacer();
        panel9.add(spacer3, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        treeTableCollapseAllButton = new JButton();
        treeTableCollapseAllButton.setIcon(new ImageIcon(getClass().getResource("/icons/collapse.png")));
        treeTableCollapseAllButton.setToolTipText("Collapse all");
        panel9.add(treeTableCollapseAllButton, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        final JPanel panel10 = new JPanel();
        panel10.setLayout(new GridLayoutManager(2, 3, new Insets(10, 10, 10, 10), -1, -1));
        panel10.setOpaque(false);
        tabbedPane.addTab("Outputs", panel10);
        final JScrollPane scrollPane2 = new JScrollPane();
        panel10.add(scrollPane2, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        variablesTree = new JTree();
        variablesTree.setRootVisible(false);
        variablesTree.setShowsRootHandles(true);
        scrollPane2.setViewportView(variablesTree);
        final JPanel panel11 = new JPanel();
        panel11.setLayout(new GridLayoutManager(4, 1, new Insets(0, 0, 0, 0), -1, -1));
        panel11.setOpaque(false);
        panel10.add(panel11, new GridConstraints(1, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        addOutputButton = new JButton();
        addOutputButton.setHorizontalAlignment(2);
        addOutputButton.setIcon(new ImageIcon(getClass().getResource("/icons/add.png")));
        addOutputButton.setText("Vector");
        panel11.add(addOutputButton, new GridConstraints(2, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final Spacer spacer4 = new Spacer();
        panel11.add(spacer4, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_VERTICAL, 1, GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        final Spacer spacer5 = new Spacer();
        panel11.add(spacer5, new GridConstraints(3, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_VERTICAL, 1, GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        addScalarOutputPortButton = new JButton();
        addScalarOutputPortButton.setHorizontalAlignment(2);
        addScalarOutputPortButton.setIcon(new ImageIcon(getClass().getResource("/icons/add.png")));
        addScalarOutputPortButton.setText("Scalar");
        panel11.add(addScalarOutputPortButton, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JScrollPane scrollPane3 = new JScrollPane();
        panel10.add(scrollPane3, new GridConstraints(1, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        outportsTree = new JTree();
        outportsTree.setRootVisible(false);
        outportsTree.setShowsRootHandles(true);
        scrollPane3.setViewportView(outportsTree);
        final JPanel panel12 = new JPanel();
        panel12.setLayout(new GridLayoutManager(1, 6, new Insets(0, 0, 0, 0), 5, -1));
        panel12.setOpaque(false);
        panel10.add(panel12, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        final Spacer spacer6 = new Spacer();
        panel12.add(spacer6, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        moveUpButton = new JButton();
        moveUpButton.setIcon(new ImageIcon(getClass().getResource("/icons/arrow-up.png")));
        moveUpButton.setText("");
        panel12.add(moveUpButton, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        moveDownButton = new JButton();
        moveDownButton.setIcon(new ImageIcon(getClass().getResource("/icons/arrow-down.png")));
        moveDownButton.setText("");
        panel12.add(moveDownButton, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_EAST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        editButton = new JButton();
        editButton.setIcon(new ImageIcon(getClass().getResource("/icons/edit.png")));
        editButton.setText("");
        panel12.add(editButton, new GridConstraints(0, 3, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        removeOutputPortButton = new JButton();
        removeOutputPortButton.setIcon(new ImageIcon(getClass().getResource("/icons/remove.png")));
        removeOutputPortButton.setText("");
        panel12.add(removeOutputPortButton, new GridConstraints(0, 4, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        resetOutputsButton = new JButton();
        resetOutputsButton.setIcon(new ImageIcon(getClass().getResource("/icons/reload.png")));
        resetOutputsButton.setText("");
        panel12.add(resetOutputsButton, new GridConstraints(0, 5, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        final JPanel panel13 = new JPanel();
        panel13.setLayout(new GridLayoutManager(1, 3, new Insets(0, 0, 0, 0), -1, -1));
        panel13.setOpaque(false);
        panel10.add(panel13, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        variablesTreeExpandAllButton = new JButton();
        variablesTreeExpandAllButton.setIcon(new ImageIcon(getClass().getResource("/icons/expand.png")));
        variablesTreeExpandAllButton.setToolTipText("Expand all");
        panel13.add(variablesTreeExpandAllButton, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        final Spacer spacer7 = new Spacer();
        panel13.add(spacer7, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        variablesTreeCollapseAllButton = new JButton();
        variablesTreeCollapseAllButton.setIcon(new ImageIcon(getClass().getResource("/icons/collapse.png")));
        variablesTreeCollapseAllButton.setToolTipText("Collapse all");
        panel13.add(variablesTreeCollapseAllButton, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        final JPanel panel14 = new JPanel();
        panel14.setLayout(new GridLayoutManager(10, 2, new Insets(15, 15, 15, 15), 15, 12));
        panel14.setOpaque(false);
        tabbedPane.addTab("Advanced", panel14);
        txtUnzipDirectory = new JTextField();
        panel14.add(txtUnzipDirectory, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(150, -1), null, 0, false));
        final Spacer spacer8 = new Spacer();
        panel14.add(spacer8, new GridConstraints(9, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_VERTICAL, 1, GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        final JLabel label13 = new JLabel();
        label13.setText("Unzip directory:");
        panel14.add(label13, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label14 = new JLabel();
        label14.setText("Sample time:");
        panel14.add(label14, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        txtSampleTime = new JTextField();
        txtSampleTime.setText("-1");
        panel14.add(txtSampleTime, new GridConstraints(1, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(120, -1), null, 0, false));
        chckbxUseSourceCode = new JCheckBox();
        chckbxUseSourceCode.setOpaque(false);
        chckbxUseSourceCode.setText("Use source code");
        panel14.add(chckbxUseSourceCode, new GridConstraints(8, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JPanel panel15 = new JPanel();
        panel15.setLayout(new GridLayoutManager(1, 2, new Insets(0, 0, 0, 0), -1, -1));
        panel15.setOpaque(false);
        panel14.add(panel15, new GridConstraints(3, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        cmbbxLogLevel = new JComboBox();
        final DefaultComboBoxModel defaultComboBoxModel2 = new DefaultComboBoxModel();
        defaultComboBoxModel2.addElement("Info");
        defaultComboBoxModel2.addElement("Warning");
        defaultComboBoxModel2.addElement("Discard");
        defaultComboBoxModel2.addElement("Error");
        defaultComboBoxModel2.addElement("Fatal");
        defaultComboBoxModel2.addElement("None");
        cmbbxLogLevel.setModel(defaultComboBoxModel2);
        panel15.add(cmbbxLogLevel, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final Spacer spacer9 = new Spacer();
        panel15.add(spacer9, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        chckbxDebugLogging = new JCheckBox();
        chckbxDebugLogging.setOpaque(false);
        chckbxDebugLogging.setText("Enable debug logging");
        panel14.add(chckbxDebugLogging, new GridConstraints(6, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label15 = new JLabel();
        label15.setText("Log level:");
        panel14.add(label15, new GridConstraints(3, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label16 = new JLabel();
        label16.setText("Relative tolerance:");
        panel14.add(label16, new GridConstraints(2, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        txtRelativeTolerance = new JTextField();
        txtRelativeTolerance.setText("0");
        panel14.add(txtRelativeTolerance, new GridConstraints(2, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(120, -1), null, 0, false));
        txtLogFile = new JTextField();
        txtLogFile.setEnabled(false);
        txtLogFile.setText("");
        panel14.add(txtLogFile, new GridConstraints(4, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(150, -1), null, 0, false));
        final JLabel label17 = new JLabel();
        label17.setText("Log file:");
        panel14.add(label17, new GridConstraints(4, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        chckbxLogToFile = new JCheckBox();
        chckbxLogToFile.setOpaque(false);
        chckbxLogToFile.setText("Log to file");
        panel14.add(chckbxLogToFile, new GridConstraints(5, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        chckbxLogFMICalls = new JCheckBox();
        chckbxLogFMICalls.setOpaque(false);
        chckbxLogFMICalls.setText("Log FMI calls");
        panel14.add(chckbxLogFMICalls, new GridConstraints(7, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
    }

    /**
     * @noinspection ALL
     */
    public JComponent $$$getRootComponent$$$() {
        return contentPane;
    }

}
