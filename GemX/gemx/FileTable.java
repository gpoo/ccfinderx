package gemx;

import java.util.*;
import java.text.DecimalFormat;

import model.FileMetricModel;
import model.Model;
import model.SourceFile;

import org.eclipse.swt.*;
import org.eclipse.swt.dnd.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import res.Messages;
import resources.MetricColors;
import utility.StringUtil;

import customwidgets.*;

import gnu.trove.*;

public class FileTable implements FileSelectionListener {
	private Table table;
	private TableWithCheckHelper tableWithCheckHelper;

	private MainWindow mainWindow;
	private Shell shell;
	private Display display;
	private TableColumn[] cols;

	private Composite sc;
	
	private Label commonPathLabel;
	
	private FileMetricModel fileMetricModel;
	private int[] fileIndex2Id;
	private int[] fileId2Index;
	private String[] fileNames;
	public String searchingText = null;
	public int searchingIndex = -1;
	
	private DecimalFormat[] fieldFormats;
	
	public Control getControl() {
		return sc;
	}
	
	public void addListener(int eventType, Listener listener) {
		assert eventType == SWT.FocusIn;
		this.table.addListener(eventType, listener);
	}
	
	public TableWithCheckHelper getTableWithCheckHelper() {
		return tableWithCheckHelper;
	}
	
	private String selectedItemsToString() {
		String commonPath = commonPathLabel.getText();
		int[] selectedIndex = table.getSelectionIndices();
		StringBuffer buffer = new StringBuffer();
		if (fileMetricModel != null) {
			TableColumn[] columns = table.getColumns();
			for (int j = 0; j < columns.length - 1; ++j) {
				buffer.append(columns[j].getText());
				buffer.append('\t');
			}
			buffer.append(columns[columns.length - 1].getText());
			buffer.append(StringUtil.NewLineString);
			for (int i = 0; i < selectedIndex.length; ++i) {
				int index = selectedIndex[i];
				TableItem item = table.getItem(index);
				int fileID = fileIndex2Id[index];
				double[] fm = fileMetricModel.getMetricDataOfFile(fileID);
				for (int j = 0; j < columns.length; ++j) {
					if (j == 1) {
						buffer.append(commonPath);
						buffer.append(fileNames[index]);
					} else if (j >= 3) {
						int metricIndex = j - 2;
						String text = fieldFormats[metricIndex].format(fm[metricIndex]);
						buffer.append(text);
					} else {
						buffer.append(item.getText(j));
					}
					if (j < columns.length - 1) {
						buffer.append('\t');
					}
				}
				buffer.append(StringUtil.NewLineString);
			}
		} else {
			TableColumn[] columns = table.getColumns();
			for (int j = 0; j < 4 - 1; ++j) {
				buffer.append(columns[j].getText());
				buffer.append('\t');
			}
			buffer.append(columns[4 - 1].getText());
			buffer.append(StringUtil.NewLineString);
			for (int i = 0; i < selectedIndex.length; ++i) {
				int index = selectedIndex[i];
				TableItem item = table.getItem(index);
				buffer.append(item.getText(0));
				buffer.append('\t');
				buffer.append(commonPath);
				buffer.append(fileNames[index]);
				buffer.append('\t');
				buffer.append(item.getText(2));
				buffer.append('\t');
				buffer.append(item.getText(3));
				buffer.append(StringUtil.NewLineString);
			}
		}
		return buffer.toString();
	}
	
	private String selectedFilesToString() {
		String commonPath = commonPathLabel.getText();
		int[] selectedIndex = table.getSelectionIndices();
		StringBuffer buffer = new StringBuffer();
		for (int i = 0; i < selectedIndex.length; ++i) {
			int index = selectedIndex[i];
			TableItem item = table.getItem(index);
			buffer.append(commonPath);
			buffer.append(item.getText(1));
			buffer.append(StringUtil.NewLineString);
		}
		return buffer.toString();
	}
	
	private class FilesUnderDirectory extends SelectionAdapter {
		private int nthParent;
		public FilesUnderDirectory(int rad) {
			this.nthParent = rad;
		}
		@Override
		public void widgetSelected(SelectionEvent e) {
			int index = FileTable.this.table.getSelectionIndex();
			if (index >= 0) {
				FileTable.this.mainWindow.selectFilesUnderDirectory(index, nthParent);
			}
		}
	}

	private void drawCell(GC gc, int x, int y, int cellWidth, int cellHeight, Color foreground, 
			double rvalue, String text) {
		int width = (int)((cellWidth - 3) * rvalue) + 2;
		Color color = MetricColors.getColor(rvalue);
		gc.setBackground(color);
		gc.setForeground(display.getSystemColor(SWT.COLOR_GRAY));
		int height = (int)((cellHeight - 3) * 0.4 + 2);
		int cornerR = (int)(height * 0.8);
		gc.setAdvanced(true);
		if (cornerR >= 2 && gc.getAdvanced()) {
			gc.setAntialias(SWT.ON);
			gc.fillRoundRectangle(x, y + cellHeight - height - 1, width, height, cornerR, cornerR);
			gc.drawRoundRectangle(x, y + cellHeight - height - 1, width, height, cornerR, cornerR);
			gc.setAntialias(SWT.OFF);
		} else {
			gc.fillRectangle(x, y + cellHeight - height - 1, width, height);
			gc.drawRectangle(x, y + cellHeight - height - 1, width, height);
		}
		
		Point size = gc.textExtent(text);					
		int yoffset = Math.max(0, (cellHeight - size.y) / 2);
		int xoffset = Math.max(0, cellWidth - size.x - 4);
		gc.setForeground(foreground);
		gc.drawText(text, x + xoffset, y + yoffset, true);						
	}
	
	public void copyItemsToClipboard() {
		Clipboard clipboard = mainWindow.clipboard;
		clipboard.setContents(new Object[]{ FileTable.this.selectedItemsToString() }, 
				new Transfer[]{ TextTransfer.getInstance() });
	}
	
	public void selectAllItems() {
		table.selectAll();
	}
	
	public int[] getCheckedFileIndices() {
		gnu.trove.TIntArrayList checked = new gnu.trove.TIntArrayList();
		final int itemCount = table.getItemCount();
		for (int i = 0; i < itemCount; ++i) {
			TableItem item = table.getItem(i);
			if (item.getChecked()) {
				checked.add(i);
			}
		}
		return checked.toNativeArray();
	}
	
	private void buildMenuToTable(boolean bAddResetScopeItemToContextMenu) {
		Menu pmenu = new Menu(shell, SWT.POP_UP);
		table.setMenu(pmenu);

		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.M_COPY_ITEMS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					copyItemsToClipboard();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.M_COPY_FILE_PATHS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					Clipboard clipboard = mainWindow.clipboard;
					clipboard.setContents(new Object[]{ selectedFilesToString() }, 
							new Transfer[]{ TextTransfer.getInstance() });
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.M_PASTE_SELECTION")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					Clipboard clipboard = mainWindow.clipboard;
					TextTransfer textTransfer = TextTransfer.getInstance();
					String str = (String)clipboard.getContents(textTransfer);
					if(str == null) {
						return;
					}
					if (str.length() > 0 && str.charAt(0) == '#') {
						searchingId(str);
						return;
					}
					int[] selected = null;
					try {
						String[] lines = str.split(StringUtil.NewLineString);
						int[] selections = new int[lines.length];
						int count = 0;
						for (int i = 0; i < lines.length; ++i) {
							try {
								String line = lines[i];
								int tabpos = line.indexOf('\t');
								if (tabpos >= 0) {
									line = line.substring(0, tabpos);
								}
								int fileID = Integer.parseInt(line);
								int fileIndex = -1;
								if (fileID < fileId2Index.length && (fileIndex = fileId2Index[fileID]) != -1) {
									selections[count] = fileIndex;
									++count;
								}
							} catch (NumberFormatException ex) {
								// nothing to do
							}
						}
						selected = utility.ArrayUtil.slice(selections, 0, count);
					} finally {
						if (selected != null) {
							FileTable.this.mainWindow.setFileSelection(selected, null);
						} else {
							FileTable.this.mainWindow.setFileSelection(new int[] { }, null);
						}
					}
				}
			});
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitemCheck = new MenuItem(pmenu, SWT.CASCADE);
			pitemCheck.setText(Messages.getString("gemx.FileTable.M_CHECK_MARK")); //$NON-NLS-1$
			Menu pmenuCheck = new Menu(pitemCheck);
			pitemCheck.setMenu(pmenuCheck);
			
			tableWithCheckHelper.addCheckRelatedItemsToMenu(pmenuCheck);
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			Menu pmenuScope = pmenu;
			{
				MenuItem pitem = new MenuItem(pmenuScope, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.FileTable.M_FIT_SCOPE_TO_SELECTED_FLIES")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						int[] selectedIndex = table.getSelectionIndices();
						FileTable.this.mainWindow.fitScopeToFiles(selectedIndex);
					}
				});
			}
			{
				MenuItem pitem = new MenuItem(pmenuScope, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.FileTable.M_POP_SCOPE")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						FileTable.this.mainWindow.popScope();
					}
				});
			}
//			if (bAddResetScopeItemToContextMenu) {
//				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
//				pitem.setText("&Reset Scope");
//				pitem.setSelection(true);
//				pitem.addSelectionListener(new SelectionAdapter() {
//					public void widgetSelected(SelectionEvent e) {
//						FileTable.this.mainWindow.resetScope();
//					}
//				});
//			}
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.S_SELECT_FILES_THAT_HAVE_CLONES_BETWEEN_THEM"));  //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					int[] selectedIndices = table.getSelectionIndices();
					FileTable.this.mainWindow.selectFilesThatHaveClonesBetweenFiles(selectedIndices, null); 
					/* Do not pass FileTable.this as a second parameter of mainWindow.selectFilesThatHaveClonesBetweenFile,
					 * in order to receive call-back message from mainWindow.
					 * By the call-back message, FileTable.this will update
					 * the state of file(s) selection.
					 */
				}
			});
		}
		{
			MenuItem pitemSelectFilesUnderDirectory = new MenuItem(pmenu, SWT.CASCADE);
			pitemSelectFilesUnderDirectory.setText(Messages.getString("gemx.FileTable.M_SELECT_FILES_UNDER")); //$NON-NLS-1$
			Menu pmenuSelectFilesUnderDirectory = new Menu(pitemSelectFilesUnderDirectory);
			pitemSelectFilesUnderDirectory.setMenu(pmenuSelectFilesUnderDirectory);
			
			{
				MenuItem pitem = new MenuItem(pmenuSelectFilesUnderDirectory, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.FileTable.M_THE_SAME_DIRECTORY")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new FilesUnderDirectory(1));
			}
			for (int i = 1; i < 6; ++i) {
				MenuItem pitem = new MenuItem(pmenuSelectFilesUnderDirectory, SWT.PUSH);
				StringBuffer buf = new StringBuffer();
				for (int j = 0; j < i; ++j) {
					buf.append("../"); //$NON-NLS-1$
				}
				buf.append(" (&" + i + ")"); //$NON-NLS-1$ //$NON-NLS-2$
				pitem.setText(buf.toString());
				pitem.setSelection(true);
				pitem.addSelectionListener(new FilesUnderDirectory(i));
			}
			MenuItem pitem = new MenuItem(pmenuSelectFilesUnderDirectory, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.M_NAMED_PARENT_DIRECTORY")); //$NON-NLS-1$
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					int index = FileTable.this.table.getSelectionIndex();
					if (index >= 0) {
						FileTable.this.mainWindow.doFileSelectionFromOneOfParentPaths(index);
					}
				}
			});
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.S_SELECT_CLONE_SETS_INCULDED_BY_THEM")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					int[] selectedIndex = table.getSelectionIndices();
					FileTable.this.mainWindow.selectCloneClassIDByFileID(selectedIndex);
					FileTable.this.mainWindow.showCloneSetTable();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.M_CLONE_SETS_COMMONLY_INCLUDED_BY_THEM")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					int[] selectedIndex = table.getSelectionIndices();
					FileTable.this.mainWindow.selectCommonCloneClassIDByFileID(selectedIndex);
					FileTable.this.mainWindow.showCloneSetTable();
				}
			});
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.M_ADD_FILE_METRICS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					FileTable.this.mainWindow.addFileMetrics();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.FileTable.M_FILTER_FILE_BY_METRIC"));  //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					FileTable.this.mainWindow.doFilteringFilesByMetrics();
				}
			});
		}
	}

	public FileTable(Composite parent, boolean bAddResetScopeItemToContextMenu, MainWindow mainWindow) {
		this.display = parent.getDisplay();
		this.mainWindow = mainWindow;
		shell = parent.getShell();
		
		sc = new Composite(parent, SWT.NONE);
		{
			GridLayout layout = new GridLayout(1, false);
			layout.marginHeight = 0;
			layout.marginWidth = 0;
			sc.setLayout(layout);
		}
		
		commonPathLabel = new Label(sc, SWT.LEFT);
		commonPathLabel.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL));
		commonPathLabel.setText("-"); //$NON-NLS-1$
		
		table = new Table(sc, SWT.VIRTUAL | SWT.MULTI | SWT.FULL_SELECTION | SWT.CHECK);
		tableWithCheckHelper = new TableWithCheckHelper(table) {
			@Override
			public void selectCheckedItems() {
				super.selectCheckedItems();
				
				int[] selectedIndex = table.getSelectionIndices();
				FileTable.this.mainWindow.setFileSelection(selectedIndex, FileTable.this);
			}
		};
		
		table.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));

		table.setHeaderVisible(true);
		String[] colCaps = { "File ID", "Path          ", "LEN", "CLN", "", "", "", "", "" }; //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$ //$NON-NLS-5$ //$NON-NLS-6$ //$NON-NLS-7$ //$NON-NLS-8$ //$NON-NLS-9$
		int[] colWids = { 40, 100, 50, 50, 5, 5, 5, 5, 5 };
		int[] colAligns = { SWT.RIGHT, SWT.LEFT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.RIGHT, SWT.LEFT };
		cols = new TableColumn[colCaps.length];
		for (int i = 0; i < colCaps.length; ++i) {
			TableColumn col = new TableColumn(table, colAligns[i]);
			cols[i] = col;
			col.setText(colCaps[i]);
			col.setWidth(colWids[i]);
		}
		table.addListener(SWT.PaintItem, new Listener() {
			public void handleEvent(Event event) {
				if (event.index >= 3) {
					if (fileIndex2Id != null && fileMetricModel != null) {
						TableItem item = (TableItem)event.item;
						int i = table.indexOf(item);
						int fileID = fileIndex2Id[i];
						int metricIndex = event.index - 2; // magic number
						double fm = fileMetricModel.getNthMetricDataOfFile(fileID, metricIndex);
						double rfm = fileMetricModel.getNthRelativeMetricDataOfFile(fileID, metricIndex);
						String text = fieldFormats[metricIndex].format(fm);
						
						GC gc = event.gc;
						Color foreground = gc.getForeground();
						int colWidth = cols[event.index].getWidth();
						drawCell(event.gc, event.x, event.y, colWidth, event.height, foreground, rfm, text);
					}
//				} else if (event.index == 1) {
//					if (fileNames != null) {
//						TableItem item = (TableItem)event.item;
//						int i = table.indexOf(item);
//						String text = fileNames[i];
//						
//						GC gc = event.gc;
//						Color background = gc.getBackground();
//						Color foreground = gc.getForeground();
//						int colWidth = cols[event.index].getWidth();
//						drawFileNameCell(event.gc, event.x, event.y, colWidth, event.height, background, foreground, text);
//					}
				}
			}
		});
		
		buildMenuToTable(bAddResetScopeItemToContextMenu);
		
		table.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				int[] selectedIndex = table.getSelectionIndices();
				FileTable.this.mainWindow.setFileSelection(selectedIndex, FileTable.this);
			}
		});
	}

	private static boolean isFileFiltered(Model data, int fileID) {
		if (data.containsFileRemark(fileID)) {
			ArrayList<String> remarks = data.getFileRemarkFromFileID(fileID);
			for (String remark : remarks) {
				if (remark.equals("masked")) { //$NON-NLS-1$
					return true;
				}
			}
		}
		return false;
	}
	
	public void updateModel(Model data) {
		Color filteredFileColor = gemx.scatterplothelper.PlottingColors.getMaskedFileBackground();
		fileMetricModel = null;
		fileIndex2Id = null;
		fileNames = null;
		
		searchingIndex = -1;
		searchingText = null;
		
		boolean tableVisible = table.isVisible();
		if (tableVisible) {
			table.setVisible(false);
			commonPathLabel.setVisible(false);
		}
		try {
			String commonPath = data.getCommonFilePath();
			commonPathLabel.setText(commonPath);
			
			table.removeAll();
			
			final int ccount = table.getColumnCount();
			for (int i = 4; i < ccount; ++i) {
				TableColumn tci = table.getColumn(i);
				tci.setText(""); //$NON-NLS-1$
			}
			
			int fileCount = data.getFileCount();
			fileIndex2Id = new int[fileCount];
			fileId2Index = new int[data.getMaxFileID() + 1];
			fileNames = new String[fileCount];
			for (int i = 0; i < fileId2Index.length; ++i) {
				fileId2Index[i] = -1; // invalid
			}
			for (int i = 0; i < fileCount; ++i) {
				SourceFile f = data.getFile(i);
				fileIndex2Id[i] = f.id;
				fileId2Index[f.id] = i;
				String pathDifference = f.path.substring(commonPath.length());
				fileNames[i] = pathDifference;
				TableItem item = new TableItem(table, SWT.NULL);
				String[] itemData = new String[ccount];
				itemData[0] = String.valueOf(f.id);
				itemData[1] = pathDifference;
//				{
//					PathAndFName pf = splitToPathAndFName(pathDifference);
//					String s;
//					if (pf.path.length() > 0) {
//						s = pf.fname + " - " + pf.path;
//					} else {
//						s = pf.fname;
//					}
//					itemData[1] = s;
//				}
				itemData[2] = String.valueOf(f.size);
				itemData[3] = String.valueOf(data.getCloneSetCountOfFile(i));
				for (int ci = 4; ci < ccount; ++ci) {
					itemData[ci] = ""; //$NON-NLS-1$
				}
				item.setText(itemData);
				
				if (isFileFiltered(data, f.id)) {
					item.setBackground(filteredFileColor);
				}
				
			}
			
			{
				TableColumn[] columns = table.getColumns();
				for (int cnt = 0; cnt < columns.length; cnt++) {
				  columns[cnt].pack();
				}
			}
		} finally {	
			if (tableVisible) {
				table.setVisible(true);
				commonPathLabel.setVisible(true);
			}
		}
	}

	public void setSelection(int[] selectedIndex) {
		table.setSelection(selectedIndex);
	}
	
	public void addCheckmarks(int[] indices) {
		final int itemCount = table.getItemCount();
		for (int i = 0; i < indices.length; ++i) {
			final int index = indices[i];
			if (0 <= index && index < itemCount) {
				TableItem item = table.getItem(index);
				item.setChecked(true);
			}
		}
	}
	
	public int[] getIndicesFromFileIDs(int[] fileIDs) {
		int[] indices = new int[fileIDs.length];
		for (int i = 0; i < fileIDs.length; ++i) {
			indices[i] = fileId2Index[fileIDs[i]];
		}
		return indices;
	}
	
	public void addFileMetricModel(FileMetricModel fmModel) {
		fileMetricModel = fmModel;
		
		boolean tableVisible = table.isVisible();
		if (tableVisible) {
			table.setVisible(false);
			commonPathLabel.setVisible(false);
		}
		try {
			for (int i = 0; i < fileMetricModel.getFieldCount(); ++i) {
				TableColumn tc = table.getColumn(i + 2);
				tc.setText(fileMetricModel.getMetricName(i)); //$NON-NLS-1$
			}
			
			final DecimalFormat intFormat = new DecimalFormat("#"); //$NON-NLS-1$
			final DecimalFormat doubleFormat = new DecimalFormat("#.000"); //$NON-NLS-1$
			this.fieldFormats = new DecimalFormat[fileMetricModel.getFieldCount()];
			for (int i = 0; i < fieldFormats.length; ++i) {
				if (fileMetricModel.isFlotingPoint(i)) {
					fieldFormats[i] = doubleFormat;
				} else {
					fieldFormats[i] = intFormat;
				}
			}
			
			if (fileIndex2Id != null) {
				final String nullString = new String();
				int count = fileIndex2Id.length;
				for (int i = 0; i < count; ++i) {
					TableItem item = table.getItem(i);
					for (int columnIndex = 3; columnIndex < 8; ++columnIndex) {
						//int metricIndex = columnIndex - 2;
						item.setText(columnIndex, nullString);
					}
				}
			}
			
			{
				TableColumn[] columns = table.getColumns();
				for (int cnt = 0; cnt < columns.length; cnt++) {
				  columns[cnt].pack();
				}
			}
		} finally {	
			if (tableVisible) {
				table.setVisible(true);
				commonPathLabel.setVisible(true);
			}
		}
	}
	
	private void searchingId(String text) {
		assert text.startsWith("#"); //$NON-NLS-1$
		if (fileId2Index == null) {
			return;
		}
		try {
			final String[] fields = utility.StringUtil.split(text.substring(1), ',');
			final TIntArrayList foundIndices = new TIntArrayList();
			for (int fi = 0; fi < fields.length; ++fi) {
				int id = Integer.parseInt(fields[fi]);
				if (0 <= id && id < fileId2Index.length) {
					foundIndices.add(fileId2Index[id]);
				}
			}
			if (foundIndices.size() > 0) {
				final int[] indices = foundIndices.toNativeArray();
				final int i = indices[0];
				searchingIndex = i;
				FileTable.this.mainWindow.setFileSelection(indices, FileTable.this);
				table.setSelection(i);
			}
		} catch (NumberFormatException e) {
			// error!
		}
	}
	
	public SearchboxListener getSearchboxListener() {
		return new SearchboxListener() {
			public void searchCanceled(SearchboxData data) {
				searchingIndex = -1;
				searchingText = null;
			}
			public void searchForward(SearchboxData data) {
				String text = data.text;
				if (text.startsWith("#")) { //$NON-NLS-1$
					searchingId(text);
					return;
				}
				
				searchingText = text;
				int index = table.getSelectionIndex();
				if (index >= 0) {
					if (index == searchingIndex) {
						++index;
					}
				} else {
					index = 0;
				}
				if (data.isIgnoreCase) {
					String textLower = text.toLowerCase();
					for (int i = index; i < table.getItemCount(); ++i) {
						TableItem item = table.getItem(i);
						String itemText = item.getText(1);
						if (itemText.toLowerCase().indexOf(textLower) >= 0) {
							searchingIndex = i;
							FileTable.this.mainWindow.setFileSelection(new int[] { searchingIndex }, FileTable.this);
							table.setSelection(i);
							return;
						}
					}
				} else {
					for (int i = index; i < table.getItemCount(); ++i) {
						TableItem item = table.getItem(i);
						String itemText = item.getText(1);
						if (itemText.indexOf(text) >= 0) {
							searchingIndex = i;
							FileTable.this.mainWindow.setFileSelection(new int[] { searchingIndex }, FileTable.this);
							table.setSelection(i);
							return;
						}
					}
				}
			}
			public void searchBackward(SearchboxData data) {
				String text = data.text;
				if (text.startsWith("#")) { //$NON-NLS-1$
					searchingId(text);
					return;
				}
				
				searchingText = text;
				int index = table.getSelectionIndex();
				if (index >= 0) {
					if (index == searchingIndex) {
						--index;
					}
				} else {
					index = table.getItemCount() - 1;
				}
				if (data.isIgnoreCase) {
					String textLower = text.toLowerCase();
					for (int i = index; i > 0; ++i) {
						TableItem item = table.getItem(i);
						String itemText = item.getText(1);
						if (itemText.toLowerCase().indexOf(textLower) >= 0) {
							searchingIndex = i;
							FileTable.this.mainWindow.setFileSelection(new int[] { searchingIndex }, FileTable.this);
							table.setSelection(i);
							return;
						}
					}
				} else {
					for (int i = index; i > 0; ++i) {
						TableItem item = table.getItem(i);
						String itemText = item.getText(1);
						if (itemText.indexOf(text) >= 0) {
							searchingIndex = i;
							FileTable.this.mainWindow.setFileSelection(new int[] { searchingIndex }, FileTable.this);
							table.setSelection(i);
							return;
						}
					}
				}
			}
		};
	}
}


