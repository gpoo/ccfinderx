package gemx.dialogs;

import java.io.File;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.dnd.*;

import res.Messages;

public class PreprocessScriptAndDirectoriesDialog {
	private Shell parent;
	private Shell shellC;
	private Combo comboPreprocessScript;
	private String resStringPreprocessScript;
	private List directoriesList;
	private String lastDirectory;
	private String[] resDirectories;
	private boolean valueValid;
	
	public PreprocessScriptAndDirectoriesDialog(Shell shell) {
		resStringPreprocessScript = ""; //$NON-NLS-1$
		resDirectories = new String[0];
		valueValid = false;
		
		parent = shell;

		shellC = new Shell(shell, SWT.DIALOG_TRIM | SWT.PRIMARY_MODAL);
		{
			GridLayout gridLayout = new GridLayout();
			gridLayout.numColumns = 2;
			gridLayout.marginHeight = 15;
			gridLayout.marginWidth = 15;
			gridLayout.horizontalSpacing = 15;
			shellC.setLayout(gridLayout);
		}

		{
			Label labelPreprocessScript = new Label(shellC, SWT.NULL);
			labelPreprocessScript.setText(Messages.getString("gemx.PreprocessScriptAndDirectoriesDialog.S_PREPROCESS_SCRIPT")); //$NON-NLS-1$

			comboPreprocessScript = new Combo(shellC, SWT.DROP_DOWN);
			{
				GridData gridData = new GridData(GridData.HORIZONTAL_ALIGN_BEGINNING);
				gridData.widthHint = 200;
				comboPreprocessScript.setLayoutData(gridData);
			}
			comboPreprocessScript.setFocus();
			comboPreprocessScript.addKeyListener(new KeyAdapter() {
				public void keyPressed(KeyEvent e) {
					if (e.character == SWT.CR) {
						resStringPreprocessScript = comboPreprocessScript.getText();
						shellC.dispose();
					}
				}
			});
		}
		
		Button defaultButton;
		
		{
			Label directoriesLabel = new Label(shellC, SWT.NULL);
			directoriesLabel.setText(Messages.getString("gemx.PreprocessScriptAndDirectoriesDialog.S_TARGET_DIRECTORY_LIST")); //$NON-NLS-1$
			
			Button addButton = new Button(shellC, SWT.PUSH);
			{
				GridData gridData = new GridData(GridData.FILL | GridData.HORIZONTAL_ALIGN_END);;
				gridData.widthHint = 150;
				addButton.setLayoutData(gridData);
			}
			addButton.setText(Messages.getString("gemx.PreprocessScriptAndDirectoriesDialog.S_ADD_DIRECTORY")); //$NON-NLS-1$
			addButton.addSelectionListener(new SelectionAdapter() {
				@Override
				public void widgetSelected(SelectionEvent e) {
					DirectoryDialog dialog = new DirectoryDialog(shellC);
					if (lastDirectory != null) {
						dialog.setFilterPath(lastDirectory);
					}
					dialog.setText(Messages.getString("gemx.PreprocessScriptAndDirectoriesDialog.S_GEMX_SELECT_TARGET_DIRECTORY")); //$NON-NLS-1$
					//dialog.setMessage(Messages.getString("gemx.MainWindow.S_SELECT_A_ROOT_DIRECTORY_OF_THE_TARGET_SOURCE_FILES")); //$NON-NLS-1$
					
					String path = dialog.open();
					
					if (path == null || path.length() == 0) {
						return;
					}
					directoriesList.add(path);
					lastDirectory = path;
				}
			});
			
			//Label dummyLabel = new Label(shellC, SWT.NULL);
			
			directoriesList = new List(shellC, SWT.VIRTUAL | SWT.MULTI | SWT.V_SCROLL);
			{
				GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
				gridData.widthHint = 500;
				gridData.heightHint = 100;
				gridData.horizontalIndent = 10;
				gridData.horizontalSpan = 2;
				directoriesList.setLayoutData(gridData);
			}
			Menu pmenu = new Menu(shellC, SWT.POP_UP);
			directoriesList.setMenu(pmenu);
			{
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.PreprocessScriptAndDirectoriesDialog.S_REMOVE")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						int[] selectionIndices = directoriesList.getSelectionIndices();
						directoriesList.remove(selectionIndices);
					}
				});
			}
			
			DropTarget target = new DropTarget(directoriesList, DND.DROP_DEFAULT | DND.DROP_COPY);
			FileTransfer transfer = FileTransfer.getInstance();
			Transfer[] types = new Transfer[]{transfer};
			target.setTransfer(types);
			target.addDropListener(new DropTargetAdapter() {
				@Override
				public void dragEnter(DropTargetEvent evt){
					evt.detail = DND.DROP_COPY;
				}
				
				@Override
				public void drop(DropTargetEvent evt){
					String[] directories = (String[])evt.data;
					for (String d : directories) {
						if (! new File(d).isDirectory()) {
							MessageBox mes = new MessageBox(shellC, SWT.OK | SWT.ICON_ERROR);
							mes.setText("Error - GemX"); //$NON-NLS-1$
							mes.setMessage(Messages.getString("gemx.PreprocessScriptAndDirectoriesDialog.S_DROPPED_ITEM_IS_NOT_DIRECTORY")); //$NON-NLS-1$
							mes.open();
							return;
						}
					}
					for (String d : directories) {
						directoriesList.add(d);
					}
				}
			});
			
			defaultButton = addButton;
		}
		
		{
			Composite buttonsCompo = new Composite(shellC, SWT.NONE);
			GridData gridData2 = new GridData(GridData.HORIZONTAL_ALIGN_END);
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
			Button button1 = new Button(buttonsCompo, SWT.PUSH);
			{
				GridData gridData = new GridData(GridData.FILL);
				gridData.widthHint = 100;
				button1.setLayoutData(gridData);
				button1.setText(Messages.getString("gemx.MainWindow.S_NEXT")); //$NON-NLS-1$
				button1.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						resStringPreprocessScript = comboPreprocessScript.getText();
						resDirectories = directoriesList.getItems();
						if (resDirectories == null || resDirectories.length == 0) {
							MessageBox mes = new MessageBox(shellC, SWT.OK | SWT.ICON_ERROR);
							mes.setText("Error - GemX"); //$NON-NLS-1$
							mes.setMessage(Messages.getString("gemx.PreprocessScriptAndDirectoriesDialog.S_NO_DIRECTORIES_ARE_SELECTED")); //$NON-NLS-1$
							mes.open();
							return;
						}
						valueValid = true;
						shellC.dispose();
					}
				});
			}
	
			Button button2 = new Button(buttonsCompo, SWT.PUSH);
			{
				GridData gridData = new GridData(GridData.FILL_HORIZONTAL);
				gridData.widthHint = 100;
				button2.setLayoutData(gridData);
				button2.setText(Messages.getString("gemx.MainWindow.S_CANCEL")); //$NON-NLS-1$
				button2.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						resStringPreprocessScript = ""; //$NON-NLS-1$
						resDirectories = new String[0];
						valueValid = false;
						shellC.dispose();
					}
				});
			}
		}
		
		defaultButton.setFocus();
	}

	public void setText(String str) {
		shellC.setText(str);
	}

	public String getText() {
		return shellC.getText();
	}

	public boolean open() {
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
		return valueValid;
	}

	public void addPreprocessScript(String str, int i) {
		comboPreprocessScript.add(str, i);
	}

	public void addPreprocessScript(String str) {
		comboPreprocessScript.add(str);
	}

	public void removePreprocesssScripts(int from, int to) {
		comboPreprocessScript.remove(from, to);
	}

	public void removePrprocessScript(int i) {
		comboPreprocessScript.remove(i);
	}

	public void removePreprocessScript(String str) {
		comboPreprocessScript.remove(str);
	}

	public void removeAllPreprocessScripts() {
		comboPreprocessScript.removeAll();
	}

	public void setPreprocessScript(String str) {
		comboPreprocessScript.setText(str);
	}

	public String getPreprocessScript() {
		return resStringPreprocessScript;
	}
	
	public void setLastDirectory(String lastDirectory) {
		this.lastDirectory = lastDirectory;
	}
	
	public String[] getDirectories() {
		return this.resDirectories;
	}
}
