package utility;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Cursor;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

public class TemporaryMouseCursorChanger {
	private final Shell shell;
	private final Cursor prevCursor;
	private final Cursor curCursor;
	private boolean disposed;
	public TemporaryMouseCursorChanger(Shell shell) {
		this(shell, SWT.CURSOR_WAIT);
	}
	public TemporaryMouseCursorChanger(Shell shell, int systemCursor) {
		this.disposed = false;
		this.shell = shell;
		Cursor prev = null;
		boolean oldSWT = false;
		try {
			prev = shell.getCursor();
			oldSWT = true;
		} catch (NoSuchMethodError e) {
			// old version of SWT
		}
		this.prevCursor = prev;
		
		if (oldSWT) {
			this.curCursor = new Cursor(null, systemCursor);
		} else {
			Display display = shell.getDisplay();
			this.curCursor = new Cursor(display, systemCursor);
		}
		shell.setCursor(curCursor);
	}
	public void dispose() {
		if (! disposed) {
			if (this.prevCursor != null) {
				shell.setCursor(prevCursor);
			} else {
				// old version of SWT
				shell.setCursor(new Cursor(null, SWT.CURSOR_ARROW));
			}
			curCursor.dispose();
			disposed = true;
		}
	}
}

