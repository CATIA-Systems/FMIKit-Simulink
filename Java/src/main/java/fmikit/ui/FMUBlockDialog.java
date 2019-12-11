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

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import java.util.List;


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
    private JLabel lblDescription;
    private JLabel lblGenerationTool;
    private JLabel lblGenerationDate;
    private JLabel lblContinuousStates;
    private JLabel lblEventIndicators;
    private JLabel lblVariables;
    public JButton btnApply;
    private JButton btnMoveUp;
    private JButton btnMoveDown;
    private JButton button3;
    private JButton removeOutputPortButton;
    private JButton btnResetOutputs;
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

    private ImageIcon outportIcon = new ImageIcon(getClass().getResource("/icons/outport.png"));


    public FMUBlockDialog() {

        setMinimumSize(new Dimension(850, 600));

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

        button3.addActionListener(new ActionListener() {

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
                System.out.println(dialog.getSFunctionParameters());
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

        btnMoveUp.addActionListener(new ActionListener() {

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

        btnMoveDown.addActionListener(new ActionListener() {

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

        btnResetOutputs.addActionListener(new ActionListener() {

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
        return getImplemenation().modelIdentifier;
    }

    public List<List<ScalarVariable>> getInputPorts() {

        List<List<ScalarVariable>> inputPorts = new ArrayList<List<ScalarVariable>>();
        ScalarVariable previousVariable = null;

        for (ScalarVariable variable : modelDescription.scalarVariables) {

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

        // start values
        ArrayList<Integer> scalarStartTypes = new ArrayList<Integer>();
        ArrayList<String> scalarStartVRs = new ArrayList<String>();
        ArrayList<String> scalarStartValues = new ArrayList<String>();

        ArrayList<String> stringStartVRs = new ArrayList<String>();
        ArrayList<String> stringStartValues = new ArrayList<String>();

        for (ScalarVariable variable : modelDescription.scalarVariables) {

            String startValue = variable.startValue;

            if (startValues.containsKey(variable.name)) {
                startValue = startValues.get(variable.name);
            } else {
                continue;
            }

            String valueReference = variable.valueReference;
            int type = Util.typeEnumForName(variable.type);

            // start values
            if (startValue != null) {
                if ("String".equals(variable.type)) {
                    stringStartVRs.add(valueReference);
                    stringStartValues.add("'" + startValue + "'");
                } else {
                    scalarStartTypes.add(type);
                    scalarStartVRs.add(valueReference);
                    scalarStartValues.add(startValue);
                }
            }

        }

        // input ports
        ArrayList<Integer> inputPortWidths = new ArrayList<Integer>();
        ArrayList<Integer> inputPortTypes = new ArrayList<Integer>();
        ArrayList<String> inputPortVariableVRs = new ArrayList<String>();

        List<List<ScalarVariable>> inputPorts = getInputPorts();

        for (List<ScalarVariable> inputPort : inputPorts) {

            inputPortWidths.add(inputPort.size());

            inputPortTypes.add(Util.typeEnumForName(inputPort.get(0).type));

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

            outportWidths.add(outputPort.size());

            outportPortTypes.add(Util.typeEnumForName(outputPort.get(0).type));

            for (ScalarVariable variable : outputPort) {
                outputPortVariableVRs.add(variable.valueReference);
            }
        }

        boolean generic = !chckbxUseSourceCode.isSelected();
        int runAsKind = cmbbxRunAsKind.getSelectedIndex();
        boolean isModelExchange = runAsKind == 0;

        ArrayList<String> params = new ArrayList<String>();

        if (generic) {
            params.add("'" + modelDescription.fmiVersion + "'");
            params.add(Integer.toString(runAsKind));
            params.add("'" + modelDescription.guid + "'");
            params.add("'" + getModelIdentifier() + "'");
        }

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

        if (generic) {
            params.add(Integer.toString(modelDescription.numberOfContinuousStates));
            params.add(Integer.toString(modelDescription.numberOfEventIndicators));
        }

        // start values
        params.add("[" + Util.join(scalarStartTypes, " ") + "]");
        params.add("[" + Util.join(scalarStartVRs, " ") + "]");
        params.add("[" + Util.join(scalarStartValues, " ") + "]");

        params.add("[" + Util.join(stringStartVRs, " ") + "]");
        params.add("char({" + Util.join(stringStartValues, " ") + "})");

        List<Boolean> inputPortDirectFeedThrough = getInputPortDirectFeedThrough(inputPorts, outputPorts);

        ArrayList<Double> inputPortDirectFeedThroughDouble = new ArrayList<Double>();
        for (Boolean value : inputPortDirectFeedThrough) {
            inputPortDirectFeedThroughDouble.add(value ? 1.0 : 0.0);
        }

        if (generic) {

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
        }

        return Util.join(params, " ");
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

        for (ScalarVariable sv : scalarVariables) {
            if ("output".equals(sv.causality)) {
                if (sv.name.endsWith("]")) {
                    String name = sv.name.substring(0, sv.name.lastIndexOf("["));
                    if (lastOutport == null || !name.equals(lastOutport.getUserObject())) {
                        lastOutport = new DefaultMutableTreeNode(name);
                        root.add(lastOutport);
                    }
                    lastOutport.add(new DefaultMutableTreeNode(sv));
                } else {
                    DefaultMutableTreeNode outputPortNode = new DefaultMutableTreeNode(sv.name);
                    root.add(outputPortNode);
                    outputPortNode.add(new DefaultMutableTreeNode(sv));
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

        loadModelDescription();
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
        String description = modelDescription.description;
        // clip long descriptions so it does not break the layout
        if (description != null && description.length() > 90) {
            description = description.substring(0, 90) + "...";
            lblDescription.setToolTipText(modelDescription.description);
        }
        lblDescription.setText(description);
        lblGenerationTool.setText(modelDescription.generationTool);
        lblGenerationDate.setText(modelDescription.generationDateAndTime);
        lblContinuousStates.setText(Integer.toString(modelDescription.numberOfContinuousStates));
        lblEventIndicators.setText(Integer.toString(modelDescription.numberOfEventIndicators));
        lblVariables.setText(Integer.toString(modelDescription.scalarVariables.size()));

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
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.NAME_COLUMN).setPreferredWidth(200);
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.START_COLUMN).setPreferredWidth(50);
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.UNIT_COLUMN).setPreferredWidth(50);
        treeTable.getColumnModel().getColumn(VariablesTreeTableModel.DESCRIPTION_COLUMN).setPreferredWidth(473);

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
        return !getImplemenation().sourceFiles.isEmpty();
    }

    private Implementation getImplemenation() {
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

    public void generateSourceSFunction() throws FileNotFoundException, UnsupportedEncodingException {

        String fmiVersion = modelDescription.fmiVersion.substring(0, 1);

        Implementation implemenation = getImplemenation();

        String modelIdentifier = implemenation.modelIdentifier;
        String kind = implemenation instanceof ModelExchange ? "me" : "cs";

        // input ports
        ArrayList<Integer> inputPortWidths = new ArrayList<Integer>();
        ArrayList<Integer> inputPortTypes = new ArrayList<Integer>();
        ArrayList<Integer> inputPortFeedThrough = new ArrayList<Integer>();
        ArrayList<String> inputVariableVRs = new ArrayList<String>();
        ScalarVariable previousInport = null;

        for (ScalarVariable sv : modelDescription.scalarVariables) {

            if ("input".equals(sv.causality)) {

                if (belongsToSameArray(sv, previousInport)) {
                    int size = inputPortWidths.size();
                    Integer last = inputPortWidths.get(size - 1);
                    inputPortWidths.set(size - 1, last + 1);
                } else {
                    inputPortWidths.add(1);
                    inputPortTypes.add(Util.typeEnumForName(sv.type));
                    inputPortFeedThrough.add(1);
                }

                inputVariableVRs.add(sv.valueReference);
            }

        }

        // output ports
        ArrayList<Integer> outputPortWidths = new ArrayList<Integer>();
        ArrayList<Integer> outputPortTypes = new ArrayList<Integer>();
        ArrayList<String> outputPortVariableVRs = new ArrayList<String>();

        for (int i = 0; i < outportRoot.getChildCount(); i++) {

            DefaultMutableTreeNode outportNode = (DefaultMutableTreeNode) outportRoot.getChildAt(i);

            outputPortWidths.add(outportNode.getChildCount());

            DefaultMutableTreeNode first = (DefaultMutableTreeNode) outportNode.getChildAt(0);
            ScalarVariable varibale = (ScalarVariable) first.getUserObject();
            outputPortTypes.add(Util.typeEnumForName(varibale.type));

            for (int j = 0; j < outportNode.getChildCount(); j++) {
                DefaultMutableTreeNode scalarVariableNode = (DefaultMutableTreeNode) outportNode.getChildAt(j);
                ScalarVariable scalarVaribale = (ScalarVariable) scalarVariableNode.getUserObject();
                outputPortVariableVRs.add(scalarVaribale.valueReference);
            }

        }

        PrintWriter w = new PrintWriter(mdlDirectory + File.separator + "sfun_" + modelIdentifier + ".c");

        w.println("/* Copyright (c) 2019 Dassault Systemes. All rights reserved. */");
        w.println();
        w.println("#define MODEL_IDENTIFIER " + modelIdentifier);
        w.println("#define MODEL_GUID \"" + modelDescription.guid + "\"");
        w.println();

        w.println("#define NX " + modelDescription.numberOfContinuousStates);
        w.println("#define NZ " + modelDescription.numberOfEventIndicators);
        w.println();

        w.println("#define NU " + inputPortWidths.size());
        w.println("#define N_INPUT_VARIABLES " + inputVariableVRs.size());
        if (inputPortWidths.size() > 0) {
            w.println("#define INPUT_PORT_WIDTHS " + Util.join(inputPortWidths, ", "));
            w.println("#define INPUT_PORT_TYPES " + Util.join(inputPortTypes, ", "));
            w.println("#define INPUT_PORT_FEED_THROUGH " + Util.join(inputPortFeedThrough, ", "));
            w.println("#define INPUT_VARIABLE_VRS " + Util.join(inputVariableVRs, ", "));
        }
        w.println();

        w.println("#define NY " + outputPortWidths.size());
        if (outputPortWidths.size() > 0) {
            w.println("#define OUTPUT_PORT_WIDTHS " + Util.join(outputPortWidths, ", "));
            w.println("#define OUTPUT_PORT_TYPES " + Util.join(outputPortTypes, ", "));
            w.println("#define N_OUTPUT_VARIABLES " + outputPortVariableVRs.size());
            w.println("#define OUTPUT_VARIABLE_VRS " + Util.join(outputPortVariableVRs, ", "));
        }
        w.println();

        w.println("#define S_FUNCTION_NAME sfun_" + modelIdentifier);
        w.println("#define FMI2_FUNCTION_PREFIX " + modelIdentifier + "_");
        w.println();
        w.println("#include \"sfun_fmu_fmi" + fmiVersion + "_" + kind + ".c\"");

        w.close();
    }

    public List<String> getSourceFiles() {
        return getImplemenation().sourceFiles;
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
        panel4.setLayout(new GridLayoutManager(13, 2, new Insets(15, 15, 15, 15), 15, 15));
        panel4.setOpaque(false);
        tabbedPane.addTab("Overview", panel4);
        final JLabel label1 = new JLabel();
        label1.setText("FMU:");
        panel4.add(label1, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(18, 38), null, 0, false));
        final JPanel panel5 = new JPanel();
        panel5.setLayout(new GridLayoutManager(1, 3, new Insets(0, 0, 0, 0), 5, 0));
        panel5.setOpaque(false);
        panel4.add(panel5, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, new Dimension(296, 38), null, 0, false));
        txtFMUPath = new JTextField();
        panel5.add(txtFMUPath, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(150, -1), null, 0, false));
        loadButton = new JButton();
        loadButton.setText("Load");
        panel5.add(loadButton, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(65, -1), null, 0, false));
        reloadButton = new JButton();
        reloadButton.setText("Reload");
        panel5.add(reloadButton, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(65, -1), null, 0, false));
        final Spacer spacer2 = new Spacer();
        panel4.add(spacer2, new GridConstraints(12, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_VERTICAL, 1, GridConstraints.SIZEPOLICY_WANT_GROW, null, new Dimension(18, 14), null, 0, false));
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
        label6.setText("Description:");
        panel4.add(label6, new GridConstraints(5, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label7 = new JLabel();
        label7.setText("Generation tool:");
        panel4.add(label7, new GridConstraints(7, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label8 = new JLabel();
        label8.setText("Generation date:");
        panel4.add(label8, new GridConstraints(8, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label9 = new JLabel();
        label9.setText("Continuous states:");
        panel4.add(label9, new GridConstraints(9, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label10 = new JLabel();
        label10.setText("Event indicators:");
        panel4.add(label10, new GridConstraints(10, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label11 = new JLabel();
        label11.setText("Variables:");
        panel4.add(label11, new GridConstraints(11, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblFmiVersion = new JLabel();
        lblFmiVersion.setText("");
        panel4.add(lblFmiVersion, new GridConstraints(2, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblPlatforms = new JLabel();
        lblPlatforms.setText("");
        panel4.add(lblPlatforms, new GridConstraints(3, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblModelName = new JLabel();
        lblModelName.setText("");
        panel4.add(lblModelName, new GridConstraints(4, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblDescription = new JLabel();
        lblDescription.setText("");
        panel4.add(lblDescription, new GridConstraints(5, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblGenerationTool = new JLabel();
        lblGenerationTool.setText("");
        panel4.add(lblGenerationTool, new GridConstraints(7, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblGenerationDate = new JLabel();
        lblGenerationDate.setText("");
        panel4.add(lblGenerationDate, new GridConstraints(8, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblContinuousStates = new JLabel();
        lblContinuousStates.setText("");
        panel4.add(lblContinuousStates, new GridConstraints(9, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblEventIndicators = new JLabel();
        lblEventIndicators.setText("");
        panel4.add(lblEventIndicators, new GridConstraints(10, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblVariables = new JLabel();
        lblVariables.setText("");
        panel4.add(lblVariables, new GridConstraints(11, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
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
        final Spacer spacer3 = new Spacer();
        panel6.add(spacer3, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        final JLabel label12 = new JLabel();
        label12.setText("Documentation:");
        panel4.add(label12, new GridConstraints(6, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        lblDocumentation = new JLabel();
        lblDocumentation.setText("");
        panel4.add(lblDocumentation, new GridConstraints(6, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JPanel panel7 = new JPanel();
        panel7.setLayout(new GridLayoutManager(1, 1, new Insets(10, 10, 10, 10), -1, -1));
        panel7.setOpaque(false);
        tabbedPane.addTab("Variables", panel7);
        final JScrollPane scrollPane1 = new JScrollPane();
        panel7.add(scrollPane1, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        treeTable = new JXTreeTable();
        treeTable.putClientProperty("terminateEditOnFocusLost", Boolean.TRUE);
        scrollPane1.setViewportView(treeTable);
        final JPanel panel8 = new JPanel();
        panel8.setLayout(new GridLayoutManager(2, 3, new Insets(10, 10, 10, 10), -1, -1));
        panel8.setOpaque(false);
        tabbedPane.addTab("Outputs", panel8);
        final JScrollPane scrollPane2 = new JScrollPane();
        panel8.add(scrollPane2, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        variablesTree = new JTree();
        variablesTree.setRootVisible(false);
        variablesTree.setShowsRootHandles(true);
        scrollPane2.setViewportView(variablesTree);
        final JPanel panel9 = new JPanel();
        panel9.setLayout(new GridLayoutManager(4, 1, new Insets(0, 0, 0, 0), -1, -1));
        panel9.setOpaque(false);
        panel8.add(panel9, new GridConstraints(1, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        addOutputButton = new JButton();
        addOutputButton.setHorizontalAlignment(2);
        addOutputButton.setIcon(new ImageIcon(getClass().getResource("/icons/plus.png")));
        addOutputButton.setText("Vector");
        panel9.add(addOutputButton, new GridConstraints(2, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final Spacer spacer4 = new Spacer();
        panel9.add(spacer4, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_VERTICAL, 1, GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        final Spacer spacer5 = new Spacer();
        panel9.add(spacer5, new GridConstraints(3, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_VERTICAL, 1, GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        addScalarOutputPortButton = new JButton();
        addScalarOutputPortButton.setHorizontalAlignment(2);
        addScalarOutputPortButton.setIcon(new ImageIcon(getClass().getResource("/icons/plus.png")));
        addScalarOutputPortButton.setText("Scalar");
        panel9.add(addScalarOutputPortButton, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JScrollPane scrollPane3 = new JScrollPane();
        panel8.add(scrollPane3, new GridConstraints(1, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        outportsTree = new JTree();
        outportsTree.setRootVisible(false);
        outportsTree.setShowsRootHandles(true);
        scrollPane3.setViewportView(outportsTree);
        final JPanel panel10 = new JPanel();
        panel10.setLayout(new GridLayoutManager(1, 6, new Insets(0, 0, 0, 0), 5, -1));
        panel10.setOpaque(false);
        panel8.add(panel10, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        final Spacer spacer6 = new Spacer();
        panel10.add(spacer6, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        btnMoveUp = new JButton();
        btnMoveUp.setIcon(new ImageIcon(getClass().getResource("/icons/arrow-up.png")));
        btnMoveUp.setText("");
        panel10.add(btnMoveUp, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        btnMoveDown = new JButton();
        btnMoveDown.setIcon(new ImageIcon(getClass().getResource("/icons/arrow-down.png")));
        btnMoveDown.setText("");
        panel10.add(btnMoveDown, new GridConstraints(0, 2, 1, 1, GridConstraints.ANCHOR_EAST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        button3 = new JButton();
        button3.setIcon(new ImageIcon(getClass().getResource("/icons/pencil.png")));
        button3.setText("");
        panel10.add(button3, new GridConstraints(0, 3, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        removeOutputPortButton = new JButton();
        removeOutputPortButton.setIcon(new ImageIcon(getClass().getResource("/icons/minus.png")));
        removeOutputPortButton.setText("");
        panel10.add(removeOutputPortButton, new GridConstraints(0, 4, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        btnResetOutputs = new JButton();
        btnResetOutputs.setIcon(new ImageIcon(getClass().getResource("/icons/repeat.png")));
        btnResetOutputs.setText("");
        panel10.add(btnResetOutputs, new GridConstraints(0, 5, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, new Dimension(22, 22), new Dimension(22, 22), new Dimension(22, 22), 0, false));
        final JPanel panel11 = new JPanel();
        panel11.setLayout(new GridLayoutManager(10, 2, new Insets(15, 15, 15, 15), 15, 12));
        panel11.setOpaque(false);
        tabbedPane.addTab("Advanced", panel11);
        txtUnzipDirectory = new JTextField();
        panel11.add(txtUnzipDirectory, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(150, -1), null, 0, false));
        final Spacer spacer7 = new Spacer();
        panel11.add(spacer7, new GridConstraints(9, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_VERTICAL, 1, GridConstraints.SIZEPOLICY_WANT_GROW, null, null, null, 0, false));
        final JLabel label13 = new JLabel();
        label13.setText("Unzip directory:");
        panel11.add(label13, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label14 = new JLabel();
        label14.setText("Sample time:");
        panel11.add(label14, new GridConstraints(1, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        txtSampleTime = new JTextField();
        txtSampleTime.setText("-1");
        panel11.add(txtSampleTime, new GridConstraints(1, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(120, -1), null, 0, false));
        chckbxUseSourceCode = new JCheckBox();
        chckbxUseSourceCode.setOpaque(false);
        chckbxUseSourceCode.setText("Use source code");
        panel11.add(chckbxUseSourceCode, new GridConstraints(8, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JPanel panel12 = new JPanel();
        panel12.setLayout(new GridLayoutManager(1, 2, new Insets(0, 0, 0, 0), -1, -1));
        panel12.setOpaque(false);
        panel11.add(panel12, new GridConstraints(3, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_BOTH, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, null, null, null, 0, false));
        cmbbxLogLevel = new JComboBox();
        final DefaultComboBoxModel defaultComboBoxModel2 = new DefaultComboBoxModel();
        defaultComboBoxModel2.addElement("Info");
        defaultComboBoxModel2.addElement("Warning");
        defaultComboBoxModel2.addElement("Discard");
        defaultComboBoxModel2.addElement("Error");
        defaultComboBoxModel2.addElement("Fatal");
        defaultComboBoxModel2.addElement("None");
        cmbbxLogLevel.setModel(defaultComboBoxModel2);
        panel12.add(cmbbxLogLevel, new GridConstraints(0, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final Spacer spacer8 = new Spacer();
        panel12.add(spacer8, new GridConstraints(0, 1, 1, 1, GridConstraints.ANCHOR_CENTER, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, 1, null, null, null, 0, false));
        chckbxDebugLogging = new JCheckBox();
        chckbxDebugLogging.setOpaque(false);
        chckbxDebugLogging.setText("Enable debug logging");
        panel11.add(chckbxDebugLogging, new GridConstraints(6, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label15 = new JLabel();
        label15.setText("Log level:");
        panel11.add(label15, new GridConstraints(3, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        final JLabel label16 = new JLabel();
        label16.setText("Relative tolerance:");
        panel11.add(label16, new GridConstraints(2, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        txtRelativeTolerance = new JTextField();
        txtRelativeTolerance.setText("0");
        panel11.add(txtRelativeTolerance, new GridConstraints(2, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(120, -1), null, 0, false));
        txtLogFile = new JTextField();
        txtLogFile.setEnabled(false);
        txtLogFile.setText("");
        panel11.add(txtLogFile, new GridConstraints(4, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_HORIZONTAL, GridConstraints.SIZEPOLICY_WANT_GROW, GridConstraints.SIZEPOLICY_FIXED, null, new Dimension(150, -1), null, 0, false));
        final JLabel label17 = new JLabel();
        label17.setText("Log file:");
        panel11.add(label17, new GridConstraints(4, 0, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_FIXED, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        chckbxLogToFile = new JCheckBox();
        chckbxLogToFile.setOpaque(false);
        chckbxLogToFile.setText("Log to file");
        panel11.add(chckbxLogToFile, new GridConstraints(5, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
        chckbxLogFMICalls = new JCheckBox();
        chckbxLogFMICalls.setOpaque(false);
        chckbxLogFMICalls.setText("Log FMI calls");
        panel11.add(chckbxLogFMICalls, new GridConstraints(7, 1, 1, 1, GridConstraints.ANCHOR_WEST, GridConstraints.FILL_NONE, GridConstraints.SIZEPOLICY_CAN_SHRINK | GridConstraints.SIZEPOLICY_CAN_GROW, GridConstraints.SIZEPOLICY_FIXED, null, null, null, 0, false));
    }

    /**
     * @noinspection ALL
     */
    public JComponent $$$getRootComponent$$$() {
        return contentPane;
    }

}
