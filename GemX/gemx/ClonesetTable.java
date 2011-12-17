package gemx;

import java.text.DecimalFormat;
import java.util.*;

import gnu.trove.*;

import model.*;

import org.eclipse.swt.SWT;
import org.eclipse.swt.dnd.Clipboard;
import org.eclipse.swt.dnd.TextTransfer;
import org.eclipse.swt.dnd.Transfer;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.*;

import customwidgets.SearchboxData;
import customwidgets.SearchboxListener;

import res.Messages;
import resources.MetricColors;
import utility.StringUtil;

public class ClonesetTable implements CloneSelectionListener {
	private Table table;
	private TableWithCheckHelper tableWithCheckHelper;
	
	private MainWindow mainWindow;
	private Shell shell;
	private TableColumn[] cols;
	
	private int maxCloneSetCount = 500000;
	private int indexAndMore;
	private long andMoreCloneSetCount;
	private CloneSet[] cloneSets = null;
	
	private DecimalFormat[] fieldFormats;
	
	private ClonesetMetricModel cloneSetMetricModel = null;
	
	public Control getControl() {
		return table;
	}
	
	public void addListener(int eventType, Listener listener) {
		assert eventType == SWT.FocusIn;
		this.table.addListener(eventType, listener);
	}
	
	public TableWithCheckHelper getTableWithCheckHelper() {
		return tableWithCheckHelper;
	}
	
	private long[] getSelectedCloneSetIDs() {
		int[] selectedIndex = table.getSelectionIndices();
		if (selectedIndex.length == 0) {
			return new long[] {};
		}
		
		int size = selectedIndex.length;
		if (Arrays.binarySearch(selectedIndex, indexAndMore) >= 0) {
			--size;
		}
		assert size >= 0;
		long[] ids = new long[size];
		int p = 0;
		for (int i = 0; i < selectedIndex.length; ++i) {
			int index = selectedIndex[i];
			if (index != indexAndMore) {
				long id = cloneSets[index].id;
				ids[p] = id;
				++p;
			}
		}
		return ids;
	}
	
	private String selectedItemsToString() {
		int[] selectedIndex = table.getSelectionIndices();
		StringBuffer buffer = new StringBuffer();
		if (cloneSetMetricModel != null) {
			TableColumn[] columns = table.getColumns();
			for (int j = 0; j < columns.length - 1; ++j) {
				buffer.append(columns[j].getText());
				buffer.append('\t');
			}
			buffer.append(columns[columns.length - 1].getText());
			buffer.append(StringUtil.NewLineString);
			for (int i = 0; i < selectedIndex.length; ++i) {
				int index = selectedIndex[i];
				long cloneSetID = cloneSets[index].id;
				double[] cm = cloneSetMetricModel.getMetricDataOfCloneSet(cloneSetID);
				TableItem item = table.getItem(index);
				for (int j = 0; j < columns.length; ++j) {
					if (j >= 1) {
						int metricIndex = j - 1;
						String text = fieldFormats[metricIndex].format(cm[metricIndex]);
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
			for (int j = 0; j < 2 - 1; ++j) {
				buffer.append(columns[j].getText());
				buffer.append('\t');
			}
			buffer.append(columns[2 - 1].getText());
			buffer.append(StringUtil.NewLineString);
			for (int i = 0; i < selectedIndex.length; ++i) {
				int index = selectedIndex[i];
				buffer.append(String.valueOf(cloneSets[index].id));
				buffer.append('\t');
				buffer.append(String.valueOf(cloneSets[index].length));
				buffer.append(StringUtil.NewLineString);
			}
		}
		return buffer.toString();
	}
	
	private String selectedCloneSetIDsToString() {
		int[] selectedIndex = table.getSelectionIndices();
		StringBuffer buffer = new StringBuffer();
		for (int i = 0; i < selectedIndex.length; ++i) {
			int index = selectedIndex[i];
			TableItem item = table.getItem(index);
			buffer.append(item.getText(0));
			buffer.append(StringUtil.NewLineString);
		}
		return buffer.toString();
	}
	
	private void drawCell(GC gc, int x, int y, int cellWidth, int cellHeight, String text) {
		Point size = gc.textExtent(text);					
		int yoffset = Math.max(0, (cellHeight - size.y) / 2);
		int xoffset = Math.max(0, cellWidth - size.x - 4);
		gc.drawText(text, x + xoffset, y + yoffset, true);						
	}
	
	private void drawCell(GC gc, int x, int y, int cellWidth, int cellHeight, 
			double rvalue, String text) {
		Color foreground = gc.getForeground();
		Color background = gc.getBackground();
		int width = (int)((cellWidth - 3) * rvalue) + 2;
		Color color = MetricColors.getColor(rvalue);
		gc.setBackground(color);
		gc.setForeground(MetricColors.getFrameColor());
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
		gc.setBackground(background);
	}
	
	private static class CloneSetMetricComparator implements Comparator<CloneSet> {
		private final ClonesetMetricModel cloneSetMetricModel;
		private final int metricIndex;
		private final int dir;
		public CloneSetMetricComparator(ClonesetMetricModel cloneSetMetricModel, int metricIndex, int dir) {
			this.cloneSetMetricModel = cloneSetMetricModel;
			this.metricIndex = metricIndex;
			this.dir = dir;
		}
		public int compare(CloneSet cs0, CloneSet cs1) {
			long id0 = cs0.id;
			long id1 = cs1.id;
			double metric0 = cloneSetMetricModel.getNthMetricDataOfCloneSet(id0, metricIndex);
			double metric1 = cloneSetMetricModel.getNthMetricDataOfCloneSet(id1, metricIndex);
			if (dir == -1) {
				return metric0 <= metric1 ? -1 : 1;
			}
			else {
				return metric0 <= metric1 ? 1 : -1;
			}
		}
	}
	
	private void sortCloneIDsByMetric(CloneSet cloneSets[], int start, int end, 
			ClonesetMetricModel cloneSetMetricModel, int metricIndex,
			int order /* -1 up, 1 down */)
	{
		Arrays.sort(cloneSets, start, end, new CloneSetMetricComparator(cloneSetMetricModel, metricIndex, order));
	}
	
	private void sortCloneIDsByLength(CloneSet cloneSets[], int start, int end, 
			int order /* -1 up, 1 down */)
	{
		if (order == -1) {
			Arrays.sort(cloneSets, start, end, new Comparator<CloneSet>() {
				public int compare(CloneSet cs0, CloneSet cs1) {
					return cs0.length <= cs1.length ? -1 : 1;
				}
			});
		} else {
			Arrays.sort(cloneSets, start, end, new Comparator<CloneSet>() {
				public int compare(CloneSet cs0, CloneSet cs1) {
					return cs0.length <= cs1.length ? 1 : -1;
				}
			});
		}
	}
	
	private void sortCloneIDsByID(CloneSet cloneSets[], int start, int end, 
			int order /* -1 up, 1 down */)
	{
		if (order == -1) {
			Arrays.sort(cloneSets, start, end, new Comparator<CloneSet>() {
				public int compare(CloneSet cs0, CloneSet cs1) {
					return cs0.id <= cs1.id ? -1 : 1;
				}
			});
		} else {
			Arrays.sort(cloneSets, start, end, new Comparator<CloneSet>() {
				public int compare(CloneSet cs0, CloneSet cs1) {
					return cs0.id <= cs1.id ? 1 : -1;
				}
			});
		}
	}
	
	public void copyItemsToClipboard() {
		Clipboard clipboard = mainWindow.clipboard;
		clipboard.setContents(new Object[]{ selectedItemsToString() }, 
				new Transfer[]{ TextTransfer.getInstance() });
	}

	public void selectAllItems() {
		table.selectAll();
	}
	
	public void addCheckMarksToSelectedItems() {
		int[] selectedIndices = table.getSelectionIndices();
		Arrays.sort(selectedIndices);
		for (int i = 0; i < selectedIndices.length; ++i) {
			TableItem item = table.getItem(selectedIndices[i]);
			item.setChecked(true);
		}
	}
	
	public void removeCheckMarksFromSelectedItems() {
		int[] selectedIndices = table.getSelectionIndices();
		Arrays.sort(selectedIndices);
		for (int i = 0; i < selectedIndices.length; ++i) {
			TableItem item = table.getItem(selectedIndices[i]);
			item.setChecked(false);
		}
	}
	
	public void invertCheckMarks() {
		final int itemCount = table.getItemCount();
		for (int i = 0; i < itemCount; ++i) {
			TableItem item = table.getItem(i);
			boolean checked = item.getChecked();
			item.setChecked(! checked);
		}
	}
	
	public long[] getCheckedCloneSetIDs() {
		TLongArrayList checked = new TLongArrayList();
		final int itemCount = table.getItemCount();
		for (int i = 0; i < itemCount; ++i) {
			TableItem item = table.getItem(i);
			if (item.getChecked()) {
				long id = cloneSets[i].id;
				checked.add(id);
			}
		}
		return checked.toNativeArray();
	}
	
	private void buildMenuToTable(boolean bAddResetScopeItemToContextMenu) {
		Menu pmenu = new Menu(shell, SWT.POP_UP);
		table.setMenu(pmenu);

		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_COPY_ITEMS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					copyItemsToClipboard();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_COPY_CLONE_SET_IDS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					Clipboard clipboard = mainWindow.clipboard;
					clipboard.setContents(new Object[]{ selectedCloneSetIDsToString() }, 
							new Transfer[]{ TextTransfer.getInstance() });
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_PASTE_SELECTION")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					long[] cloneSetIDs = new long[ClonesetTable.this.cloneSets.length];
					for (int i = 0; i < cloneSetIDs.length; ++i) {
						cloneSetIDs[i] = ClonesetTable.this.cloneSets[i].id;
					}
					Arrays.sort(cloneSetIDs);
					Clipboard cb = mainWindow.clipboard;
					TextTransfer textTransfer = TextTransfer.getInstance();
					String str = (String)cb.getContents(textTransfer);
			        if (str == null) {
			        	return;
			        }
			        if (str.length() > 0 && str.charAt(0) == '#') {
			        	searchingId(str);
			        	return;
			        }
			        
					long[] selected = null;
					try {
						String[] lines = str.split(StringUtil.NewLineString);
						long[] selections = new long[lines.length];
						int count = 0;
						for (int i = 0; i < lines.length; ++i) {
							try {
								String line = lines[i];
								int tabpos = line.indexOf('\t');
								if (tabpos >= 0) {
									line = line.substring(0, tabpos);
								}
								long cloneSetID = Long.parseLong(line);
								if (Arrays.binarySearch(cloneSetIDs, cloneSetID) >= 0) {
									selections[count] = cloneSetID;
									++count;
								}
							} catch (NumberFormatException ex) {
								// nothing to do
							}
						}
						selected = utility.ArrayUtil.slice(selections, 0, count);
					} finally {
						if (selected != null) {
							ClonesetTable.this.mainWindow.setCloneSelection(selected, null);
						} else {
							ClonesetTable.this.mainWindow.setCloneSelection(new long[] { }, null);
						}
					}
				}
			});
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitemCheck = new MenuItem(pmenu, SWT.CASCADE);
			pitemCheck.setText(Messages.getString("gemx.ClonesetTable.M_CHECK_MARK")); //$NON-NLS-1$
			Menu pmenuCheck = new Menu(pitemCheck);
			pitemCheck.setMenu(pmenuCheck);
			
			tableWithCheckHelper.addCheckRelatedItemsToMenu(pmenuCheck);
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_FIT_SCOPE_TO_SELECTED_CLONE_SETS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					long[] selectedIDs = getSelectedCloneSetIDs();
					ClonesetTable.this.mainWindow.fitScopeToCloneSetIDs(selectedIDs);
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_POP_SCOPE")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ClonesetTable.this.mainWindow.popScope();
				}
			});
		}
//		if (bAddResetScopeItemToContextMenu) {
//			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
//			pitem.setText("&Reset Scope");
//			pitem.setSelection(true);
//			pitem.addSelectionListener(new SelectionAdapter() {
//				public void widgetSelected(SelectionEvent e) {
//					ClonesetTable.this.mainWindow.resetScope();
//				}
//			});
//		}
		
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_SELECT_FILES_INCLUDING_THEM")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					long[] selectedIDs = getSelectedCloneSetIDs();
					ClonesetTable.this.mainWindow.selectFilesByCloneClassID(selectedIDs);
					ClonesetTable.this.mainWindow.showFileTable();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_SELECT_FILES_INCLUDING_ALL_OF_THEM")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					long[] selectedIDs = getSelectedCloneSetIDs();
					ClonesetTable.this.mainWindow.selectCommonFilesByCloneClassID(selectedIDs);
					ClonesetTable.this.mainWindow.showFileTable();
				}
			});
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_ADD_CLONE_SET_METRICS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ClonesetTable.this.mainWindow.addCloneSetMetrics();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_FILTER_CLONE_SET_BY_METRIC"));  //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ClonesetTable.this.mainWindow.doFilteringCloneSetByMetrics();
				}
			});
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.ClonesetTable.M_SHOW_A_CODE_FRAGMENT")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					long[] selectedIDs = getSelectedCloneSetIDs();
					if (selectedIDs.length == 1) {
						if (! ClonesetTable.this.mainWindow.isTextPaneShown()) {
							ClonesetTable.this.mainWindow.showTextPane();
						}
						ClonesetTable.this.mainWindow.showACodeFragmentOfClone(selectedIDs[0], ClonesetTable.this);
					}
					else {
						MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_WARNING);
						mes.setText("Warning - GemX"); //$NON-NLS-1$
						mes.setMessage(Messages.getString("gemx.CloneSetTable.S_SELECT_ONE_CLONE_SET")); //$NON-NLS-1$
						mes.open();
						return;
					}
				}
			});
		}
		new MenuItem(pmenu, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_SHOW_NEXT_CODE")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					long[] selectedIDs = getSelectedCloneSetIDs();
					if (selectedIDs.length == 1) {
						if (! ClonesetTable.this.mainWindow.isTextPaneShown()) {
							ClonesetTable.this.mainWindow.showTextPane();
						}
						ClonesetTable.this.mainWindow.showNextPairedCodeOfClone(selectedIDs[0], 1, ClonesetTable.this);
					}
					else {
						MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_WARNING);
						mes.setText("Warning - GemX"); //$NON-NLS-1$
						mes.setMessage(Messages.getString("gemx.CloneSetTable.S_SELECT_ONE_CLONE_SET")); //$NON-NLS-1$
						mes.open();
						return;
					}
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.CloneSetTable.M_SHOW_PREV_CODE")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					long[] selectedIDs = getSelectedCloneSetIDs();
					if (selectedIDs.length == 1) {
						if (! ClonesetTable.this.mainWindow.isTextPaneShown()) {
							ClonesetTable.this.mainWindow.showTextPane();
						}
						ClonesetTable.this.mainWindow.showNextPairedCodeOfClone(selectedIDs[0], -1, ClonesetTable.this);
					}
					else {
						MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_WARNING);
						mes.setText("Warning - GemX"); //$NON-NLS-1$
						mes.setMessage(Messages.getString("gemx.CloneSetTable.S_SELECT_ONE_CLONE_SET")); //$NON-NLS-1$
						mes.open();
						return;
					}
				}
			});
		}
	}

	public ClonesetTable(Composite parent, boolean bAddResetScopeItemToContextMenu, MainWindow mainWindow) {
		this.mainWindow = mainWindow;
		shell = parent.getShell();
		
		table = new Table(parent, SWT.VIRTUAL | SWT.MULTI | SWT.FULL_SELECTION | SWT.CHECK);
		tableWithCheckHelper = new TableWithCheckHelper(table) {
			@Override
			public void selectCheckedItems() {
				super.selectCheckedItems();
				
				long[] selectedIDs = getSelectedCloneSetIDs();
				if (selectedIDs.length == 1 && ClonesetTable.this.mainWindow.isTextPaneShown()) {
					ClonesetTable.this.mainWindow.showACodeFragmentOfClone(selectedIDs[0], ClonesetTable.this);
				} else {
					ClonesetTable.this.mainWindow.setCloneSelection(selectedIDs, ClonesetTable.this);
				}
			}
		};
		
		table.setHeaderVisible(true);
		String[] colCaps = { "Clone-Set ID", "LEN", "", "", "", "", "", "", "", "" }; //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$ //$NON-NLS-5$ //$NON-NLS-6$ //$NON-NLS-7$ //$NON-NLS-8$ //$NON-NLS-9$ //$NON-NLS-10$
		int[] colWids = { 30, 50, 5, 5, 5, 5, 5, 5, 5, 5 };
		cols = new TableColumn[colCaps.length];
		for (int i = 0; i < colCaps.length; ++i) {
			TableColumn col = new TableColumn(table, SWT.RIGHT);
			cols[i] = col;
			col.setText(colCaps[i]);
			col.setWidth(colWids[i]);
		}
		
		table.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				long[] selectedIDs = getSelectedCloneSetIDs();
				if (selectedIDs.length == 1 && ClonesetTable.this.mainWindow.isTextPaneShown()) {
					if (ClonesetTable.this.mainWindow.getMain().settingClonesetTableClickToShowPair) {
						int direction = (e.stateMask & SWT.SHIFT) != 0 ? 1 : -1;
						ClonesetTable.this.mainWindow.showNextPairedCodeOfClone(selectedIDs[0], direction, ClonesetTable.this);
					} else {
						ClonesetTable.this.mainWindow.showACodeFragmentOfClone(selectedIDs[0], ClonesetTable.this);
					}
				} else {
					ClonesetTable.this.mainWindow.setCloneSelection(selectedIDs, ClonesetTable.this);
				}
			}
		});
		table.addListener(SWT.PaintItem, new Listener() {
			public void handleEvent(Event event) {
				TableItem item = (TableItem)event.item;
				int i = table.indexOf(item);
				int colWidth = cols[event.index].getWidth();
				if (event.index == 0) {
					// do nothing
				} else if (event.index == 1) {
					if (cloneSets != null) {
						if (i != indexAndMore) {
							if (cloneSetMetricModel != null) {
								long cloneSetID = cloneSets[i].id;
								int metricIndex = event.index - 1; // magic number
								double cm = cloneSetMetricModel.getNthMetricDataOfCloneSet(cloneSetID, metricIndex);
								double rcm = cloneSetMetricModel.getNthRelativeMetricDataOfCloneSet(cloneSetID, metricIndex);
								String text = fieldFormats[metricIndex].format(cm);
								
								drawCell(event.gc, event.x, event.y, colWidth, event.height, rcm, text);
							} else {
								String str = String.valueOf(cloneSets[i].length);
								drawCell(event.gc, event.x, event.y, colWidth, event.height, str);
							}
						}
					}
				} else {
					if (cloneSets != null) {
						if (i != indexAndMore) {
							if (cloneSetMetricModel != null) {
								long cloneSetID = cloneSets[i].id;
								int metricIndex = event.index - 1; // magic number
								double cm = cloneSetMetricModel.getNthMetricDataOfCloneSet(cloneSetID, metricIndex);
								double rcm = cloneSetMetricModel.getNthRelativeMetricDataOfCloneSet(cloneSetID, metricIndex);
								String text = fieldFormats[metricIndex].format(cm);
								
								drawCell(event.gc, event.x, event.y, colWidth, event.height, rcm, text);
							}
						}
					}
				}
			}
		});
		
		{
			Listener sortListener = new Listener() {
				public void handleEvent(Event e) {
					// determine new sort column and direction
					TableColumn sortColumn = table.getSortColumn();
					TableColumn currentColumn = (TableColumn) e.widget;
					int dir = table.getSortDirection();
					if (sortColumn == currentColumn) {
						dir = dir == SWT.UP ? SWT.DOWN : SWT.UP;
					} else {
						table.setSortColumn(currentColumn);
						dir = SWT.UP;
					}
					// sort the data based on column and direction
					int index = -1;
					for (int i = 0; i < cols.length; ++i) {
						if (currentColumn == cols[i]) {
							index = i;
						}
					}
					int endIndex = ClonesetTable.this.indexAndMore != -1 ? ClonesetTable.this.indexAndMore : cloneSets.length;
					if (cloneSets != null && index == 0) {
						sortCloneIDsByID(cloneSets, 0, endIndex, dir == SWT.UP ? -1 : 1);
					} else if (cloneSets != null && cloneSetMetricModel != null) {
						int metricIndex = index - 1; // magic number
						sortCloneIDsByMetric(cloneSets, 0, endIndex, cloneSetMetricModel, metricIndex, dir == SWT.UP ? -1 : 1);
					} else if (cloneSets != null && index == 1) {
						sortCloneIDsByLength(cloneSets, 0, endIndex, dir == SWT.UP ? -1 : 1);
					}
					
					// update data displayed in table
					table.setSortDirection(dir);
					table.clearAll();
					updateCloneSetIDCells();
				}
			};
			for (int i = 0; i < cols.length; ++i) {
				cols[i].addListener(SWT.Selection, sortListener);
			}
		}
		table.setSortColumn(cols[0]);
		table.setSortDirection(SWT.UP);
		
		buildMenuToTable(bAddResetScopeItemToContextMenu);
	}
	
	private void updateCloneSetIDCells() {
		for (int i = 0; i < this.cloneSets.length; ++i) {
			TableItem item = table.getItem(i);
			if (i == indexAndMore) {
				item.setText(0, "+" + String.valueOf(andMoreCloneSetCount) + " clone sets"); //$NON-NLS-1$ //$NON-NLS-2$
			} else {
				CloneSet cs = cloneSets[i];
				item.setText(0, String.valueOf(cs.id));
			}
		}
	}
	
	public boolean isCloneSetSizeTooLarge() {
		return indexAndMore != -1;
	}

	public void updateModel(Model data) {
		cloneSetMetricModel = null;
		
		boolean tableVisible = table.isVisible();
		if (tableVisible) {
			table.setVisible(false);
		}
		try {
			table.setSortColumn(cols[0]);
			table.setSortDirection(SWT.UP);
			
			final int ccount = table.getColumnCount();
			for (int i = 2; i < ccount; ++i) {
				TableColumn tci = table.getColumn(i);
				tci.setText(""); //$NON-NLS-1$
			}
			
			table.removeAll();
			CloneSet[] cloneSets = data.getCloneSets(maxCloneSetCount);
			this.andMoreCloneSetCount = data.getCloneSetCount() - cloneSets.length;
			
			int size = cloneSets.length;
			indexAndMore = -1;
			if (cloneSets.length >= 1 && cloneSets[cloneSets.length - 1].id == -1) {
				indexAndMore = cloneSets.length - 1;
				--size;
			}
			assert size >= 0;
			for (int i = 0; i < cloneSets.length; ++i) {
				new TableItem(table, SWT.NULL);
			}
			
			this.cloneSets = new CloneSet[size];
			for (int i = 0; i < this.cloneSets.length; ++i) {
				if (i == indexAndMore) {
					this.cloneSets[i] = new CloneSet(-1, -1);
				} else {
					this.cloneSets[i] = cloneSets[i];
				}
			}
			updateCloneSetIDCells();
			
			{
				TableColumn[] columns = table.getColumns();
				for (int cnt = 0; cnt < columns.length; cnt++) {
				  columns[cnt].pack();
				}
			}
		} finally {
			if (tableVisible) {
				table.setVisible(true);
			}
		}
	}
	
	public void setCloneSelection(long[] ids, CloneSelectionListener src) {
		if (src == this) {
			return;
		}
		
		long[] idsc = ids.clone();
		
		Arrays.sort(idsc);
		
		boolean firstOne = true;
		table.deselectAll();
		int itemCount = table.getItemCount();
		for (int i = 0; i < itemCount; ++i) {
			if (i != indexAndMore) {
				long id = cloneSets[i].id;
				if (Arrays.binarySearch(idsc, id) >= 0) {
					table.select(i);
					if (firstOne) {
						table.showItem(table.getItem(i));
					}
					firstOne = false;
				}
			}
		}
	}
	
	public void addCloneSetMetricModel(ClonesetMetricModel cloneSetMetricModel) {
		boolean tableVisible = table.isVisible();
		if (tableVisible) {
			table.setVisible(false);
		}
		try {
			this.cloneSetMetricModel = cloneSetMetricModel;
			
			for (int i = 0; i < cloneSetMetricModel.getFieldCount(); ++i) {
				TableColumn tc = table.getColumn(i + 1);
				tc.setText(cloneSetMetricModel.getMetricName(i)); //$NON-NLS-1$
			}
			
			DecimalFormat intFormat = new DecimalFormat("#"); //$NON-NLS-1$
			DecimalFormat doubleFormat = new DecimalFormat("#.000"); //$NON-NLS-1$
			fieldFormats = new DecimalFormat[cloneSetMetricModel.getFieldCount()];
			for (int i = 0; i < fieldFormats.length; ++i) {
				if (cloneSetMetricModel.isFlotingPoint(i)) {
					fieldFormats[i] = doubleFormat;
				} else {
					fieldFormats[i] = intFormat;
				}
			}
			
			if (cloneSets != null) {
				final String nullString = new String();
				int count = cloneSets.length;
				for (int i = 0; i < count; ++i) {
					TableItem item = table.getItem(i);
					for (int columnIndex = 1; columnIndex < 9; ++columnIndex) {
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
			}
		}
	}
	
	public void addCheckmarks(long[] cloneSetIDs) {
		long[] ids = cloneSetIDs.clone();
		Arrays.sort(ids);
		
		final int count = table.getItemCount();
		for (int i = 0; i < count; ++i) {
			if (i != indexAndMore) {
				long id = cloneSets[i].id;
				if (Arrays.binarySearch(ids, id) >= 0) {
					TableItem item = table.getItem(i);
					item.setChecked(true);
				}
			}
		}
	}
	
	private void searchingId(String text) {
		assert text.startsWith("#"); //$NON-NLS-1$
		if (cloneSets == null) {
			return;
		}
		try {
			final String[] fields = utility.StringUtil.split(text.substring(1), ',');
			final long[] ids = new long[fields.length];
			for (int fi = 0; fi < fields.length; ++fi) {
				long id = Long.parseLong(fields[fi]);
				ids[fi] = id;
			}
			Arrays.sort(ids);
			final utility.BitArray founds = new utility.BitArray(fields.length);
			final int itemCount = table.getItemCount();
			int firstFoundIndex = -1;
			for (int i = 0; i < itemCount; ++i) {
				if (i != indexAndMore) {
					for (int fi = 0; fi < fields.length; ++fi) {
						if (Arrays.binarySearch(ids, cloneSets[i].id) >= 0) {
							founds.setAt(fi, true);
							if (firstFoundIndex < 0) {
								firstFoundIndex = i;
							}
						}
					}
				}
			}
			TLongArrayList foundIDs = new TLongArrayList();
			{
				int fi = founds.find(true);
				while (fi >= 0) {
					foundIDs.add(ids[fi]);
					fi = founds.find(true, fi + 1);
				}
			}
			if (foundIDs.size() > 0) {
				ClonesetTable.this.mainWindow.setCloneSelection(foundIDs.toNativeArray(), ClonesetTable.this);
				assert firstFoundIndex >= 0;
				table.setSelection(firstFoundIndex);
			}
		} catch (NumberFormatException e) {
			// error!
		}
	}

	public SearchboxListener getSearchboxListener() {
		return new SearchboxListener() {
			public void searchBackward(SearchboxData data) {
				String text = data.text;
				if (text.startsWith("#")) { //$NON-NLS-1$
					searchingId(text);
					return;
				}
			}

			public void searchCanceled(SearchboxData data) {
			}

			public void searchForward(SearchboxData data) {
				String text = data.text;
				if (text.startsWith("#")) { //$NON-NLS-1$
					searchingId(text);
					return;
				}
			}
		};
	}
}

