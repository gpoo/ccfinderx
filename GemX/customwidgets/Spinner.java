package customwidgets;

import org.eclipse.swt.SWT;
import org.eclipse.swt.SWTError;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.TypedListener;

public class Spinner extends Composite {
	private org.eclipse.swt.widgets.Spinner spinner;
	
	public Spinner(Composite parent, int style) {
		super(parent, style);
		
		spinner = new org.eclipse.swt.widgets.Spinner(this, style);
		spinner.setFont(getFont());
	}

	@Override
	public void setFont(Font font) {
		super.setFont(font);
		spinner.setFont(font);
	}

	public void setSelection(int selection) {
		spinner.setSelection(selection);
	}

	public int getSelection() {
		return spinner.getSelection();
	}

	public void setMaximum(int maximum) {
		spinner.setMaximum(maximum);
		resize();
	}

	public int getMaximum() {
		return spinner.getMaximum();
	}

	public void setMinimum(int minimum) {
		spinner.setMinimum(minimum);
	}

	public int getMinimum() {
		return spinner.getMinimum();
	}

	@Override
	public Point computeSize(int widthHint, int heightHint, boolean changed) {
		Point size = spinner.computeSize(SWT.DEFAULT, SWT.DEFAULT, false);
		size.y = (int)(size.y * 1.5);
		if (widthHint != SWT.DEFAULT) {
			size.x = widthHint;
		}
		if (heightHint != SWT.DEFAULT) {
			size.y = heightHint;
		}
		return size;
	}
	
	private void resize() {
		Point size = computeSize(SWT.DEFAULT, SWT.DEFAULT, false);
		spinner.setBounds(0, 0, size.x, size.y);
	}

	public void addSelectionListener(SelectionListener listener) {
		if (listener == null)
			throw new SWTError(SWT.ERROR_NULL_ARGUMENT);
		spinner.addListener(SWT.Selection, new TypedListener(listener));
	}
	
	public void setDigits(int value) {
		spinner.setDigits(value);
		resize();
	}
}
