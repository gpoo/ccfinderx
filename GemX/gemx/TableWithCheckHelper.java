package gemx;

import java.util.Arrays;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.MenuItem;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Table;

import res.Messages;

public class TableWithCheckHelper {
	private Table table;
	
	public TableWithCheckHelper(Table table) {
		this.table = table;
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
	
	public void clearCheckMarks() {
		final int itemCount = table.getItemCount();
		for (int i = 0; i < itemCount; ++i) {
			TableItem item = table.getItem(i);
			item.setChecked(false);
		}
	}
	
	public void selectCheckedItems() {
		gnu.trove.TIntArrayList checked = new gnu.trove.TIntArrayList();
		final int itemCount = table.getItemCount();
		for (int i = 0; i < itemCount; ++i) {
			TableItem item = table.getItem(i);
			if (item.getChecked()) {
				checked.add(i);
			}
		}
		table.setSelection(checked.toNativeArray());
	}
	
	public void addCheckRelatedItemsToMenu(Menu pmenuCheck) {
		{
			MenuItem pitem = new MenuItem(pmenuCheck, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.TableWithCheckHelper.M_ADD_CHECK_MARKS_TO_SELECTED_ITEMS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					TableWithCheckHelper.this.addCheckMarksToSelectedItems();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenuCheck, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.TableWithCheckHelper.M_REMOVE_CHECK_MARKS_FROM_SELECTED_ITEMS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					TableWithCheckHelper.this.removeCheckMarksFromSelectedItems();
				}
			});
		}
		new MenuItem(pmenuCheck, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenuCheck, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.TableWithCheckHelper.M_INVERT_CHECK_MARKS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					TableWithCheckHelper.this.invertCheckMarks();
				}
			});
		}
		{
			MenuItem pitem = new MenuItem(pmenuCheck, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.TableWithCheckHelper.M_CLEAR_CHECK_MARKS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					TableWithCheckHelper.this.clearCheckMarks();
				}
			});
		}
		new MenuItem(pmenuCheck, SWT.SEPARATOR);
		{
			MenuItem pitem = new MenuItem(pmenuCheck, SWT.PUSH);
			pitem.setText(Messages.getString("gemx.TableWithCheckHelper.M_SELECT_CHECKED_ITEMS")); //$NON-NLS-1$
			pitem.setSelection(true);
			pitem.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					TableWithCheckHelper.this.selectCheckedItems();
				}
			});
		}
	}
}
