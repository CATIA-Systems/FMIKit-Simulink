package fmikit.ui.tree;


import fmikit.ScalarVariable;

import javax.swing.*;
import javax.swing.table.TableCellRenderer;
import java.awt.*;
import java.util.Set;

public class InputCellRenderer implements TableCellRenderer {

    private final JCheckBox checkBox;
    private final Set<String> inputVariables;
    private final JPanel panel;

    public InputCellRenderer(Set<String> inputVariables) {

        this.inputVariables = inputVariables;

        checkBox = new JCheckBox();
        checkBox.setHorizontalAlignment(SwingConstants.CENTER);

        panel = new JPanel();
        panel.setOpaque(false);
    }

    @Override
    public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {

        Color bg = isSelected ? table.getSelectionBackground() : table.getBackground();
        checkBox.setBackground(bg);

        if (value instanceof ScalarVariable) {

            ScalarVariable variable = (ScalarVariable)value;

            boolean input = "input".equals(variable.causality);
            boolean tunable = "tunable".equals(variable.variability);

            if (input || tunable) {
                checkBox.setEnabled(tunable);
                checkBox.setSelected(input || inputVariables.contains(variable.name));
                return checkBox;
            }

        }

        return panel;
    }
}