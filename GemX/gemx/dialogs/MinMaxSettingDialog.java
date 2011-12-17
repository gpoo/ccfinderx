package gemx.dialogs;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.*;

import res.Messages;

public class MinMaxSettingDialog {
	private Shell parent;
	private Shell shellC;
	private Label[] labels;
	private customwidgets.Spinner[] mins;
	private customwidgets.Spinner[] maxs;
	private boolean[] valueEditeds;
	private Button okButton;
	private Button cancelButton;
	private Button[] actionButtons;
	private int result = SWT.CANCEL;
	private int valueMins[];
	private int valueMaxs[];
	private int valueDigits[];
	private int action;
	
	public static final int ActionNone = 0;
	public static final int ActionMakeScope = 1;
	public static final int ActionAddCheckmark = 2;
	public static final int ActionSelect = 3;
	public static final int CountOfActions = ActionSelect;
	
	class SelectionAdapterAndIndex extends SelectionAdapter {
		public int index;
		
		public SelectionAdapterAndIndex(int index) {
			this.index = index;
		}
	}
	public MinMaxSettingDialog(Shell shell, int countOfValues) {
		valueMins = new int[countOfValues];
		valueMaxs = new int[countOfValues];
		valueDigits = new int[countOfValues];
		valueEditeds = new boolean[countOfValues];
		
		for (int i = 0; i < countOfValues; ++i) {
			valueMins[i] = 0;
			valueMaxs[i] = 100;
			valueDigits[i] = 0;
		}
		action = ActionMakeScope; // default action
		
		parent = shell;
		shellC = new Shell(shell, SWT.DIALOG_TRIM | SWT.PRIMARY_MODAL);
		{
			GridLayout gridLayout = new GridLayout();
			gridLayout.numColumns = 3;
			gridLayout.marginHeight = 15;
			gridLayout.marginWidth = 25;
			gridLayout.horizontalSpacing = 15;
			shellC.setLayout(gridLayout);
		}
		
		{
			Label dummy1 = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 80;
			dummy1.setLayoutData(gridData);
			
			Label minColumnLabel = new Label(shellC, SWT.NONE);
			minColumnLabel.setText(Messages.getString("gemx.MainWindow.S_Minumum")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 80;
			minColumnLabel.setLayoutData(gridData);
			Label maxColumnLabel = new Label(shellC, SWT.NONE);
			maxColumnLabel.setText(Messages.getString("gemx.MainWindow.S_Maximum")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 80;
			maxColumnLabel.setLayoutData(gridData);
		}
		
		labels = new Label[countOfValues];
		mins = new customwidgets.Spinner[countOfValues];
		maxs = new customwidgets.Spinner[countOfValues];
		
		for (int i = 0; i < countOfValues; ++i) {
			labels[i] = new Label(shellC, SWT.CHECK);
			mins[i] = new customwidgets.Spinner(shellC, SWT.RIGHT);
			mins[i].setSelection(valueMins[i]);
			mins[i].setDigits(valueDigits[i]);
			mins[i].setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
			mins[i].addSelectionListener(new SelectionAdapterAndIndex(i) {
				public void widgetSelected(SelectionEvent e) {
					valueMins[index] = mins[index].getSelection();
					valueEditeds[index] = true;
				}
			});
			maxs[i] = new customwidgets.Spinner(shellC, SWT.RIGHT);
			maxs[i].setSelection(valueMaxs[i]);
			maxs[i].setDigits(valueDigits[i]);
			maxs[i].setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_END));
			maxs[i].addSelectionListener(new SelectionAdapterAndIndex(i) {
				public void widgetSelected(SelectionEvent e) {
					valueMaxs[index] = maxs[index].getSelection();
					valueEditeds[index] = true;
				}
			});
		}
		
		{
			Group gr = new Group(shellC, SWT.NONE);
			gr.setText(Messages.getString("gemx.MinMaxSettingDialog.S_ACTION")); //$NON-NLS-1$
			gr.setLayout(new GridLayout());
			{
				GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
				gridData.horizontalSpan = 3;
				gr.setLayoutData(gridData);
			}
			
			//Composite buttonsCompo = new Composite(shellC, SWT.NONE);
			//GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			//gridData.horizontalSpan = 3;
			//buttonsCompo.setLayoutData(gridData);
			//{
			//	GridLayout gridLayout = new GridLayout();
			//	gridLayout.numColumns = 1;
			//	gridLayout.marginHeight = 0;
			//	gridLayout.marginWidth = 0;
			//	gridLayout.horizontalSpacing = 10;
			//	gridLayout.makeColumnsEqualWidth = true;
			//	buttonsCompo.setLayout(gridLayout);
			//}
			
			actionButtons = new Button[CountOfActions];
			String[] buttonTexts = new String[] {
				Messages.getString("gemx.MinMaxSettingDialog.S_MAKE_SCOPE"), //$NON-NLS-1$
				Messages.getString("gemx.MinMaxSettingDialog.S_ADD_CHECK_MARK"), //$NON-NLS-1$
				Messages.getString("gemx.MinMaxSettingDialog.S_SELECT"), //$NON-NLS-1$
			};
			
			for (int i = 0; i < CountOfActions; ++i) {
				Button button = new Button(gr, SWT.RADIO);
				actionButtons[i] = button;
				button.setText(buttonTexts[i]);
				GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
				gridData.widthHint = 120;
				button.setLayoutData(gridData);
				button.addSelectionListener(new SelectionAdapterAndIndex(i) {
					public void widgetSelected(SelectionEvent e) {
						if (((Button)e.widget).getSelection()) {
							MinMaxSettingDialog.this.action = index + 1;
						}
					}
				});
			}
			actionButtons[action - 1].setSelection(true);
		}
		
		{
			Composite buttonsCompo = new Composite(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_END);
			gridData.horizontalSpan = 3;
			buttonsCompo.setLayoutData(gridData);
			{
				GridLayout gridLayout = new GridLayout();
				gridLayout.numColumns = 2;
				gridLayout.marginHeight = 0;
				gridLayout.marginWidth = 0;
				gridLayout.horizontalSpacing = 10;
				gridLayout.makeColumnsEqualWidth = true;
				buttonsCompo.setLayout(gridLayout);
			}
			
			okButton = new Button(buttonsCompo, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 80;
			okButton.setLayoutData(gridData);
			okButton.setText(Messages.getString("gemx.CloneDetectionOptionDialog.S_OK"));  //$NON-NLS-1$
			okButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					result = SWT.OK;
					shellC.dispose();
				}
			});
			
			cancelButton = new Button(buttonsCompo, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 80;
			cancelButton.setLayoutData(gridData);
			cancelButton.setText(Messages.getString("gemx.CloneDetectionOptionDialog.S_CANCEL"));  //$NON-NLS-1$
			cancelButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					result = SWT.CANCEL;
					shellC.dispose();
				}
			});
		}
		okButton.setFocus();
	}
	
	public void setText(String text) {
		shellC.setText(text);
	}
	
	public void setName(int index, String label) {
		labels[index].setText(label);
	}
	public void setTooltip(int index, String text) {
		labels[index].setToolTipText(text);
		mins[index].setToolTipText(text);
		maxs[index].setToolTipText(text);
	}
	public void setMinMaxDigits(int index, int rangeMin, int rangeMax, int digits) {
		mins[index].setMinimum(rangeMin);
		mins[index].setMaximum(rangeMax);
		mins[index].setDigits(digits);
		mins[index].setSelection(rangeMin);
		valueMins[index] = rangeMin;
		
		maxs[index].setMinimum(rangeMin);
		maxs[index].setMaximum(rangeMax);
		maxs[index].setDigits(digits);
		maxs[index].setSelection(rangeMax);
		valueMaxs[index] = rangeMax;
		
		valueDigits[index] = digits;
	}
	
	public int getMinimum(int index) {
		return valueMins[index];
	}
	
	public int getMaximum(int index) {
		return valueMaxs[index];
	}
	
	public int getDigits(int index) {
		return valueDigits[index];
	}
	
	public boolean getValueEdited(int index) {
		return valueEditeds[index];
	}
	
	public void setAction(int action) {
		assert 1 <= action;
		assert action <= CountOfActions;
		this.action = action;
		
		if (actionButtons != null) {
			actionButtons[action - 1].setSelection(true);
		}
	}
	
	public int getAction() {
		return this.action;
	}
	
	public int open() {
		shellC.pack();
		shellC.setLocation(parent.getLocation().x
				+ (parent.getSize().x - shellC.getSize().x) / 2, parent
				.getLocation().y
				+ (parent.getSize().y - shellC.getSize().y) / 2);
		shellC.open();

		while (!shellC.isDisposed()) {
			if (!shellC.getDisplay().readAndDispatch()) {
				shellC.getDisplay().sleep();
			}
		}
		return result;
	}
	
	public static void main(String[] args) {
		Shell shell = new Shell();
		MinMaxSettingDialog dialog = new MinMaxSettingDialog(shell, 3);
		dialog.setText("Input birthday"); //$NON-NLS-1$
		dialog.setName(0, "Year"); //$NON-NLS-1$
		dialog.setMinMaxDigits(0, 1900, 2050, 0);
		dialog.setName(1, "Month"); //$NON-NLS-1$
		dialog.setMinMaxDigits(1, 1, 12, 0);
		dialog.setName(2, "Day"); //$NON-NLS-1$
		dialog.setMinMaxDigits(2, 1, 31, 0);
		int result = dialog.open();
		System.out.println("result = " + result);
	}
}

