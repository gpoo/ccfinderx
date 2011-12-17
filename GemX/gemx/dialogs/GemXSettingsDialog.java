package gemx.dialogs;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.*;

import constants.GemXDefaultSettings;

import res.Messages;

public class GemXSettingsDialog {
	private Button ckResizeScatterPlot = null;
	private Button ckCalcFileMetricAlways = null;
	private Button ckCalcCloneMetricAlways = null;
	private customwidgets.Spinner ntipleSourceTextPane = null;
	private Button ckAllFileViewModeEnabled = null;
	private Button ckClonesetTableClickToShowPair = null;
	private Button ckResetItemInContextMenus = null;
	
	private Button okButton = null;
	private Button cancelButton = null;
	private Button initializeValuesButton = null;

	private Shell parent = null;
	private Shell shellC = null;
	private int result;
	
	private boolean valueResizeScatterPlot = true;
	private boolean valueCalcFileMetricAlways = false;
	private boolean valueCalcCloneMetricAlways = false;
	private int valueNtipleSourceTextPane = 2;
	private boolean valueAllFileViewModeEnabled = false;
	private boolean valueClonesetTableClickToShowPair = true;
	private boolean valueResetItemInContextMenus = false;
	
	public GemXSettingsDialog(Shell shell) {
		result = SWT.CANCEL;
		parent = shell;
		layoutWidgets();
		loadValue();
	}

	private void loadValue() {
		ckResizeScatterPlot.setSelection(valueResizeScatterPlot);
		ckCalcFileMetricAlways.setSelection(valueCalcFileMetricAlways);
		ckCalcCloneMetricAlways.setSelection(valueCalcCloneMetricAlways);
		ntipleSourceTextPane.setSelection(valueNtipleSourceTextPane);
		ckAllFileViewModeEnabled.setSelection(valueAllFileViewModeEnabled);
		ckClonesetTableClickToShowPair.setSelection(valueClonesetTableClickToShowPair);
		ckResetItemInContextMenus.setSelection(valueResetItemInContextMenus);
	}
	
	private void layoutWidgets() {
		shellC = new Shell(parent, SWT.DIALOG_TRIM | SWT.PRIMARY_MODAL);
		{
			GridLayout gridLayout = new GridLayout();
			gridLayout.numColumns = 3;
			gridLayout.marginHeight = 10;
			gridLayout.marginWidth = 15;
			gridLayout.horizontalSpacing = 15;
			shellC.setLayout(gridLayout);
		}
		shellC.setText(Messages.getString("gemx.GemXSettingsDialog.S_GEMX_SETTINGS")); //$NON-NLS-1$
		
		GridData gridData2;
		
		{
			ckResizeScatterPlot = new Button(shellC, SWT.CHECK);
			ckResizeScatterPlot.setText(Messages.getString("gemx.GemXSettingsDialog.S_FIT_SCATTER_PLOT_TO_WINDOW_SIZE")); //$NON-NLS-1$
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.horizontalSpan = 3;
			ckResizeScatterPlot.setLayoutData(gridData2);
			ckResizeScatterPlot.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueResizeScatterPlot = ckResizeScatterPlot.getSelection();
				}
			});
		}
		
		{
			ckCalcFileMetricAlways = new Button(shellC, SWT.CHECK);
			ckCalcFileMetricAlways.setText(Messages.getString("gemx.GemXSettingsDialog.S_CALC_FILE_METRICS_ALWAYS")); //$NON-NLS-1$
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.horizontalSpan = 3;
			ckCalcFileMetricAlways.setLayoutData(gridData2);
			ckCalcFileMetricAlways.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueCalcFileMetricAlways = ckCalcFileMetricAlways.getSelection();
				}
			});
		}
		
		{
			ckCalcCloneMetricAlways = new Button(shellC, SWT.CHECK);
			ckCalcCloneMetricAlways.setText(Messages.getString("gemx.GemXSettingsDialog.S_CALC_CLONE_METRICS_ALWAYS")); //$NON-NLS-1$
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.horizontalSpan = 3;
			ckCalcCloneMetricAlways.setLayoutData(gridData2);
			ckCalcCloneMetricAlways.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueCalcCloneMetricAlways = ckCalcCloneMetricAlways.getSelection();
				}
			});
		}
		{
			//String tooltipText = Messages.getString("gemx.CcfxSettingsDialog.S_TOOLTIP_FOR_CHUNK_SIZE"); //$NON-NLS-1$
			
			Label label = new Label(shellC, SWT.NONE);
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData2);
			label.setText(Messages.getString("gemx.GemXSettingsDialog.S_MAX_NUM_FILES_SHOWN_IN_SOURCE_TEXT_PANE")); //$NON-NLS-1$
			//label.setToolTipText(tooltipText);
			
			ntipleSourceTextPane = new customwidgets.Spinner(shellC, SWT.NONE);
			ntipleSourceTextPane.setMaximum(20);
			ntipleSourceTextPane.setMinimum(1);
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			ntipleSourceTextPane.setLayoutData(gridData2);
			//ntipleSourceTextPane.setToolTipText(tooltipText);
			ntipleSourceTextPane.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueNtipleSourceTextPane = ntipleSourceTextPane.getSelection();
				}
			});
			
			Label unit = new Label(shellC, SWT.NONE);
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			unit.setLayoutData(gridData2);
			unit.setText(Messages.getString("gemx.GemXSettingsDialog.S_IT_WILL_TAKE_EFFECT_AFTER_RESTART")); //$NON-NLS-1$
			//unit.setToolTipText(tooltipText);
		}
		{
			ckAllFileViewModeEnabled = new Button(shellC, SWT.CHECK);
			ckAllFileViewModeEnabled.setText(Messages.getString("gemx.GemXSettingsDialog.S_USE_INDEPENDENT_SOURCETEXT_MODE"));  //$NON-NLS-1$
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.horizontalSpan = 3;
			ckAllFileViewModeEnabled.setLayoutData(gridData2);
			ckAllFileViewModeEnabled.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					GemXSettingsDialog.this.valueAllFileViewModeEnabled = ckAllFileViewModeEnabled.getSelection();
				}
			});
		}
		{
			ckClonesetTableClickToShowPair = new Button(shellC, SWT.CHECK);
			ckClonesetTableClickToShowPair.setText(Messages.getString("gemx.GemXSettingsDialog.S_SHOW_CODE_FRAGMENTS_OF_A_CLONE_PAIR")); //$NON-NLS-1$
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.horizontalSpan = 3;
			ckClonesetTableClickToShowPair.setLayoutData(gridData2);
			ckClonesetTableClickToShowPair.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueClonesetTableClickToShowPair = ckClonesetTableClickToShowPair.getSelection();
				}
			});
		}
		{
			ckResetItemInContextMenus = new Button(shellC, SWT.CHECK);
			ckResetItemInContextMenus.setText(Messages.getString("gemx.GemXSettingsDialog.S_ADD_RESET_SCOPE_TO_CONTEXT_MENU")); //$NON-NLS-1$
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.horizontalSpan = 3;
			ckResetItemInContextMenus.setLayoutData(gridData2);
			ckResetItemInContextMenus.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueResetItemInContextMenus = ckResetItemInContextMenus.getSelection();
				}
			});
		}
		{
			this.initializeValuesButton = new Button(shellC, SWT.NONE);
			gridData2 = new GridData(GridData.HORIZONTAL_ALIGN_BEGINNING);
			gridData2.widthHint = 150;
			initializeValuesButton.setLayoutData(gridData2);
			initializeValuesButton.setText(Messages.getString("gemx.CcfxSettingsDialog.S_INITIALIZE_SETTINGS")); //$NON-NLS-1$
			initializeValuesButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					initializeValues();
				}
			});
			
			Composite buttonsCompo = new Composite(shellC, SWT.NONE);
			gridData2 = new GridData(GridData.HORIZONTAL_ALIGN_END);
			gridData2.horizontalSpan = 2;
			buttonsCompo.setLayoutData(gridData2);
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
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.widthHint = 80;
			okButton.setLayoutData(gridData2);
			okButton.setText(Messages.getString("gemx.CcfxSettingsDialog.S_OK")); //$NON-NLS-1$
			okButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					result = SWT.OK;
					shellC.dispose();
				}
			});
			
			cancelButton = new Button(buttonsCompo, SWT.NONE);
			gridData2 = new GridData(GridData.FILL_HORIZONTAL);
			gridData2.widthHint = 80;
			cancelButton.setLayoutData(gridData2);
			cancelButton.setText(Messages.getString("gemx.CcfxSettingsDialog.S_CANCEL")); //$NON-NLS-1$
			cancelButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					result = SWT.CANCEL;
					shellC.dispose();
				}
			});
		}
	}
	
	private void initializeValues() {
		setResizeScatterPlot(GemXDefaultSettings.initResizeScatterPlot);
		setCalcFileMetricAlways(GemXDefaultSettings.initCalcFileMetricAlways);
		setCalcCloneMetricAlways(GemXDefaultSettings.initCalcCloneMetricAlways);
		setNtipleSourceTextPane(GemXDefaultSettings.initNtipleSourceTextPane);
		setAllFileViewModeEnabled(GemXDefaultSettings.initAllFileViewModeEnabled);
		setClonesetTableClickToShowPair(GemXDefaultSettings.initClonesetTableClickToShowPair);
		setResetItemInContextMenus(GemXDefaultSettings.initResetScopeItemInContextMenus);
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

	public boolean isResizeScatterPlot() {
		return valueResizeScatterPlot;
	}

	public void setResizeScatterPlot(boolean valueResizeScatterPlot) {
		this.valueResizeScatterPlot = valueResizeScatterPlot;
		if (this.ckResizeScatterPlot != null) {
			this.ckResizeScatterPlot.setSelection(this.valueResizeScatterPlot);
		}
	}

	public boolean isCalcFileMetricAlways() {
		return valueCalcFileMetricAlways;
	}

	public void setCalcFileMetricAlways(boolean valueCalcFileMetricAlways) {
		this.valueCalcFileMetricAlways = valueCalcFileMetricAlways;
		if (this.ckCalcFileMetricAlways != null) {
			this.ckCalcFileMetricAlways.setSelection(this.valueCalcFileMetricAlways);
		}
	}

	public boolean isCalcCloneMetricAlways() {
		return valueCalcCloneMetricAlways;
	}

	public void setCalcCloneMetricAlways(boolean valueCalcCloneMetricAlways) {
		this.valueCalcCloneMetricAlways = valueCalcCloneMetricAlways;
		if (this.ckCalcCloneMetricAlways != null) {
			this.ckCalcCloneMetricAlways.setSelection(this.valueCalcCloneMetricAlways);
		}
	}

	public int getNtipleSourceTextPane() {
		return valueNtipleSourceTextPane;
	}

	public void setNtipleSourceTextPane(int valueNtipleSourceTextPane) {
		this.valueNtipleSourceTextPane = valueNtipleSourceTextPane;
		if (this.ntipleSourceTextPane != null) {
			this.ntipleSourceTextPane.setSelection(valueNtipleSourceTextPane);
		}
	}
	
	public boolean isAllFileViewModeEnabled() {
		return valueAllFileViewModeEnabled;
	}
	
	public void setAllFileViewModeEnabled(boolean valueAllFileViewModeEnabled) {
		this.valueAllFileViewModeEnabled = valueAllFileViewModeEnabled;
		if (this.ckAllFileViewModeEnabled != null) {
			this.ckAllFileViewModeEnabled.setSelection(this.valueAllFileViewModeEnabled);
		}
	}
	
	public boolean isClonesetTableClickToShowPair() {
		return valueClonesetTableClickToShowPair;
	}
	public void setClonesetTableClickToShowPair(boolean value) {
		valueClonesetTableClickToShowPair = value;
		if (ckClonesetTableClickToShowPair != null) {
			ckClonesetTableClickToShowPair.setSelection(value);
		}
	}
	
	public boolean isResetItemInContextMenus() {
		return valueResetItemInContextMenus;
	}
	public void setResetItemInContextMenus(boolean value) {
		valueResetItemInContextMenus = value;
		if (ckResetItemInContextMenus != null) {
			ckResetItemInContextMenus.setSelection(value);
		}
	}
}
