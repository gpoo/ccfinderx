package gemx.dialogs;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;

import res.Messages;

public class CloneReDetectionDialog {
	private CcfxOptionsPanel ccfxOptionsPanel = null;
	private Label identifiedSourceFiles = null;
	private Button okButton = null;
	private Button cancelButton = null;
	
	private Shell parent = null;
	private Shell shellC = null;
	private int result;
	
	private int valueIdentifiedSourceFiles = 0;
	
	public CloneReDetectionDialog(Shell shell, int preprocessorForcingMode_) {
		parent = shell;
		layoutWidgets(preprocessorForcingMode_);
		loadValue();
	}

	private void layoutWidgets(int preprocessorForcingMode_) {
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
			message.setText(Messages.getString("gemx.CloneReDetectionDialog.S_MODIFY_DETECTION_OPTIONS_BY_THIS_DIALOG")); //$NON-NLS-1$
		}
		
		{
			Label labelIdentifiedSourceFiles = new Label(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			labelIdentifiedSourceFiles.setLayoutData(gridData);
			labelIdentifiedSourceFiles.setText(Messages.getString("gemx.CloneDetectionOptionDialog.S_NUM_IDENTIFIED_SOURCE_FILES"));  //$NON-NLS-1$
			
			identifiedSourceFiles = new Label(shellC, SWT.NONE);
			gridData = new GridData(GridData.FILL_HORIZONTAL);
			identifiedSourceFiles.setLayoutData(gridData);
			identifiedSourceFiles.setText("0");  //$NON-NLS-1$
		}
		
		{
			Label separator = new Label(shellC, SWT.SEPARATOR | SWT.HORIZONTAL);
			GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
			gridData.horizontalSpan = 2;
			separator.setLayoutData(gridData);
		}
		
		ccfxOptionsPanel = new CcfxOptionsPanel(shellC, preprocessorForcingMode_);
		
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
		identifiedSourceFiles.setText(String.valueOf(valueIdentifiedSourceFiles));
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
	
	public int getIdentifiedSourceFiles() {
		return valueIdentifiedSourceFiles;
	}

	public void setIdentifiedSourceFiles(int valueIdentifiedSourceFiles) {
		this.valueIdentifiedSourceFiles = valueIdentifiedSourceFiles;
		if (this.identifiedSourceFiles != null) {
			this.identifiedSourceFiles.setText(String.valueOf(this.valueIdentifiedSourceFiles));
		}
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
