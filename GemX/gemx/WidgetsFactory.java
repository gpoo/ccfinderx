package gemx;

import gemx.dialogs.AboutDialog;

import org.eclipse.swt.widgets.*;

public class WidgetsFactory {
	public FileTable newFileTable(Composite composite, boolean bAddResetScopeItemToContextMenu, MainWindow mainWindow) {
		return new FileTable(composite, bAddResetScopeItemToContextMenu, mainWindow);
	}
	public ClonesetTable newCloneSetTable(Composite composite, boolean bAddResetScopeItemToContextMenu, 
			MainWindow mainWindow) {
		return new ClonesetTable(composite, bAddResetScopeItemToContextMenu, mainWindow);
	}
	public ScatterPlotPane newScatterPlotPane(Composite composite, int shorter, boolean bAddResetScopeItemToContextMenu,
			MainWindow mainWindow) {
		return new ScatterPlotPane(composite, shorter, bAddResetScopeItemToContextMenu, mainWindow);
	}
	public MultipleTextPane newTextPane(Composite composite, MainWindow mainWindow) {
		return new MultipleTextPane(composite, mainWindow, this);
	}
	public ITextRuler newTextRuler(Composite composite, MainWindow mainWindow) {
		return new TextRuler(composite, mainWindow);
	}
	public AboutDialog newAboutDialog(Shell shell) {
		return new AboutDialog(shell);
	}
}
