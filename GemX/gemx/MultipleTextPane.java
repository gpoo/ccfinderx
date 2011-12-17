package gemx;

import java.util.*;

import gemx.CloneSelectionListener;
import gemx.FileSelectionListener;
import gemx.MainWindow;
import gemx.TextPane;
import gemx.TextPane.BeginEnd;
import model.*;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.*;

import customwidgets.*;

import utility.PrepToken;

public class MultipleTextPane implements FileSelectionListener, CloneSelectionListener {
	private MainWindow mainWindow;
	private ArrayList<TextPane> textPanes;
	private ITextRuler ruler;
	private Composite sc;
	private SashForm sash;
	private Model viewedModel;
	private boolean independentMode;
	private int focusedTextPaneIndex = -1;
	
	private ArrayList<Listener> addedListeners = new ArrayList<Listener>();
	
	public void clearData() {
		for (int i = 0; i < textPanes.size(); ++i) {
			this.textPanes.get(i).clearData();
		}
		resizePanes(0);
	}
	
	public void copyData(MultipleTextPane original) {
		int countOfPanesShowingFile = 0;
		for (int i = 0; i < textPanes.size(); ++i) {
			if (i >= original.textPanes.size()) {
				break; // for i
			}
			TextPane originalPane = original.textPanes.get(i);
			boolean originalPaneShowingFile = originalPane.getViewedFiles().length > 0;
			this.textPanes.get(i).copyData(original.textPanes.get(i));
			if (originalPaneShowingFile) {
				++countOfPanesShowingFile;
			}
		}
		resizePanes(countOfPanesShowingFile);
	}
	
	public void clearInitalTopPosition() {
		for (int i = 0; i < textPanes.size(); ++i) {
			TextPane pane = textPanes.get(i);
			pane.clearInitalTopPosition();
		}
	}
	

	public void setVisibleTokenCenterIndexOfPane(int paneIndex, int tokenIndex) {
		if (0 <= paneIndex && paneIndex < textPanes.size()) {
			textPanes.get(paneIndex).setVisibleTokenCenterIndex(tokenIndex);
		}
	}

	public int getInitialTokenPotitionOfPane(int paneIndex) {
		if (0 <= paneIndex && paneIndex < textPanes.size()) {
			return textPanes.get(paneIndex).getInitialTokenPotition();
		}
		return -1;
	}
	
	public void addListener(int eventType, Listener listener) {
		assert eventType == SWT.FocusIn;
		addedListeners.add(listener);
		for (int i = 0; i < textPanes.size(); ++i) {
			final TextPane textPane = textPanes.get(i);
			textPane.addListener(eventType, listener);
		}
	}
	
	public void copyTextToClipboard() {
		if (0 <= focusedTextPaneIndex && focusedTextPaneIndex < textPanes.size()) {
			final TextPane pane = textPanes.get(focusedTextPaneIndex);
			pane.copyTextToClipboard();
		}
	}
	
	public void selectEntireText() {
		if (0 <= focusedTextPaneIndex && focusedTextPaneIndex < textPanes.size()) {
			final TextPane pane = textPanes.get(focusedTextPaneIndex);
			pane.selectEntireText();
		}
	}
	
	public MultipleTextPane(Composite parent, MainWindow mainWindow, WidgetsFactory widgetsFactory) {
		int ntipleText = mainWindow.getMain().settingNtipleSourceTextPane;
		if (ntipleText < 0 || ntipleText > 100) {
			ntipleText = 2;
		}
		
		//Shell shell = parent.getShell();
		this.textPanes = new ArrayList<TextPane>();
		this.mainWindow = mainWindow;
		
		sc = new Composite(parent, SWT.NONE);
		{
			GridLayout layout = new GridLayout(1, false);
			layout.marginHeight = 0;
			layout.marginWidth = 0;
			sc.setLayout(layout);
		}
		
		SashForm sashRulerAndPanes = new SashForm(sc, SWT.HORIZONTAL);
		{
			GridData gridData = new GridData();
			gridData.horizontalAlignment = GridData.FILL;
			gridData.verticalAlignment = GridData.FILL;
			gridData.grabExcessHorizontalSpace = true;
			gridData.grabExcessVerticalSpace = true;
			sashRulerAndPanes.setLayoutData(gridData);
		}
		{
			ruler = widgetsFactory.newTextRuler(sashRulerAndPanes, mainWindow);
			ruler.setTextPane(this);
			
			sash = new SashForm(sashRulerAndPanes, SWT.HORIZONTAL);
			{
				GridData gridData = new GridData();
				gridData.horizontalAlignment = GridData.FILL;
				gridData.verticalAlignment = GridData.FILL;
				gridData.grabExcessHorizontalSpace = true;
				gridData.grabExcessVerticalSpace = true;
				sash.setLayoutData(gridData);
			}
			
			for (int i = 0; i < ntipleText; ++i) {
				TextPane pane = new TextPane(sash, mainWindow);
				pane.addScrollListener(ruler);
				for (int j = 0; j < addedListeners.size(); ++j) {
					final Listener listener = addedListeners.get(j);
					pane.addListener(SWT.FocusIn, listener);
				}
				pane.addListener(SWT.FocusIn, new Listener() {
					public void handleEvent(Event event) {
						for (int i = 0; i < textPanes.size(); ++i) {
							if (event.widget == textPanes.get(i).getControl()) {
								MultipleTextPane.this.focusedTextPaneIndexChanged(i);
							}
						}
					}
				});
				textPanes.add(pane);
			}
			
			{
				int[] weights = new int[ntipleText];
				weights[0] = 1;
				for (int i = 1; i < ntipleText; ++i) {
					weights[i] = 0;
				}
				sash.setWeights(weights);
			}
		}
		{
			int[] weights = new int[] { 3, 36 };
			sashRulerAndPanes.setWeights(weights);
		}
	}
	
	public int getCapacity() {
		return textPanes.size();
	}
	
	public void setCapacity(int newCapacity) {
		if (newCapacity > textPanes.size()) {
			int addition = newCapacity - textPanes.size();
			for (int i = 0; i < addition; ++i) {
				TextPane pane = new TextPane(sash, mainWindow);
				pane.addScrollListener(ruler);
				pane.setEncoding(textPanes.get(0).getEncoding());
				pane.updateModel(viewedModel);
				for (int j = 0; j < addedListeners.size(); ++j) {
					final Listener listener = addedListeners.get(j);
					pane.addListener(SWT.FocusIn, listener);
				}
				pane.addListener(SWT.FocusIn, new Listener() {
					public void handleEvent(Event event) {
						for (int i = 0; i < textPanes.size(); ++i) {
							if (event.widget == textPanes.get(i).getControl()) {
								MultipleTextPane.this.focusedTextPaneIndexChanged(i);
							}
						}
					}
				});
				textPanes.add(pane);
			}
			resizePanes(textPanes.size());
		}
	}
	
	private void focusedTextPaneIndexChanged(int index) {
		MultipleTextPane.this.focusedTextPaneIndex = index;
		//this.ruler.changeFocusedTextPane(index);
	}
	
	public boolean isIndependentMode() {
		return independentMode;
	}
	
	public void changeIndependentMode(boolean value) {
		independentMode = value;
	}
	
	public void setSelection(int[] fileIndices) {
		if (! independentMode) {
			ruler.setVisible(false);
			try {
				setSelection_i(fileIndices, false);
			}
			finally {
				ruler.setVisible(true);
			}
		}
	}
	
	private void resizePanes(int shownPanes) {
		final int panes = textPanes.size();
		final int ntiple = shownPanes > 0 ? shownPanes : 1;
		int[] weights = new int[panes];
		int i = 0;
		for (; i < ntiple; ++i) {
			weights[i] = 1;
		}
		for (; i < panes; ++i) {
			weights[i] = 0;
		}
		sash.setWeights(weights);
		this.ruler.update();
	}
	
	private void setSelection_i(int[] fileIndices, boolean requireAllFileViewingMode) {
		int ntiple = textPanes.size();
		if (fileIndices.length < ntiple) {
			ntiple = fileIndices.length;
		}
		final int panes = textPanes.size();
		assert panes >= ntiple;
		boolean rulerVisible = ruler.isVisible();
		if (rulerVisible) {
			ruler.setVisible(false);
		}
		try {
			{
				int i;
				for (i = 0; i < ntiple; ++i) {
					int[] f = new int[] { fileIndices[i] };
					textPanes.get(i).setSelection(f);
				}
				for (; i < panes; ++i) {
					textPanes.get(i).setSelection(null);
				}
			}
			resizePanes(ntiple);
		} finally {
			ruler.setVisible(rulerVisible);
		}
		if (requireAllFileViewingMode && fileIndices.length <= panes) {
			changeIndependentMode(true);
		}
	}

	public void setCloneSelection(long[] cloneSetIDs, CloneSelectionListener src) {
		if (src == this) {
			return;
		}
		
		boolean rulerVisible = ruler.isVisible();
		if (rulerVisible) {
			ruler.setVisible(false);
		}
		try {
			final int panes = textPanes.size();
			for (int i = 0; i < panes; ++i) {
				textPanes.get(i).setCloneSelection(cloneSetIDs, src);
			}
			this.ruler.update();
		} finally {
			ruler.setVisible(rulerVisible);
		}
	}

	public void updateModel(Model data, boolean isAllFileViewedModeEnabled) {
		this.viewedModel = data;
		
		changeIndependentMode(false);
		
		boolean rulerVisible = ruler.isVisible();
		if (rulerVisible) {
			ruler.setVisible(false);
		}
		try {
			final int panes = textPanes.size();
			for (int i = 0; i < panes; ++i) {
				textPanes.get(i).updateModel(data);
			}
			if (data.getFileCount() <= panes) {
				int files = data.getFileCount();
				int[] indices = new int[files];
				for (int i = 0; i < files; ++i) {
					indices[i] = i;
				}
				this.setSelection_i(indices, isAllFileViewedModeEnabled);
			}
			this.ruler.update();
		} finally {
			ruler.setVisible(rulerVisible);
		}
	}

	public int[] getViewedFiles() {
		final int panes = textPanes.size();
		int count = 0;
		for (int i = 0; i < panes; ++i) {
			count += textPanes.get(i).getViewedFiles().length;
		}
		int[] files = new int[count];
		int j = 0;
		for (int i = 0; i < panes; ++i) {
			int[] filesI = textPanes.get(i).getViewedFiles();
			for (int k = 0; k < filesI.length; ++k) {
				files[j] = filesI[k];
				++j;
			}
		}
		return files;
	}
	
	public void setClonePairSelection(ClonePair selectedPair) {
		changeIndependentMode(false);
		
		boolean rulerVisible = ruler.isVisible();
		if (rulerVisible) {
			ruler.setVisible(false);
		}
		try {
			final int panes = textPanes.size();
			textPanes.get(0).setCodeFragmentSelection(selectedPair.getLeftCodeFragment(), selectedPair.classID);
			textPanes.get(1).setCodeFragmentSelection(selectedPair.getRightCodeFragment(), selectedPair.classID);
			for (int i = 2; i < panes; ++i) {
				textPanes.get(i).setSelection(null);
			}
			int[] weights = new int[panes];
			weights[0] = 1;
			weights[1] = 1;
			for (int i = 2; i < panes; ++i) {
				weights[i] = 0;
			}
			sash.setWeights(weights);
			this.ruler.updateViewLocationDisplay();
		} finally {
			ruler.setVisible(rulerVisible);
		}
	}

	public void setCodeFragmentSelection(CodeFragment selectedCodeFragment, long cloneSetID) {
		final int panes = textPanes.size();
		if (independentMode) {
			for (int i = 0; i < panes; ++i) {
				textPanes.get(i).setCodeFragmentSelection(selectedCodeFragment, cloneSetID);
			}
			return;
		}
		
		boolean rulerVisible = ruler.isVisible();
		if (rulerVisible) {
			ruler.setVisible(false);
		}
		try {
			textPanes.get(0).setCodeFragmentSelection(selectedCodeFragment, cloneSetID);
			for (int i = 1; i < panes; ++i) {
				textPanes.get(i).setSelection(null);
			}
			int[] weights = new int[panes];
			weights[0] = 1;
			for (int i = 1; i < panes; ++i) {
				weights[i] = 0;
			}
			sash.setWeights(weights);
			this.ruler.update();
		} finally {
			ruler.setVisible(rulerVisible);
		}
	}
	
	public Control getControl() {
		return sc;
	}

	public boolean setEncoding(String encodingName) {
		boolean rulerVisible = ruler.isVisible();
		if (rulerVisible) {
			ruler.setVisible(false);
		}
		boolean result = true;
		try {
			final int panes = textPanes.size();
			for (int i = 0; i < panes; ++i) {
				result = textPanes.get(i).setEncoding(encodingName);
				if (! result) {
					break; // for
				}
			}
			this.ruler.update();
		} finally {
			ruler.setVisible(rulerVisible);
		}
		return result;
	}
	
	public PrepToken[] getTokens(int fileIndex) {
		final int panes = textPanes.size();
		for (int i = 0; i < panes; ++i) {
			PrepToken[] tokens = textPanes.get(i).getTokens(fileIndex);
			if (tokens != null) {
				return tokens;
			}
		}
		return null;
	}
	
	public ClonePair[] getClonePairs(int fileIndex) {
		final int panes = textPanes.size();
		for (int i = 0; i < panes; ++i) {
			ClonePair[] clonePairs = textPanes.get(i).getClonePairs(fileIndex);
			if (clonePairs != null) {
				return clonePairs;
			}
		}
		return null;
	}
	
//	public BeginEnd getVisibleTokenRange(int fileIndex) {
//		for (int i = 0; i < textPanes.length; ++i) {
//			BeginEnd be = textPanes[i].getVisibleTokenRange(fileIndex);
//			if (be != null) {
//				return be;
//			}
//		}
//		return null;
//	}
	
	public BeginEnd getVisibleTokenRangeOfPane(int paneIndex) {
		final int panes = textPanes.size();
		assert 0 <= paneIndex && paneIndex < panes;
		return textPanes.get(paneIndex).getVisibleTokenRange();
	}
	
	public SearchboxListener getSearchboxListener() {
		if (focusedTextPaneIndex == -1 && textPanes.size() > 0) {
			final TextPane pane = textPanes.get(0);
			return pane.getSearchboxListener();
		}
		if (0 <= focusedTextPaneIndex && focusedTextPaneIndex < textPanes.size()) {
			final TextPane pane = textPanes.get(focusedTextPaneIndex);
			return pane.getSearchboxListener();
		}
		
		return /* dummy */ new SearchboxListener() {
			public void searchBackward(SearchboxData data) {
			}
			public void searchCanceled(SearchboxData data) {
			}
			public void searchForward(SearchboxData data) {
			}
		};
	}
}

