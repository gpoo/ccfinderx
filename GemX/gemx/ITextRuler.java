package gemx;

public interface ITextRuler extends TextPaneScrollListener {
	public abstract boolean isVisible();
	public abstract void setVisible(boolean visible);
	public abstract void setTextPane(MultipleTextPane pane);
	public abstract void textScrolled();
	public abstract void update();
	public abstract void updateViewLocationDisplay();
	public abstract int getWidth();
	public void changeFocusedTextPane(int index);
}