package gemx.dialogs;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.*;

import res.Messages;

public class CloneDetectionFromFileListOptionDialog {
	private Label fileListPath = null;
	private CcfxOptionsPanel ccfxOptionsPanel = null;
	private Button okButton = null;
	private Button cancelButton = null;
	
	private Shell parent = null;
	private Shell shellC = null;
	private int result;
	
	private String valueFileListPath = ""; //$NON-NLS-1$
	
	public CloneDetectionFromFileListOptionDialog(Shell shell) {
		parent = shell;
		layoutWidgets();
		loadValue();
	}

	private void layoutWidgets() {
		shellC = new Shell(parent, SWT.DIALOG_TRIM | SWT.PRIMARY_MODAL);
		{
			GridLayout gridLayout = new GridLayout();
			gridLayout.numColumns = 2;
			gridLayout.marginHeight = 10;
			gridLayout.marginWidth = 15;
			gridLayout.horizontalSpacing = 15;
			shellC.setLayout(gridLayout);
		}
		
		{
			Label message = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			message.setLayoutData(gridData);
			message.setText(Messages.getString("gemx.CloneDetectionOptionDialog.S_SPECIFY_DETECTION_OPTION_BY_THIS_DIALOG")); //$NON-NLS-1$
		}
		
		{
			Label labelFileListPath = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 100;
			labelFileListPath.setLayoutData(gridData);
			labelFileListPath.setText(Messages.getString("gemx.CloneDetectionOptionDialogs.S_FILE_LIST1")); //$NON-NLS-1$
			
			fileListPath = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.widthHint = 400;
			fileListPath.setLayoutData(gridData);
		}
		
		{
			Label separator = new Label(shellC, SWT.SEPARATOR | SWT.HORIZONTAL);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			separator.setLayoutData(gridData);
		}
		
		ccfxOptionsPanel = new CcfxOptionsPanel(shellC, CcfxOptionsPanel.PreprocessorForcingModeAlways);

		{
			Composite buttonsCompo = new Composite(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_END);
			gridData.horizontalSpan = 2;
			buttonsCompo.setLayoutData(gridData);
			{
				GridLayout gridLayout = new GridLayout();
				gridLayout.numColumns = 2;
				gridLayout.marginHeight = 5;
				gridLayout.marginWidth = 10;
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
	}
	
	private void loadValue() {
		fileListPath.setText(valueFileListPath);
	}
	
	public void setText(String text) {
		shellC.setText(text);
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

	public String getFileListPath() {
		return valueFileListPath;
	}
	
	public void setFileListPath(String fileListPath) {
		this.valueFileListPath = fileListPath;
		if (this.fileListPath != null) {
			this.fileListPath.setText(this.valueFileListPath);
		}
	}

	public int getMinimumCloneLength() {
		return ccfxOptionsPanel.getMinimumCloneLength();
	}

	public void setMinimumCloneLength(int valueMinCloneLength) {
		ccfxOptionsPanel.setMinimumCloneLength(valueMinCloneLength);
	}

	public int getMinimumTKS() {
		return ccfxOptionsPanel.getMinimumTKS();
	}

	public void setMinimumTKS(int valueMinTKS) {
		ccfxOptionsPanel.setMinimumTKS(valueMinTKS);
	}
	
	public int getShaperLevel() {
		return ccfxOptionsPanel.getShaperLevel();
	}
	
	public void setShaperLevel(int valueShaperLevel) {
		ccfxOptionsPanel.setShaperLevel(valueShaperLevel);
	}
	
	public boolean isUsePMatch() {
		return ccfxOptionsPanel.isUsePMatch();
	}
	
	public void setUsePMatch(boolean valueUsePMatch) {
		ccfxOptionsPanel.setUsePMatch(valueUsePMatch);
	}
	
	public void addForcedPreprocessorItem(String str) {
		ccfxOptionsPanel.addForcedPreprocessorItem(str);
	}
	
	public String getForcedPreprocessor() {
		return ccfxOptionsPanel.getForcedPreprocessor();
	}
	
	public void setForcedPreprocessor(String forcedPreprocessor) {
		ccfxOptionsPanel.setForcedPreprocessor(forcedPreprocessor);
	}
	
	public boolean isUsePrescreening() {
		return ccfxOptionsPanel.isUsePrescreening();
	}
	
	public void setUsePrescreening(boolean valueUsePrescreening) {
		ccfxOptionsPanel.setUsePrescreening(valueUsePrescreening);
	}
}
