package gemx.dialogs;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Composite;
import customwidgets.Spinner;

import constants.CcfxDefaultSettings;

import res.Messages;

public class CcfxSettingsDialog {
	private Spinner minimumCloneLength = null;
	private Spinner minimumTKS = null;
	private Combo preprocessor = null;
	private Spinner chunkSize = null;
	private Combo shaperLevel = null;
	private Button usePMatch = null;
	private Button usePreprocessCache = null;
	private Text encoding = null;
	private Spinner maxWorkerThreads = null;
	private Button usePrescreening = null;
	
	private Button okButton = null;
	private Button cancelButton = null;
	private Button initializeValuesButton = null;

	private Shell parent = null;
	private Shell shellC = null;
	private int result;
	
	private int valueMinimumCloneLength = 50;
	private int valueMinimumTKS = 12;
	private String valuePreprocessor = ""; //$NON-NLS-1$
	private int valueChunkSize = 60;
	private int valueShaperLevel = 1;
	private boolean valueUsePMatch = true;
	private boolean valueUsePreprocessCache = true;
	private String valueEncoding = ""; //$NON-NLS-1$
	private int valueMaxWorkerThreads = 0;
	private boolean valueUsePrescreening = false;
	
	public CcfxSettingsDialog(Shell shell) {
		result = SWT.CANCEL;
		parent = shell;
		layoutWidgets();
		loadValue();
	}
	
	private void loadValue() {
		minimumCloneLength.setSelection(valueMinimumCloneLength);
		minimumTKS.setSelection(valueMinimumTKS);
		preprocessor.setText(valuePreprocessor);
		chunkSize.setSelection(valueChunkSize);
		shaperLevel.select(valueShaperLevel);
		usePMatch.setSelection(valueUsePMatch);
		usePreprocessCache.setSelection(valueUsePreprocessCache);
		encoding.setText(valueEncoding);
		maxWorkerThreads.setSelection(valueMaxWorkerThreads);
		usePrescreening.setSelection(valueUsePrescreening);
		
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
		shellC.setText(Messages.getString("gemx.CcfxSettingsDialog.S_CCFX_SETTINGS")); //$NON-NLS-1$
		
		GridData gridData;
		
		{
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_DEFAULT_PREPROCESSOR")); //$NON-NLS-1$
			
			preprocessor = new Combo(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			preprocessor.setLayoutData(gridData);
			preprocessor.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valuePreprocessor = preprocessor.getText();
				}
			});
		}
		
		{
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 200;
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_DEFAULT_MINIMUM_CLONE_LENGTH")); //$NON-NLS-1$
			
			minimumCloneLength = new Spinner(shellC, SWT.NONE);
			minimumCloneLength.setMaximum(10000);
			minimumCloneLength.setMinimum(10);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 100;
			minimumCloneLength.setLayoutData(gridData);
			minimumCloneLength.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueMinimumCloneLength = minimumCloneLength.getSelection();
				}
			});
			
			Label unit = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 100;
			unit.setLayoutData(gridData);
			unit.setText("Token"); //$NON-NLS-1$
		}
		
		{
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 200;
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_DEFAULT_MINIMUM_TKS")); //$NON-NLS-1$
			
			minimumTKS = new Spinner(shellC, SWT.NONE);
			minimumTKS.setMaximum(10000);
			minimumTKS.setMinimum(0);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 100;
			minimumTKS.setLayoutData(gridData);
			minimumTKS.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueMinimumTKS = minimumTKS.getSelection();
				}
			});
			
			Label unit = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 100;
			unit.setLayoutData(gridData);
			unit.setText(""); //$NON-NLS-1$
		}
		
		{
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_DEFAULT_SHAPER_LEVEL")); //$NON-NLS-1$
			
			shaperLevel = new Combo(shellC, SWT.READ_ONLY);
			shaperLevel.add(Messages.getString("gemx.CcfxSettingsDialog.S_0_DONT_USE_SHAPER")); //$NON-NLS-1$
			shaperLevel.add(Messages.getString("gemx.CcfxSettingsDialog.S_1_EASY_SHAPER")); //$NON-NLS-1$
			shaperLevel.add(Messages.getString("gemx.CcfxSettingsDialog.S_2_SOFT_SHAPER")); //$NON-NLS-1$
			shaperLevel.add(Messages.getString("gemx.CcfxSettingsDialog.S_3_HARD_SHAPER")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			shaperLevel.setLayoutData(gridData);
			shaperLevel.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueShaperLevel = shaperLevel.getSelectionIndex();
				}
			});
		}
		
		{
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_DEFAULT_PMATCH_APPLICATION")); //$NON-NLS-1$
			
			usePMatch = new Button(shellC, SWT.CHECK);
			usePMatch.setText(Messages.getString("gemx.CcfxSettingsDialog.S_USE_PMATCH")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			usePMatch.setLayoutData(gridData);
			usePMatch.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueUsePMatch = usePMatch.getSelection();
				}
			});
		}
		
		{
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_DEFAULT_PRESCREENING_UTILIZATION")); //$NON-NLS-1$
			
			usePrescreening = new Button(shellC, SWT.CHECK);
			usePrescreening.setText(Messages.getString("gemx.CcfxSettingsDialog.S_USE_PRESCREENING")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			usePrescreening.setLayoutData(gridData);
			usePrescreening.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueUsePrescreening = usePrescreening.getSelection();
				}
			});
		}
		
		{
			Label label = new Label(shellC, SWT.SEPARATOR | SWT.HORIZONTAL);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 3;
			label.setLayoutData(gridData);
		}
		
		{
			Label label = new Label(shellC, SWT.NONE);
			//label.setEnabled(false); // 2007/11/16
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_PREPROCESSED_FILE_DIRECTORY")); //$NON-NLS-1$
			
			usePreprocessCache = new Button(shellC, SWT.CHECK);
			//usePreprocessCache.setEnabled(false); // 2007/11/16
			usePreprocessCache.setText(Messages.getString("gemx.CcfxSettingsDialog.S_SAVE_TO_DISTINCT_DIRECTORY")); //$NON-NLS-1$
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			usePreprocessCache.setLayoutData(gridData);
			usePreprocessCache.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueUsePreprocessCache = usePreprocessCache.getSelection();
				}
			});
		}
		
		{
			String tooltipText = Messages.getString("gemx.CcfxSettingsDialog.S_TOOLTIP_FOR_CHUNK_SIZE"); //$NON-NLS-1$
			
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_CHUNK_SIZE")); //$NON-NLS-1$
			label.setToolTipText(tooltipText);
			
			chunkSize = new Spinner(shellC, SWT.NONE);
			chunkSize.setMaximum(10000);
			chunkSize.setMinimum(10);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			chunkSize.setLayoutData(gridData);
			chunkSize.setToolTipText(tooltipText);
			chunkSize.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueChunkSize = chunkSize.getSelection();
				}
			});
			
			Label unit = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			unit.setLayoutData(gridData);
			unit.setText("MByte"); //$NON-NLS-1$
			unit.setToolTipText(tooltipText);
		}
		
		{
			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 200;
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_MAX_WORKER_THREADS")); //$NON-NLS-1$
			
			maxWorkerThreads = new Spinner(shellC, SWT.NONE);
			maxWorkerThreads.setMaximum(100);
			maxWorkerThreads.setMinimum(0);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 100;
			maxWorkerThreads.setLayoutData(gridData);
			maxWorkerThreads.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					valueMaxWorkerThreads = maxWorkerThreads.getSelection();
				}
			});
			
			Label unit = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 100;
			unit.setLayoutData(gridData);
			unit.setText(""); //$NON-NLS-1$
		}
		
		{
			String tooltipText = Messages.getString("gemx.CcfxSettingsDialog.S_TOOLTIP_FOR_ENCODING"); //$NON-NLS-1$

			Label label = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			label.setLayoutData(gridData);
			label.setText(Messages.getString("gemx.CcfxSettingsDialog.S_ENCODING")); //$NON-NLS-1$
			label.setToolTipText(tooltipText);
			
			encoding = new Text(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			encoding.setLayoutData(gridData);
			encoding.setToolTipText(tooltipText);
			encoding.addModifyListener(new ModifyListener() {
				public void modifyText(ModifyEvent arg0) {
					valueEncoding = encoding.getText();
				}
			});
		}
		
		{
			this.initializeValuesButton = new Button(shellC, SWT.NONE);
			gridData = new GridData(GridData.HORIZONTAL_ALIGN_BEGINNING);
			gridData.widthHint = 150;
			initializeValuesButton.setLayoutData(gridData);
			initializeValuesButton.setText(Messages.getString("gemx.CcfxSettingsDialog.S_INITIALIZE_SETTINGS")); //$NON-NLS-1$
			initializeValuesButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					initializeValues();
				}
			});
			
			Composite buttonsCompo = new Composite(shellC, SWT.NONE);
			gridData = new GridData(GridData.HORIZONTAL_ALIGN_END);
			gridData.horizontalSpan = 2;
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
			okButton.setText(Messages.getString("gemx.CcfxSettingsDialog.S_OK")); //$NON-NLS-1$
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
		setMinimumCloneLength(CcfxDefaultSettings.initMinimumCloneLength);
		setMinimumTKS(CcfxDefaultSettings.initMinimumTKS);
		setPreprocessor(CcfxDefaultSettings.initPreprocessor);
		setChunkSize(CcfxDefaultSettings.initChunkSize);
		setShaperLevel(CcfxDefaultSettings.initShaperLevel);
		setUsePMatch(CcfxDefaultSettings.initUsePMatch);
		setUsePreprocessCache(CcfxDefaultSettings.initUsePreprocessCache);
		setEncoding(CcfxDefaultSettings.initEncoding);
		setMaxWorkerThreads(CcfxDefaultSettings.initMaxWorkerThreads);
	}
	
	public void addPreprocessorItem(String str) {
		this.preprocessor.add(str);
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

	public int getMinimumCloneLength() {
		return valueMinimumCloneLength;
	}

	public void setMinimumCloneLength(int valueMinCloneLength) {
		this.valueMinimumCloneLength = valueMinCloneLength;
		if (this.minimumCloneLength != null) {
			this.minimumCloneLength.setSelection(this.valueMinimumCloneLength);
		}
	}

	public int getMinimumTKS() {
		return valueMinimumTKS;
	}

	public void setMinimumTKS(int valueMinTKS) {
		this.valueMinimumTKS = valueMinTKS;
		if (this.minimumTKS != null) {
			this.minimumTKS.setSelection(this.valueMinimumTKS);
		}
	}
	
	public String getPreprocessor() {
		return valuePreprocessor;
	}

	public void setPreprocessor(String valuePreprocessor) {
		this.valuePreprocessor = valuePreprocessor;
		if (this.preprocessor != null) {
			this.preprocessor.setText(valuePreprocessor);
		}
	}

	public int getChunkSize() {
		return valueChunkSize;
	}

	public void setChunkSize(int valueChunkSize) {
		this.valueChunkSize = valueChunkSize;
		if (this.chunkSize != null) {
			this.chunkSize.setSelection(valueChunkSize);
		}
	}
	
	public int getShaperLevel() {
		return valueShaperLevel;
	}

	public void setShaperLevel(int valueShaperLevel) {
		this.valueShaperLevel = valueShaperLevel;
		if (this.shaperLevel != null) {
			this.shaperLevel.select(valueShaperLevel);
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
	
	public boolean isUsePreprocessCache() {
		return valueUsePreprocessCache;
	}
	
	public void setUsePreprocessCache(boolean valueUsePreprocessCache) {
		this.valueUsePreprocessCache = valueUsePreprocessCache;
		if (this.usePreprocessCache != null) {
			this.usePreprocessCache.setSelection(valueUsePreprocessCache);
		}
	}
	
	public String getEncoding() {
		return valueEncoding;
	}

	public void setEncoding(String valueEncoding) {
		this.valueEncoding = valueEncoding;
		if (this.encoding != null) {
			this.encoding.setText(valueEncoding);
		}
	}

	public int getMaxWorkerThreads() {
		return valueMaxWorkerThreads;
	}
	
	public void setMaxWorkerThreads(int valueMaxWorkerThreads) {
		this.valueMaxWorkerThreads = valueMaxWorkerThreads;
		if (this.maxWorkerThreads != null) {
			this.maxWorkerThreads.setSelection(valueMaxWorkerThreads);
		}
	}
	public boolean isUsePrescreening() {
		return valueUsePreprocessCache;
	}
	
	public void setUsePrescreening(boolean valueUsePrescreening) {
		this.valueUsePrescreening = valueUsePrescreening;
		if (this.usePrescreening != null) {
			this.usePrescreening.setSelection(valueUsePrescreening);
		}
	}
	
}
