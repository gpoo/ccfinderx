package customwidgets;

public interface SearchboxListener {
	void searchCanceled(SearchboxData data);
	void searchForward(SearchboxData data);
	void searchBackward(SearchboxData data);
}
