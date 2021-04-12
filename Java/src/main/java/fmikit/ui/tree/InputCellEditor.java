package fmikit.ui.tree;

import fmikit.ScalarVariable;

import javax.swing.*;
import javax.swing.table.TableCellEditor;
import java.awt.*;
import java.util.HashSet;
import java.util.Set;

public class InputCellEditor extends DefaultCellEditor {

    Set<String> inputVariables;

    public InputCellEditor(Set<String> inputVariables) {
        super(new JCheckBox());
        this.inputVariables = inputVariables;
        JCheckBox checkBox = (JCheckBox) getComponent();
        checkBox.setHorizontalAlignment(SwingConstants.CENTER);
    }

//    @Override
//    public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
//        JCheckBox checkBox = (JCheckBox) super.getTableCellEditorComponent(table, value, isSelected, row, column);
//        return checkBox;
//    }

}