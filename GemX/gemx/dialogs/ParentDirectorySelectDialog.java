package gemx.dialogs;

import java.util.ArrayList;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.*;

import res.Messages;

public class ParentDirectorySelectDialog {
	private Shell parent;
	private Shell shellC;
	
	private Button okButton;
	private Button cancelButton;
	
	private String value = null;
	private int result;

	private String[] getParentPaths(String path) {
		ArrayList<String> paths = new ArrayList<String>();
		String p = path;
		while (p.length() > 0) {
			int pos = p.lastIndexOf('\\');
			{
				int p2 = p.lastIndexOf('/');
				if (pos < 0 || p2 >= 0 && p2 < pos) {
					pos = p2;
				}
			}
			if (pos < 0) {
				break;
			}
			p = p.substring(0, pos + 1);
			paths.add(p);
			p = p.substring(0, pos);
		}
		return paths.toArray(new String[0]);
	}
	
	public ParentDirectorySelectDialog(Shell shell, String path, String terminatingDirectory) {
		parent = shell;
		shellC = new Shell(shell, SWT.DIALOG_TRIM | SWT.PRIMARY_MODAL);
		{
			GridLayout gridLayout = new GridLayout();
			gridLayout.numColumns = 1;
			gridLayout.marginHeight = 15;
			gridLayout.marginWidth = 15;
			gridLayout.horizontalSpacing = 15;
			shellC.setLayout(gridLayout);
			shellC.setText(Messages.getString("gemx.ParentDirectorySelectDialog.S_PARENT_DIRECTORY_SELECTION_GEMX")); //$NON-NLS-1$
		}
		
		String[] paths = getParentPaths(path);
		for (int i = 0; i < paths.length; ++i) {
			if (paths[i].length() < terminatingDirectory.length()) {
				break; // for i
			}
			Button button = new Button(shellC, SWT.RADIO);
			if (i == 0) {
				button.setSelection(true);
				value = paths[i];
			} else {
				button.setSelection(false);
			}
			button.setText(paths[i]);
			button.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ParentDirectorySelectDialog.this.value = ((Button)e.getSource()).getText();
				}
			});
		}
		
		{
			Composite buttonsCompo = new Composite(shellC, SWT.NONE);
			GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_END);
			gridData.horizontalSpan = 1;
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
			okButton.setText(Messages.getString("gemx.ParentDirectorySelectDialog.S_OK")); //$NON-NLS-1$
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
			cancelButton.setText(Messages.getString("gemx.ParentDirectorySelectDialog.S_CANCEL")); //$NON-NLS-1$
			cancelButton.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					result = SWT.CANCEL;
					shellC.dispose();
				}
			});
		}
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
	
	public String getChoosedPath() {
		return value;
	}
}
