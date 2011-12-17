package gemx.dialogs;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;

import res.Messages;

public class CcfxOptionsPanel { // package internal
	private customwidgets.Spinner minCloneLength = null;
	private customwidgets.Spinner minTKS = null;
	private Combo shaperLevel = null;
	private Button usePMatch = null;
	private Button usePrescreening = null;
	
	public static final int PreprocessorForcingModeNone = 0;
	public static final int PreprocessorForcingModeAlways = 2;
	
	private int preprocessorForcingMode = PreprocessorForcingModeNone;
	private Button/* Check */ preprocessorForced = null;
	private Combo forcedPreprocessor;
	private Label preprocessorForcedLabel = null;
	
	private Shell shellC = null;

	private int valueMinCloneLength = 50;
	private int valueMinTKS = 12;
	private int valueShaperLevel = 2;
	private boolean valueUsePMatch = true;
	private boolean valueUsePrescreening = false;
	
	private boolean valuePreprocessorForced = false;
	private String valueForcedPreprocessor = null;
	
	public CcfxOptionsPanel(Shell shell, int preprocessorForcingMode_) {
		preprocessorForcingMode = preprocessorForcingMode_;
		if (preprocessorForcingMode == PreprocessorForcingModeAlways) {
			valuePreprocessorForced = true;
		}
		shellC = shell;
		layoutWidgets();
		loadValue();
	}

	private void layoutWidgets() {
		switch (preprocessorForcingMode) {
		case PreprocessorForcingModeNone:
			break;
		case PreprocessorForcingModeAlways:
			{
				preprocessorForcedLabel = new Label(shellC, SWT.NONE);
				
				GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
				preprocessorForcedLabel.setLayoutData(gridData);
				preprocessorForcedLabel.setText(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_PREPROCESSOR")); //$NON-NLS-1$
				
				forcedPreprocessor = new Combo(shellC, SWT.NONE);
				gridData = new GridData(GridData.FILL_HORIZONTAL);
				forcedPreprocessor.setLayoutData(gridData);
				forcedPreprocessor.setEnabled(valuePreprocessorForced);
				
				forcedPreprocessor.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						valueForcedPreprocessor = forcedPreprocessor.getText();
					}
				});
			}
			break;
		}
		{
			Label labelMinimumCloneLength = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			labelMinimumCloneLength.setLayoutData(gridData);
			labelMinimumCloneLength.setText(Messages.getString("gemx.CloneDetectionOptionDialog.S_MINIMUM_CLONE_LENGTH"));  //$NON-NLS-1$
			
			minCloneLength = new customwidgets.Spinner(shellC, SWT.NONE);
			minCloneLength.setMaximum(10000);
			minCloneLength.setMinimum(10);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			minCloneLength.setLayoutData(gridData);
			minCloneLength.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueMinCloneLength = minCloneLength.getSelection();
				}
			});
		}
		
		{
			Label labelMinimumTKS = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			labelMinimumTKS.setLayoutData(gridData);
			labelMinimumTKS.setText(Messages.getString("gemx.CloneDetectionOptionDialog.S_MINIMUM_TKS")); //$NON-NLS-1$
			
			minTKS = new customwidgets.Spinner(shellC, SWT.NONE);
			minTKS.setMaximum(10000);
			minTKS.setMinimum(0);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			minTKS.setLayoutData(gridData);
			minTKS.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueMinTKS = minTKS.getSelection();
				}
			});
		}
		
		{
			Label labelShaperLevel = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			labelShaperLevel.setLayoutData(gridData);
			labelShaperLevel.setText(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_SHAPER_LEVEL")); //$NON-NLS-1$
			
			shaperLevel = new Combo(shellC, SWT.READ_ONLY);
			shaperLevel.add(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_0_DONT_USE_SHAPER")); //$NON-NLS-1$
			shaperLevel.add(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_1_EASY_SHAPER")); //$NON-NLS-1$
			shaperLevel.add(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_2_SOFT_SHAPER")); //$NON-NLS-1$
			shaperLevel.add(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_3_HARD_SHAPER")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			shaperLevel.setLayoutData(gridData);
			shaperLevel.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueShaperLevel = shaperLevel.getSelectionIndex();
				}
			});
		}

		{
			Label labelUsePMatch = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			labelUsePMatch.setLayoutData(gridData);
			labelUsePMatch.setText(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_PMATCH_APPLICATOIN")); //$NON-NLS-1$
			
			usePMatch = new Button(shellC, SWT.CHECK);
			usePMatch.setText(Messages.getString("gemx.CloneDetectionFromFileListOptionDialog.S_USE_PMATCH")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			usePMatch.setLayoutData(gridData);
			usePMatch.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueUsePMatch = usePMatch.getSelection();
				}
			});
		}
		
		{
			Label labelUsePrescreening = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			labelUsePrescreening.setLayoutData(gridData);
			labelUsePrescreening.setText(Messages.getString("gemxCcfxOptionsPanel.S_PRESCREENING_APPLICATION")); //$NON-NLS-1$
			
			usePrescreening = new Button(shellC, SWT.CHECK);
			usePrescreening.setText(Messages.getString("gemxCcfxOptionsPanel.S_USE_PRESCREENING")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			usePrescreening.setLayoutData(gridData);
			usePrescreening.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueUsePrescreening = usePrescreening.getSelection();
				}
			});
		}
	}
	
	private void loadValue() {
		minCloneLength.setSelection(valueMinCloneLength);
		minTKS.setSelection(valueMinTKS);
		shaperLevel.select(valueShaperLevel);
		usePMatch.setSelection(valueUsePMatch);
		if (preprocessorForcingMode >= PreprocessorForcingModeAlways) {
			if (preprocessorForced != null) {
				preprocessorForced.setSelection(valuePreprocessorForced);
			}
		}
	}
	
	public int getMinimumCloneLength() {
		return valueMinCloneLength;
	}

	public void setMinimumCloneLength(int valueMinCloneLength) {
		this.valueMinCloneLength = valueMinCloneLength;
		if (this.minCloneLength != null) {
			this.minCloneLength.setSelection(this.valueMinCloneLength);
		}
	}

	public int getMinimumTKS() {
		return valueMinTKS;
	}

	public void setMinimumTKS(int valueMinTKS) {
		this.valueMinTKS = valueMinTKS;
		if (this.minTKS != null) {
			this.minTKS.setSelection(this.valueMinTKS);
		}
	}
	
	public int getShaperLevel() {
		return valueShaperLevel;
	}
	
	public void setShaperLevel(int valueShaperLevel) {
		this.valueShaperLevel = valueShaperLevel;
		if (this.shaperLevel != null) {
			this.shaperLevel.select(this.valueShaperLevel);
		}
	}
	
	public boolean isUsePMatch() {
		return valueUsePMatch;
	}
	
	public void setUsePMatch(boolean valueUsePMatch) {
		this.valueUsePMatch = valueUsePMatch;
		if (this.usePMatch != null) {
			this.usePMatch.setSelection(valueUsePMatch);
		}
	}
	
	public void addForcedPreprocessorItem(String str) {
		if (preprocessorForcingMode >= PreprocessorForcingModeAlways) {
			this.forcedPreprocessor.add(str);
		}
	}
	
	public String getForcedPreprocessor() {
		if (preprocessorForcingMode >= PreprocessorForcingModeAlways) {
			return valueForcedPreprocessor;
		} else {
			return null;
		}
	}
	
	public void setForcedPreprocessor(String value) {
		switch (preprocessorForcingMode) {
		case PreprocessorForcingModeNone:
			break;
		case PreprocessorForcingModeAlways:
			if (value != null) {
				valueForcedPreprocessor = value;
				if (this.forcedPreprocessor != null) {
					this.forcedPreprocessor.setText(value);
				}
			}
			break;
		}
	}
	
	public boolean isUsePrescreening() {
		return valueUsePrescreening;
	}
	
	public void setUsePrescreening(boolean valueUsePrescreening) {
		this.valueUsePrescreening = valueUsePrescreening;
		if (this.usePrescreening != null) {
			this.usePrescreening.setSelection(valueUsePrescreening);
		}
	}
	
}

