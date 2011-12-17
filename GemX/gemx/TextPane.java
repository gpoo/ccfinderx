 package gemx;

import java.io.*;
import java.util.*;

import gnu.trove.*;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.dnd.Clipboard;
import org.eclipse.swt.dnd.TextTransfer;
import org.eclipse.swt.dnd.Transfer;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import customwidgets.SearchboxData;
import customwidgets.SearchboxListener;

import res.Messages;
import resources.TextColors;

import model.*;
import utility.*;

public class TextPane implements FileSelectionListener, CloneSelectionListener {
	private Shell shell;
	private MainWindow mainWindow;
	private Composite sc;
	private Label fileNameLabel;

	private Composite lineNumberAndText;
	private StyledText text;
	private Canvas lineNumber;

	private final ArrayList<TextPaneScrollListener> listeners = new ArrayList<TextPaneScrollListener>();
	
	private Model viewedModel;

	private int initialTopPosition;
	private int initialTokenPosition;
	
	private int fileIndex;
	private SourceFile file;
	private String textString;
	private String textStringLower;
	private PrepToken[] tokens;
	private int[] tokenEndIndices;
	private ClonePair[] clonePairs;
	private int[] selectedClonePairs;
	private int[] lineStatus;
	
	private static final int WHOLE_LINE_COVERED = 1 << 0;
	private static final int CLONE_BEGIN_LINE = 1 << 1;
	private static final int CLONE_END_LINE = 1 << 2;
	private static final int WHOLE_LINE_SELECTED = 1 << 3;
	private static final int SELECTION_BEGIN_LINE = 1 << 4;
	private static final int SELECTION_END_LINE = 1 << 5;
	private static final int BETWEEN_FILE_SHIFT = 6;

	private int bottomDisplayHeight = 5; // teketo-
	
	private String encodingName = "";
	
	private long[] allCloneSetIDsSelectedByRightClick;
	private long[] innerfileCloneSetIDsSelectedByRightClick;
	private long[] bothCloneSetIDsSelectedByRightClick;
	private long[] crossfileCloneSetIDsSelectedByRightClick;
	
	private int searchingIndex = -1;
	private String searchingText = null;
	
	private class ScrollRequest {
		private int cloneIndex;
		public ScrollRequest(int cloneIndex) {
			this.cloneIndex = cloneIndex;
		}
		public void run() {
			text.setTopIndex(this.cloneIndex);
		}
	}
	private ScrollRequest textScrollRequest = null;
	
	public void clearData() {
		initialTopPosition = -1; // -1 means "not initialized"
		initialTokenPosition = 0;
		
		fileIndex = -1;
		file = null;
		textString = textStringLower = ""; //$NON-NLS-1$
		tokens = null;
		tokenEndIndices = null;
		clonePairs = null;
		selectedClonePairs = null;
		lineStatus = null;

		allCloneSetIDsSelectedByRightClick = null;
		innerfileCloneSetIDsSelectedByRightClick = null;
		bothCloneSetIDsSelectedByRightClick = null;
		crossfileCloneSetIDsSelectedByRightClick = null;
	}
	
	public void copyData(TextPane original) {
		this.encodingName = original.encodingName;
		
		this.updateModel(original.viewedModel);
		boolean originalShowingFile = original.getViewedFiles().length > 0;
		if (originalShowingFile) {
			this.setFile(original.fileIndex);
			this.selectedClonePairs = original.selectedClonePairs.clone();
		} else {
			this.fileIndex = -1;
			this.file = null;
			this.textString = this.textStringLower = ""; //$NON-NLS-1$
			this.tokens = null;
			this.tokenEndIndices = null;
			this.clonePairs = null;
			this.selectedClonePairs = null;
			this.lineStatus = null;
		}
		
		this.allCloneSetIDsSelectedByRightClick = null;
		this.innerfileCloneSetIDsSelectedByRightClick = null;
		this.bothCloneSetIDsSelectedByRightClick = null;
		this.crossfileCloneSetIDsSelectedByRightClick = null;
		
		this.initialTopPosition = original.initialTopPosition;
		this.initialTokenPosition = original.initialTokenPosition;
		
		if (originalShowingFile) {
			setTextHighlightsAndLineStatus(true);
			this.text.setTopIndex(original.text.getTopIndex());
		}
		
		this.searchingIndex = original.searchingIndex;
		this.searchingText = original.searchingText;
	}
	
	public void clearInitalTopPosition() {
		this.initialTopPosition = -1; // -1 means "not initialized"
	}
	
	public int getInitialTokenPotition() {
		if (initialTopPosition == -1) {
			return -1; // not defined
		} else {
			assert initialTokenPosition != -1;
			return initialTokenPosition;
		}
	}
	
	public void addListener(int eventType, Listener listener) {
		assert eventType == SWT.FocusIn;
		this.text.addListener(eventType, listener);
	}
	
	public int getWidth() {
		if (sc == null) {
			return 1;
		}
		return sc.getClientArea().width;
	}
	
	public String getEncoding() {
		return this.encodingName; // may return null
	}
	
	public boolean setEncoding(String encodingName) {
		if (encodingName == null) {
			this.encodingName = "";
			return true;
		}
		
		this.encodingName = encodingName;
		if (! Decoder.isValidEncoding(encodingName)) {
			return false;
		}
		return true;
	}
	
	static class SelectionAdapterWithCloneSetIDs extends SelectionAdapter {
		private TextPane pane;
		private long[] selectedIDs;
		public SelectionAdapterWithCloneSetIDs(TextPane pane, long[] selectedIDs) {
			this.pane = pane;
			this.selectedIDs = selectedIDs;
		}
		@Override
		public void widgetSelected(SelectionEvent e) {
			pane.setCloneSelection(selectedIDs, null, false);
			pane.mainWindow.setCloneSelection(selectedIDs, pane);
		}
	}
	
	private static void calc_intersection(TLongHashSet result, TLongHashSet a, TLongHashSet b) {
		final long[] aary = a.toArray();
		Arrays.sort(aary);
		final long[] bary = b.toArray();
		Arrays.sort(bary);
		int ai = 0;
		int bi = 0;
		while (ai < aary.length && bi < bary.length) {
			final long av = aary[ai];
			final long bv = bary[bi];
			if (av < bv) {
				++ai;
			} else if (av == bv) {
				result.add(av);
				++ai;
				++bi;
			} else {
				assert av > bv;
				++bi;
			}
		}
	}
	
	public void copyTextToClipboard() {
		String t = text.getSelectionText();
		if (t != null && t.length() > 0) {
			Clipboard clipboard = TextPane.this.mainWindow.clipboard;
			clipboard.setContents(new Object[]{ text.getSelectionText() }, 
					new Transfer[]{ TextTransfer.getInstance() });
		}
	}
	
	public void selectEntireText() {
		text.selectAll();
	}
	
	public TextPane(Composite parent, MainWindow mainWindow) {
		this.shell = parent.getShell();
		this.mainWindow = mainWindow;
		
		this.initialTopPosition = -1; // -1 means "not initialized"
		
		sc = new Composite(parent, SWT.NONE);
		{
			GridLayout layout = new GridLayout(1, false);
			layout.marginHeight = 0;
			layout.marginWidth = 0;
			sc.setLayout(layout);
		}

		fileNameLabel = new Label(sc, SWT.LEFT);
		fileNameLabel.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL));
		fileNameLabel.setText("-"); //$NON-NLS-1$
		fileNameLabel.setToolTipText(""); //$NON-NLS-1$

		lineNumberAndText = new Composite(sc, SWT.NONE);
		lineNumberAndText.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		{
			GridLayout layout = new GridLayout(2, false);
			layout.marginHeight = 0;
			layout.marginWidth = 0;
			lineNumberAndText.setLayout(layout);
		}
		lineNumberAndText.setBackground(TextColors.getWhite());
		
		lineNumber = new Canvas(lineNumberAndText, SWT.NONE);
		{
			int width = calcWidthOfNumberString(999999);
			GridData gridData = new GridData(SWT.NONE, SWT.FILL, false, true);
			gridData.widthHint = width;
			gridData.heightHint = 200;
			lineNumber.setLayoutData(gridData);
		}
		lineNumber.addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				redrawLineNumber(e.gc, false);
				for (TextPaneScrollListener listener : TextPane.this.listeners) {
					listener.textScrolled();
				}
			}
		});
		
		text = new StyledText(lineNumberAndText, SWT.H_SCROLL | SWT.V_SCROLL);
		text.addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				final ScrollRequest sr = TextPane.this.textScrollRequest;
				if (sr != null) {
					sr.run();
					TextPane.this.textScrollRequest = null;
				}
			}
		});
		fileNameLabel.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				text.forceFocus();
			}
		});
		fileNameLabel.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				boolean rightClick = e.button == 3;
				if (rightClick) {
					Menu pmenu = new Menu(TextPane.this.shell, SWT.POP_UP);
					fileNameLabel.setMenu(pmenu);
					{
						MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
						pitem.setText(Messages.getString("gemx.SourceTextPane.M_COPY_FILE_PATH")); //$NON-NLS-1$
						pitem.setSelection(true);
						pitem.addSelectionListener(new SelectionAdapter() {
							public void widgetSelected(SelectionEvent e) {
								Clipboard clipboard = TextPane.this.mainWindow.clipboard;
								clipboard.setContents(new Object[]{ TextPane.this.file.path }, 
										new Transfer[]{ TextTransfer.getInstance() });
							}
						});
					}
				} else {
					text.forceFocus();
				}
			}
		});
		
		lineNumber.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				text.forceFocus();
			}
		});
		text.setForeground(TextColors.getNeglectedText());
		{
			GridData gridData = new GridData();
			gridData.horizontalAlignment = GridData.FILL;
			gridData.grabExcessHorizontalSpace = true;
			gridData.verticalAlignment = GridData.FILL;
			gridData.grabExcessVerticalSpace = true;
			text.setLayoutData(gridData);
		}
		text.setEditable(false);
		text.setText(""); //$NON-NLS-1$
		text.addListener(SWT.FocusIn, new Listener() {
			public void handleEvent(Event event) {
				final Display display = TextPane.this.shell.getDisplay();
				fileNameLabel.setBackground(display.getSystemColor(SWT.COLOR_TITLE_BACKGROUND));
				fileNameLabel.setForeground(display.getSystemColor(SWT.COLOR_TITLE_FOREGROUND));
			}
		});
		text.addListener(SWT.FocusOut, new Listener() {
			public void handleEvent(Event event) {
				final Display display = TextPane.this.shell.getDisplay();
				fileNameLabel.setBackground(display.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND));
				fileNameLabel.setForeground(display.getSystemColor(SWT.COLOR_WIDGET_FOREGROUND));
			}
		});
//debug
//		text.addModifyListener(new ModifyListener() {
//			public void modifyText(ModifyEvent e) {
//				GC gc = new GC(lineNumber);
//				try {
//					//TextPane.this.redrawLineNumber(gc, true);
//					for (TextPaneScrollListener listener : TextPane.this.listeners) {
//						listener.textScrolled();
//					}
//				} finally {
//					gc.dispose();
//				}
//			}
//		});
		text.addTraverseListener(new TraverseListener() {
			public void keyTraversed(TraverseEvent e) {
				GC gc = new GC(lineNumber);
				try {
					TextPane.this.redrawLineNumber(gc, false);
				} finally {
					gc.dispose();
				}
				for (TextPaneScrollListener listener : TextPane.this.listeners) {
					listener.textScrolled();
				}
			}
		});
		text.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				final ClonePair[] clonePairs = TextPane.this.clonePairs;
				final PrepToken[] tokens = TextPane.this.tokens;
				final int fileIndex = TextPane.this.fileIndex;
				boolean rightClick = e.button == 3;
				if (rightClick) {
					if (clonePairs != null && tokens != null) {
						int pos;
						Point clickedPoint = new Point(e.x, e.y);
						try {
							pos = text.getOffsetAtLocation(clickedPoint);
						} catch (IllegalArgumentException ex) {
							// this means that the point is not on any character.
							clickedPoint.x = 0;
							try {
								pos = text.getOffsetAtLocation(clickedPoint);
							} catch (IllegalArgumentException ex2) {
								TextPane.this.mainWindow.setCloneSelection(new long[0], TextPane.this);
								return;
							}
						}
						int targetTokenIndex = -1;
						{
							assert tokens.length > 0;
							if (tokens[0].beginIndex <= pos && pos < tokens[0].endIndex) {
								targetTokenIndex = 0;
							} else {
								int lastTokenEndIndex = tokens[0].endIndex;
								for (int ti = 0; ti < tokens.length; ++ti) {
									if (lastTokenEndIndex <= pos && pos < tokens[ti].endIndex) {
										targetTokenIndex = ti;
										break; // for ti
									}
									lastTokenEndIndex = tokens[ti].endIndex;
								}
							}
						}
						if (targetTokenIndex != -1) {
							TLongHashSet idsTarget = new TLongHashSet();
							for (int i = 0; i < clonePairs.length; ++i) {
								ClonePair p = clonePairs[i];
								if (p.leftFile == fileIndex) {
									if (p.leftBegin <= targetTokenIndex && targetTokenIndex < p.leftEnd) {
										idsTarget.add(p.classID);
									}
								} else if (p.rightFile == fileIndex) {
									if (p.rightBegin <= targetTokenIndex && targetTokenIndex < p.rightEnd) {
										idsTarget.add(p.classID);
									}
								}
							}
							TLongHashSet idsInner = new TLongHashSet();
							TLongHashSet idsCross = new TLongHashSet();
							for (int i = 0; i < clonePairs.length; ++i) {
								ClonePair p = clonePairs[i];
								if (idsTarget.contains(p.classID)) {
									if (p.leftFile == p.rightFile) {
										idsInner.add(p.classID);
									} else {
										idsCross.add(p.classID);
									}
								}
							}
							
							TLongHashSet idsBoth = new TLongHashSet();
							calc_intersection(idsBoth, idsInner, idsCross);
							bothCloneSetIDsSelectedByRightClick = idsBoth.toArray();
							Arrays.sort(bothCloneSetIDsSelectedByRightClick);
							
							idsInner.removeAll(bothCloneSetIDsSelectedByRightClick);
							innerfileCloneSetIDsSelectedByRightClick = idsInner.toArray();
							Arrays.sort(innerfileCloneSetIDsSelectedByRightClick);
							
							idsCross.removeAll(bothCloneSetIDsSelectedByRightClick);
							crossfileCloneSetIDsSelectedByRightClick = idsCross.toArray();
							Arrays.sort(crossfileCloneSetIDsSelectedByRightClick);
							
							allCloneSetIDsSelectedByRightClick = new long[innerfileCloneSetIDsSelectedByRightClick.length + 
							                                              bothCloneSetIDsSelectedByRightClick.length +
							                                              crossfileCloneSetIDsSelectedByRightClick.length];
							int ii = 0;
							for (int i = 0; i < innerfileCloneSetIDsSelectedByRightClick.length; ++i) {
								allCloneSetIDsSelectedByRightClick[ii++] = innerfileCloneSetIDsSelectedByRightClick[i];
							}
							for (int i = 0; i < bothCloneSetIDsSelectedByRightClick.length; ++i) {
								allCloneSetIDsSelectedByRightClick[ii++] = bothCloneSetIDsSelectedByRightClick[i];
							}
							for (int i = 0; i < crossfileCloneSetIDsSelectedByRightClick.length; ++i) {
								allCloneSetIDsSelectedByRightClick[ii++] = crossfileCloneSetIDsSelectedByRightClick[i];
							}
							assert ii == allCloneSetIDsSelectedByRightClick.length;
							Arrays.sort(allCloneSetIDsSelectedByRightClick);
						} else {
							innerfileCloneSetIDsSelectedByRightClick = new long[] { };
							bothCloneSetIDsSelectedByRightClick = new long[] { };
							crossfileCloneSetIDsSelectedByRightClick = new long[] { };
							allCloneSetIDsSelectedByRightClick = new long[] { };
						}
						
						{
							Menu pmenu = new Menu(TextPane.this.shell, SWT.POP_UP);
							text.setMenu(pmenu);
	
							{
								MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
								pitem.setText(Messages.getString("gemx.SourceTextPane.M_COPY_TEXT")); //$NON-NLS-1$
								pitem.setSelection(true);
								pitem.addSelectionListener(new SelectionAdapter() {
									public void widgetSelected(SelectionEvent e) {
										TextPane.this.copyTextToClipboard();
									}
								});
							}
							{
								MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
								pitem.setText(Messages.getString("gemx.SourceTextPane.M_COPY_FILE_PATH")); //$NON-NLS-1$
								pitem.setSelection(true);
								pitem.addSelectionListener(new SelectionAdapter() {
									public void widgetSelected(SelectionEvent e) {
										Clipboard clipboard = TextPane.this.mainWindow.clipboard;
										clipboard.setContents(new Object[]{ TextPane.this.file.path }, 
												new Transfer[]{ TextTransfer.getInstance() });
									}
								});
							}
							new MenuItem(pmenu, SWT.SEPARATOR);
							{
								MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
								pitem.setText(Messages.getString("gemx.SourceTextPane.M_SELECT_ALL_CLONE_SETS_AROUND")); //$NON-NLS-1$
								pitem.setSelection(true);
								pitem.addSelectionListener(new SelectionAdapter() {
									public void widgetSelected(SelectionEvent e) {
										TextPane.this.setCloneSelection(allCloneSetIDsSelectedByRightClick, null, false);
										TextPane.this.mainWindow.setCloneSelection(allCloneSetIDsSelectedByRightClick, TextPane.this);
									}
								});
							}
							{
								MenuItem pitemSelectACloneSet = new MenuItem(pmenu, SWT.CASCADE);
								pitemSelectACloneSet.setText(Messages.getString("gemx.SourceTextPane.M_SELECT_ONE_CLONE_SET")); //$NON-NLS-1$
								Menu pmenuSelectACloneSet = new Menu(pitemSelectACloneSet);
								pitemSelectACloneSet.setMenu(pmenuSelectACloneSet);
								if (allCloneSetIDsSelectedByRightClick.length > 0) {
									for (int i = 0; i < innerfileCloneSetIDsSelectedByRightClick.length; ++i) {
										MenuItem pitem = new MenuItem(pmenuSelectACloneSet, SWT.PUSH);
										pitem.setText("ID " + innerfileCloneSetIDsSelectedByRightClick[i]); //$NON-NLS-1$
										pitem.setSelection(true);
										pitem.addSelectionListener(new SelectionAdapterWithCloneSetIDs(TextPane.this, new long[] { innerfileCloneSetIDsSelectedByRightClick[i] }));
									}
									new MenuItem(pmenuSelectACloneSet, SWT.SEPARATOR);
									for (int i = 0; i < bothCloneSetIDsSelectedByRightClick.length; ++i) {
										MenuItem pitem = new MenuItem(pmenuSelectACloneSet, SWT.PUSH);
										pitem.setText("ID " + bothCloneSetIDsSelectedByRightClick[i]); //$NON-NLS-1$
										pitem.setSelection(true);
										pitem.addSelectionListener(new SelectionAdapterWithCloneSetIDs(TextPane.this, new long[] { bothCloneSetIDsSelectedByRightClick[i] }));
									}
									new MenuItem(pmenuSelectACloneSet, SWT.SEPARATOR);
									for (int i = 0; i < crossfileCloneSetIDsSelectedByRightClick.length; ++i) {
										MenuItem pitem = new MenuItem(pmenuSelectACloneSet, SWT.PUSH);
										pitem.setText("ID " + crossfileCloneSetIDsSelectedByRightClick[i]); //$NON-NLS-1$
										pitem.setSelection(true);
										pitem.addSelectionListener(new SelectionAdapterWithCloneSetIDs(TextPane.this, new long[] { crossfileCloneSetIDsSelectedByRightClick[i] }));
									}
								}
							}
							new MenuItem(pmenu, SWT.SEPARATOR);
							{
								MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
								pitem.setText(Messages.getString("gemx.SourceTextPane.M_COPY_TO_SCRAPBOOK")); //$NON-NLS-1$
								pitem.setSelection(true);
								pitem.addSelectionListener(new SelectionAdapter() {
									public void widgetSelected(SelectionEvent e) {
										TextPane.this.mainWindow.copyTextToScrapbook();
									}
								});
							}
						}
					}
				}
			}
		});
		ScrollBar bar = text.getVerticalBar();
		bar.addSelectionListener(new SelectionListener() {
			public void widgetSelected(SelectionEvent e) {
				GC gc = new GC(lineNumber);
				try {
					TextPane.this.redrawLineNumber(gc, false);
				} finally {
					gc.dispose();
				}
				for (TextPaneScrollListener listener : TextPane.this.listeners) {
					listener.textScrolled();
				}
			}
			public void widgetDefaultSelected(SelectionEvent e) {
				GC gc = new GC(lineNumber);
				try {
					TextPane.this.redrawLineNumber(gc, false);
				} finally {
					gc.dispose();
				}
				for (TextPaneScrollListener listener : TextPane.this.listeners) {
					listener.textScrolled();
				}
			}
		});
		{
			Menu pmenu = new Menu(shell, SWT.POP_UP);
			text.setMenu(pmenu);

			{
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.SourcePane.M_POP_SCOPE")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						TextPane.this.mainWindow.popScope();
					}
				});
			}
		}
	}

	public void addScrollListener(TextPaneScrollListener listener) {
		this.listeners.add(listener);
	}
	
	private static int[] downTrianglePath(int x, int y, int width, int height) {
		return new int[] { 
				x, y, 
				x + width / 2,  y + height / 3,
				x + width, y
		};
	}
	private static int[] upTrianglePath(int x, int y, int width, int height) {
		return new int[] { 
				x, y + height,
				x + width / 2, y + height - height / 3,
				x + width, y + height
		};
	}
	
	private void redrawLineNumber(GC gc, boolean selectionChanged) {
		final Color white = TextColors.getWhite();
		final Color black = TextColors.getBlack();
		int charHeight = gc.getFontMetrics().getHeight();
		Rectangle clientRect = lineNumber.getClientArea();
		
		{
			ScrollBar bar = text.getHorizontalBar();
			bottomDisplayHeight = bar.getSize().y;
		}
		
		boolean[] cloneExists = new boolean[] { false, false };
		if (textString != null && textString.length() > 0) {
			int lineNumberCount = text.getLineCount();
			for (int i = 0; i < lineNumberCount; ++i) {
				if (lineStatus != null) {
					if ((lineStatus[i] & (WHOLE_LINE_COVERED | CLONE_END_LINE | CLONE_BEGIN_LINE)) != 0) {
						cloneExists[0] = true;
					}
					if ((lineStatus[i] & ((WHOLE_LINE_COVERED | CLONE_END_LINE | CLONE_BEGIN_LINE) << BETWEEN_FILE_SHIFT)) != 0) {
						cloneExists[1] = true;
					}
				}
			}
		}
		{
			final int w1 = clientRect.width / 2;
			final int w2 = clientRect.width - w1;
			
			int y = -text.getTopPixel();
			gc.setLineStyle(SWT.LINE_SOLID);
			gc.setLineWidth(1);
			if (textString != null && textString.length() > 0) {
				int lineNumberCount = text.getLineCount();
				for (int i = 0; i < lineNumberCount; ++i) {
					if (0 <= y + charHeight && y < clientRect.height) {
						int num = i + 1;
						String str = String.valueOf(num);
						Point extent = gc.stringExtent(str);
						if (lineStatus != null) {
							int lsi = lineStatus[i];
							int x1 = w1;
							int x2 = w2;
							for (int t = 0; t < 2; ++t) {
								if ((lsi & WHOLE_LINE_COVERED) != 0) {
									if ((lsi & WHOLE_LINE_SELECTED) != 0) {
										gc.setBackground(TextColors.getSelectedClonePair());
										gc.fillRectangle(x1, y, x2, charHeight);
										
										if ((lsi & CLONE_END_LINE) != 0) {
											gc.setForeground(white);
											gc.drawPolyline(downTrianglePath(x1, y, x2, charHeight));
										}
										if ((lsi & CLONE_BEGIN_LINE) != 0) {
											gc.setForeground(white);
											gc.drawPolyline(upTrianglePath(x1, y, x2, charHeight));
										}
									} else {
										gc.setBackground(TextColors.getClonePair());
										gc.fillRectangle(x1, y, x2, charHeight);
										
										gc.setBackground(TextColors.getSelectedClonePair());
										gc.setForeground(white);
										if ((lsi & CLONE_END_LINE) != 0) {
											gc.setForeground(white);
											gc.drawPolyline(downTrianglePath(x1, y, x2, charHeight));
										}
										if ((lsi & CLONE_BEGIN_LINE) != 0) {
											gc.setForeground(white);
											gc.drawPolyline(upTrianglePath(x1, y, x2, charHeight));
										}
										if ((lsi & SELECTION_END_LINE) != 0) {
											final int[] line = downTrianglePath(x1, y, x2, charHeight);
											gc.fillPolygon(line);
										}
										if ((lsi & SELECTION_BEGIN_LINE) != 0) {
											final int[] line = upTrianglePath(x1, y, x2, charHeight);
											gc.fillPolygon(line);
										}
									}
								} else {
									gc.setBackground(white);
									gc.fillRectangle(x1, y, x2, charHeight);
									if ((lsi & CLONE_END_LINE) != 0) {
										gc.setBackground((lsi & SELECTION_END_LINE) != 0 ? 
												TextColors.getSelectedClonePair() : TextColors.getClonePair());
										final int[] line = downTrianglePath(x1, y, x2, charHeight);
										gc.fillPolygon(line);
									}
									if ((lsi & CLONE_BEGIN_LINE) != 0) {
										gc.setBackground((lsi & SELECTION_BEGIN_LINE) != 0 ? 
												TextColors.getSelectedClonePair() : TextColors.getClonePair());
										final int[] line = upTrianglePath(x1, y, x2, charHeight);
										gc.fillPolygon(line);
									}
								}
								x1 = 0;
								x2 = w1;
								lsi >>>= BETWEEN_FILE_SHIFT;
							}
						} else {
							gc.setBackground(white);
						}
						gc.setForeground(black);
						gc.drawText(str, clientRect.width - extent.x, y, SWT.DRAW_TRANSPARENT);
						boolean inTheInitialPosition = initialTopPosition != -1 && i == initialTopPosition;
						if (inTheInitialPosition) {
							Point ex = gc.stringExtent("M"); //$NON-NLS-1$
							String s = "i"; //$NON-NLS-1$
							gc.setBackground(black);
							gc.fillRectangle(0, y, ex.x, charHeight);
							gc.setForeground(white);
							gc.drawText(s, (ex.x - 0) / 2, y, SWT.NONE);
						}
					}
					y += charHeight;
				}
			}
		}
		
		final int w1 = clientRect.width / 2;
		final int w2 = clientRect.width - w1;
		int y = -text.getTopPixel() + text.getLineCount() * charHeight;
		if (y < clientRect.height - bottomDisplayHeight) {
			gc.setBackground(white);
			gc.fillRectangle(0, y, clientRect.width, clientRect.height - bottomDisplayHeight - y);
		}
		{
			gc.setBackground(white);
			int y0 = clientRect.height - bottomDisplayHeight;
			gc.fillRectangle(0, y0, clientRect.width, bottomDisplayHeight);
			gc.setBackground(TextColors.getClonePair());
			final int margin = 2;
			if (cloneExists[0]) {
				gc.fillRectangle(w1 + margin, y0 + margin, w2 - margin * 2, bottomDisplayHeight - margin * 2);
			}
			if (cloneExists[1]) {
				gc.fillRectangle(0 + margin, y0 + margin, w1 - margin * 2, bottomDisplayHeight - margin * 2);
			}
		}
	}
	
	public Control getControl() {
		return text;
	}

	public PrepToken[] getTokens(int fileIndex) {
		if (fileIndex == this.fileIndex) {
			return tokens;
		}
		
		return null;
	}
	
	public ClonePair[] getClonePairs(int fileIndex) {
		if (fileIndex == this.fileIndex) {
			return clonePairs;
		}
		
		return null;
	}
	
	public void updateModel(Model data) {
		viewedModel = data;
		
		fileIndex = -1;
		clonePairs = null;
		selectedClonePairs = null;
		lineStatus = null;
		text.setText(""); //$NON-NLS-1$
		textString = textStringLower = null;
		tokens = null;
		tokenEndIndices = null;
		innerfileCloneSetIDsSelectedByRightClick = null;
		bothCloneSetIDsSelectedByRightClick = null;
		crossfileCloneSetIDsSelectedByRightClick = null;
		allCloneSetIDsSelectedByRightClick = null;
		
		searchingIndex = -1;
		searchingText = null;
		
		int newFileIndex = file != null ? data.findFile(file) : -1;
		if (newFileIndex >= 0) {
			setFile(newFileIndex);
		} else {
			fileNameLabel.setText("-"); //$NON-NLS-1$
			fileNameLabel.setToolTipText(""); //$NON-NLS-1$
			GC gc = new GC(lineNumber);
			try {
				redrawLineNumber(gc, true);
			} finally {
				gc.dispose();
			}
		}
	}

	private String readSourceFile(String path) throws FileNotFoundException,
			IOException {
		final File file = new File(path);
		final InputStream inp = new FileInputStream(file);
		final BufferedInputStream inpBuf = new BufferedInputStream(inp);
		if (encodingName.length() == 0) {
			Reader reader = new InputStreamReader(inpBuf, "UTF8"); //$NON-NLS-1$
			StringWriter writer = new StringWriter();
			int data;
			while ((data = reader.read()) != -1) {
				writer.write(data);
			}
			writer.flush();
			String str = writer.toString();
			reader.close();
			writer.close();
			return str;
		} else {
			final byte[] seq = new byte[inpBuf.available()];
			inpBuf.read(seq);
			inp.close();
			
			return Decoder.decode(seq, encodingName);
		}
	}
	
	public int[] getViewedFiles() {
		if (fileIndex != -1) {
			return new int[] { fileIndex };
		} else {
			return new int[0];
		}
	}
	
	public static class BeginEnd {
		public final int begin;
		public final int end;
		public BeginEnd(int begin, int end) {
			this.begin = begin;
			this.end = end;
		}
	}
	
	public void setVisibleTokenCenterIndex(int tokenIndex) {
		if (tokens == null || tokenEndIndices == null) {
			return;
		}
		
		boolean textVisible = text.isVisible();
		if (textVisible) {
			text.setVisible(false);
			lineNumber.setVisible(false);
		}
		try {
			if (tokenIndex < 0) {
				tokenIndex = 0;
			} else if (tokenIndex >= tokenEndIndices.length) {
				tokenIndex = tokenEndIndices.length - 1;
			}
			
			int charIndex = tokenEndIndices[tokenIndex];
			
			int lineIndex = text.getLineAtOffset(charIndex);
			
			final int height = text.getLineHeight();
			final int lineCount = text.getClientArea().height / height;
			
			int topLineIndex = lineIndex - lineCount / 2;
			if (topLineIndex < 0) {
				topLineIndex = 0;
			} else if (topLineIndex >= text.getLineCount()) {
				topLineIndex = text.getLineCount() - 1;
			}
			
			text.setTopIndex(topLineIndex);
			for (TextPaneScrollListener listener : TextPane.this.listeners) {
				listener.textScrolled();
			}
		}
		finally {
			if (textVisible) {
				text.setVisible(true);
				lineNumber.setVisible(true);
			}
		}
	}
	
	public BeginEnd getVisibleTokenRange() {
		if (tokens == null) {
			return null;
		}
		
		if (tokenEndIndices == null) {
			tokenEndIndices = new int[tokens.length];
			for (int i = 0; i < tokens.length; ++i) {
				tokenEndIndices[i] = tokens[i].endIndex;
			}
		}
		
		int topLine = text.getTopIndex();
		int topCharIndex = text.getOffsetAtLine(topLine);
		final int height = text.getLineHeight();
		final int lineCount = text.getClientArea().height / height;
		int bottomLine= topLine + lineCount;
		int bottomCharIndex = bottomLine < text.getLineCount() ? text.getOffsetAtLine(bottomLine) : text.getCharCount();
		
		int topTokenIndex = -1;
		int bottomTokenIndex = -1;
		int i = Arrays.binarySearch(tokenEndIndices, topCharIndex);
		if (i < 0) {
			i = -(i + 1);
		}
		if (i >= tokens.length) {
			i = tokens.length;
		}
		topTokenIndex = i;
		if (bottomLine >= text.getLineCount()) {
			return new BeginEnd(topTokenIndex, tokens.length);
		}
		i = Arrays.binarySearch(tokenEndIndices, bottomCharIndex);
		if (i < 0) {
			i = -(i + 1);
		}
		if (i >= tokens.length) {
			i = tokens.length;
		}
		bottomTokenIndex = i;
		return new BeginEnd(topTokenIndex, bottomTokenIndex);
	}
	
//	public BeginEnd getVisibleTokenRange(int fileIndex) {
//		if (fileIndex == this.fileIndex) {
//			return getVisibleTokenRange();
//		} else {
//			return null;
//		}
//	}

	private int calcWidthOfNumberString(int value) {
		int nineValue = 0;
		for (int v = value; v != 0; v = v / 10) {
			nineValue = nineValue * 10 + 9;
		}
		if (nineValue < 999999) {
			nineValue = 999999;
		}
		GC gc = new GC(this.lineNumber);
		Point size = gc.textExtent(String.valueOf(nineValue));
		gc.dispose();
		return size.x;
	}
	
	private static String toPrepFilePath(String path, String[] prepDirs) {
		for (int i = 0; i < prepDirs.length; ++i) {
			String prepDir = prepDirs[i];
			if (path.startsWith(prepDir)) {
				if (path.length() > prepDir.length() && path.charAt(prepDir.length()) == File.separatorChar) {
					return prepDir + File.separator + ".ccfxprepdir" + path.substring(prepDir.length()); //$NON-NLS-1$
				}
			}
		}
		return path;
	}
	
	private void setFile(int fileIndex) {
		this.fileIndex = fileIndex;
		file = viewedModel.getFile(fileIndex);
		String filenamestr = String.valueOf(file.id) + " " + file.path; //$NON-NLS-1$
		fileNameLabel.setText(filenamestr); //$NON-NLS-1$
		fileNameLabel.setToolTipText(filenamestr);
		CcfxDetectionOptions options = viewedModel.getDetectionOption();
		String postfix = options.getPostfix();
		String prepDirs[] = options.get("n"); //$NON-NLS-1$
		if (postfix == null) {
			postfix = "." + viewedModel.getPreprocessScript() + ".ccfxprep"; //$NON-NLS-1$ //$NON-NLS-2$
		}
		tokenEndIndices = null;
		
		String prepFilePath = toPrepFilePath(file.path, prepDirs);
		
		try {
			tokens = (new PrepReader()).read(prepFilePath, postfix);
		} catch (PrepReaderError e) {
			tokens = null;
		} catch (IOException e) {
			tokens = null;
		}
		
		selectedClonePairs = new int[0];
		lineStatus = null;
		
		boolean lineNumberAndTextVisible = lineNumberAndText.isVisible();
		if (lineNumberAndTextVisible) {
			lineNumberAndText.setVisible(false);
		}

		searchingIndex = -1;
		searchingText = null;
		
		try {
			String str = readSourceFile(file.path);
			if (tokens != null && tokens[tokens.length - 1].endIndex > str.length()) {
				tokens = null; // maybe the encoding used to generate the clone data is not the same as the encoding to the current encoding
			}
			textString = str;
			textStringLower = textString.toLowerCase();
			text.setText(str);
			lineStatus = new int[text.getLineCount() + 1];
			//Point size = lineNumber.computeSize(lineNumberWidth, SWT.DEFAULT);
			{
				int width = calcWidthOfNumberString(text.getLineCount() + 1);
				GridData gridData = new GridData(SWT.NONE, SWT.FILL, false, true);
				gridData.widthHint = width;
				gridData.heightHint = 200;
				lineNumber.setLayoutData(gridData);
			}
			this.lineNumberAndText.layout();

			clonePairs = viewedModel.getClonePairsOfFile(fileIndex).clone();
			Arrays.sort(clonePairs);
			
			setTextHighlightsAndLineStatus(false);
			//rebuildLineNumberImage();
		} catch (FileNotFoundException e) {
			textString = textStringLower = ""; //$NON-NLS-1$
			text.setText(textString); //$NON-NLS-1$
		} catch (IOException e) {
			textString = textStringLower = ""; //$NON-NLS-1$
			text.setText(textString); //$NON-NLS-1$
		} finally {
			if (lineNumberAndTextVisible) {
				lineNumberAndText.setVisible(true);
			}
		}
	}
	
	public void setSelection(int[] fileIndices) {
		if (viewedModel != null && fileIndices != null && fileIndices.length == 1) {
			if (fileIndex != fileIndices[0]) {
				setFile(fileIndices[0]);
				initialTopPosition = 0;
				initialTokenPosition = 0;
			}
		} else {
			fileIndex = -1;
			clonePairs = null;
			selectedClonePairs = null;
			lineStatus = null;
			text.setText(""); //$NON-NLS-1$
			textString = textStringLower = null;
			tokens = null;
			tokenEndIndices = null;
			
			initialTopPosition = 0;
			initialTokenPosition = 0;
			
			searchingIndex = -1;
			searchingText = null;
			
			fileNameLabel.setText("-"); //$NON-NLS-1$
			fileNameLabel.setToolTipText(""); //$NON-NLS-1$
			GC gc = new GC(lineNumber);
			try {
				this.redrawLineNumber(gc, true);
			} finally {
				gc.dispose();
			}
			for (TextPaneScrollListener listener : TextPane.this.listeners) {
				listener.textScrolled();
			}
		}
	}
	
	private void set_line_status(int index, ClonePair p, int status) {
		lineStatus[index] |= (p.leftFile == p.rightFile) ? status : (status << BETWEEN_FILE_SHIFT);
	}
	private void set_line_status(int beginIndex, int endIndex, ClonePair p, int status) {
		int value = (p.leftFile == p.rightFile) ? status : (status << BETWEEN_FILE_SHIFT);
		for (int i = beginIndex; i < endIndex; ++i) {
			lineStatus[i] |= value;
		}
	}
	
	private static boolean areWhiteSpaces(String str, int begin, int end) {
		for (int i = begin; i < end; ++i) {
			char ch = str.charAt(i);
			switch (ch) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '\f':
				break;
			default:
				return false;
			}
		}
		return true;
	}
	
	private static int findNoWhiteSpace(String str, int begin) {
		int i = begin;
		while (i < str.length()) {
			char ch = str.charAt(i);
			switch (ch) {
			case ' ':
			case '\t':
			case '\r':
			case '\n':
			case '\f':
				break;
			default:
				return i;
			}
			++i;
		}
		return i;
	}
	
	private boolean[] appearingOnlyWhitespacesBeforeToken() {
		final String sourceText = textString != null ? textString : "";
		boolean[] values = new boolean[tokens.length];
		for (int i = 0; i < tokens.length - 1; ++i) {
			final PrepToken curToken = tokens[i];
			final PrepToken nextToken = tokens[i + 1];
			if (areWhiteSpaces(sourceText, curToken.endIndex, nextToken.beginIndex)) {
				values[i] = true;
			}
		}
		return values;
	}
	
	private void setTokenRangeColor(int begin, int end, Color bgcolor,
			boolean[] appearingOnlyWhitespacesBeforeTokenData) {
		StyleRange negStyleRange = new StyleRange();
		negStyleRange.foreground = TextColors.getNeglectedText();
		negStyleRange.background = bgcolor;
		
		StyleRange resStyleRange = new StyleRange();
		resStyleRange.foreground = TextColors.getReservedWord();
		resStyleRange.fontStyle = SWT.BOLD;
		resStyleRange.background = bgcolor;
		
		StyleRange txtStyleRange = new StyleRange();
		txtStyleRange.foreground = TextColors.getBlack();
		txtStyleRange.background = bgcolor;
		
		final String sourceText = textString != null ? textString : "";
		int textSize = text.getCharCount();
		int charIndex = tokens[begin].beginIndex;
		int i = begin;
		while (i < end) {
			final PrepToken token = tokens[i];
			int tokenEndIndex = token.endIndex;
			if (tokenEndIndex > textSize) {
				break; // while
			}
			//int line = text.getLineAtOffset(token.beginIndex);
			if (charIndex < token.beginIndex) {
				StyleRange styleRange = (StyleRange)negStyleRange.clone();
				styleRange.start = charIndex;
				styleRange.length = token.beginIndex - charIndex;
				text.setStyleRange(styleRange);
			}
			int len = 1;
			{
				PrepToken nextToken;
				while (i + len < end && (nextToken = tokens[i + len]).isReservedWord == token.isReservedWord
						&& appearingOnlyWhitespacesBeforeTokenData[i + len - 1]) {
					tokenEndIndex = nextToken.endIndex;
					++len;
				}
			}
			final StyleRange styleRange = token.isReservedWord ? (StyleRange)resStyleRange.clone() : (StyleRange)txtStyleRange.clone();
			styleRange.start = tokens[i].beginIndex;
			int endIndex = (i + len < end) ? findNoWhiteSpace(sourceText, tokens[i + len - 1].endIndex)
					: tokens[i + len - 1].endIndex;
			styleRange.length = endIndex - tokens[i].beginIndex;
			text.setStyleRange(styleRange);
			charIndex = endIndex;
			i += len;
		}
		
		final PrepToken beginToken = tokens[begin];
		final PrepToken endToken = tokens[end - 1];
		int beginLine = text.getLineAtOffset(beginToken.beginIndex);
		int endLine = text.getLineAtOffset(endToken.endIndex - 1);
		text.setLineBackground(beginLine, endLine - beginLine, bgcolor);
	}
	
	private void setTextHighlightsAndLineStatus(boolean updateSelectionOnly) {
		if (textString == null || textString.length() == 0 || tokens == null) {
			return;
		}
		
		final int strLength = textString.length();
		//final boolean updateUnselected = ! updateSelectionOnly;
		
		final boolean[] appearingOnlyWhitespacesBeforeTokenData = appearingOnlyWhitespacesBeforeToken();
		setTokenRangeColor(0, tokens.length, TextColors.getWhite(), appearingOnlyWhitespacesBeforeTokenData);
		{
			{
				utility.BitArray tokenDones = new utility.BitArray(tokens.length * 3);
				for (int i = 0; i < clonePairs.length; ++i) {
					ClonePair p = clonePairs[i];
					if (Arrays.binarySearch(selectedClonePairs, i) < 0) {
						if (p.leftFile == fileIndex) {
							if (0 <= p.leftBegin && p.leftBegin < p.leftEnd && p.leftEnd <= tokens.length) {
								tokenDones.fill(p.leftBegin * 3 + 1, p.leftEnd * 3 - 1, true);
							}
						}
					}
				}
				{
					int i = tokenDones.find(true);
					while (i >= 0) {
						int j = tokenDones.find(false, i);
						if (j < 0) {
							j = tokenDones.length();
						}
						
						// here, tokenDones[i] .. tokenDones[j - 1] are true. tokenDones[j] = false
						
						setTokenRangeColor(i / 3, j / 3, TextColors.getClonePair(), appearingOnlyWhitespacesBeforeTokenData);
						i = tokenDones.find(true, j);
					}
				}
			}
			
			{
				Arrays.fill(lineStatus, 0);
				for (int i = 0; i < clonePairs.length; ++i) {
					boolean selected = Arrays.binarySearch(selectedClonePairs, i) >= 0;
					final ClonePair p = clonePairs[i];
					if (p.leftFile == fileIndex) {
						if (0 <= p.leftBegin && p.leftBegin < p.leftEnd && p.leftEnd <= tokens.length) {
							int beginPos = tokens[p.leftBegin].beginIndex;
							int endPos = tokens[p.leftEnd - 1].endIndex;
		
							if (0 <= beginPos && beginPos < endPos && endPos <= strLength) {
								if (!(endPos < strLength)) {
									endPos = strLength - 1;
								}
								int beginLine = text.getLineAtOffset(beginPos);
								int endLine = text.getLineAtOffset(endPos - 1);
								if (beginLine == endLine) {
									set_line_status(beginLine, p, WHOLE_LINE_COVERED | (selected ? WHOLE_LINE_SELECTED : 0));
								}
								else {
									set_line_status(beginLine, p, CLONE_BEGIN_LINE | (selected ? SELECTION_BEGIN_LINE : 0));
									set_line_status(beginLine + 1, endLine, p, WHOLE_LINE_COVERED | (selected ? WHOLE_LINE_SELECTED : 0));
									set_line_status(endLine, p, CLONE_END_LINE | (selected ? SELECTION_END_LINE : 0));
								}
							}
						}
					}
				}
			}
			
			for (int i = 0; i < selectedClonePairs.length; ++i) {
				final ClonePair p = clonePairs[selectedClonePairs[i]];
				if (p.leftFile == fileIndex) {
					if (0 <= p.leftBegin && p.leftBegin < p.leftEnd && p.leftEnd <= tokens.length) {
						setTokenRangeColor(p.leftBegin, p.leftEnd, TextColors.getSelectedClonePair(), appearingOnlyWhitespacesBeforeTokenData);
					}
				}
			}
		}
	}
	
	public void setCodeFragmentSelection(CodeFragment selectedCodeFragment, long cloneSetID) {
		long[] cloneSetIDs = new long[] { cloneSetID };
		setCloneSelection(cloneSetIDs, selectedCodeFragment, true);
	}
	
	public void setCloneSelection(long[] cloneSetIDs, CloneSelectionListener src) {
		if (src == this) {
			return;
		}
		
		setCloneSelection(cloneSetIDs, null, true);
	}
	
	private void setCloneSelection(long[] cloneSetIDs, 
				CodeFragment targetCodeFragment, boolean scrollToIt) {
		if (clonePairs == null) {
			selectedClonePairs = null;
			return;
		}
		
		long[] ids = cloneSetIDs.clone();
		Arrays.sort(ids);
		
		int[] clonePairIndices;
		{
			TIntArrayList cpis = new TIntArrayList();
			for (int j = 0; j < clonePairs.length; ++j) {
				ClonePair p = clonePairs[j];
				if (Arrays.binarySearch(ids, p.classID) >= 0) {
					cpis.add(j);
				}
			}
			clonePairIndices = cpis.toNativeArray();
			Arrays.sort(clonePairIndices);
		}
		
		boolean selectedClonePairUnchanged = true;
		if (clonePairIndices.length != selectedClonePairs.length) {
			selectedClonePairUnchanged = false;
		}
		else {
			for (int i = 0; i < clonePairIndices.length; ++i) {
				if (clonePairIndices[i] != selectedClonePairs[i]) {
					selectedClonePairUnchanged = false;
					break; // for i
				}
			}
		}
		if (! selectedClonePairUnchanged) {
			selectedClonePairs = clonePairIndices;
		}
		
		boolean textVisible = text.isVisible();
		if (textVisible) {
			text.setVisible(false);
			lineNumber.setVisible(false);
		}
		try {
			if (! selectedClonePairUnchanged) {
				setTextHighlightsAndLineStatus(true);
				//rebuildLineNumberImage();
			}
			
			if (tokens != null && scrollToIt) {
				if (targetCodeFragment != null && targetCodeFragment.file == fileIndex) {
					// move caret to the left code fragment of the clone pair
					//int curIndex = text.getTopIndex();
					int cloneIndex = text.getLineAtOffset(tokens[targetCodeFragment.begin].beginIndex);
					
					TextPane.this.textScrollRequest = new ScrollRequest(cloneIndex);
					//text.setTopIndex(cloneIndex);
					
					if (initialTopPosition == -1) {
						initialTopPosition = cloneIndex;
						initialTokenPosition = targetCodeFragment.begin;
					}
				} else {
					// move caret to the nearest clone
					int curIndex = text.getTopIndex();
					int nearestCloneIndex = -1;
					int nearestCloneTokenPosition = -1;
					int minDistance = Integer.MAX_VALUE;
					for (int i = 0; i < clonePairIndices.length; ++i) {
						ClonePair pair = clonePairs[clonePairIndices[i]];
						if (pair.leftFile == fileIndex) {
							int cloneIndex = text.getLineAtOffset(tokens[pair.leftBegin].beginIndex);
							int distance = cloneIndex - curIndex;
							if (distance < 0) {
								if (-distance < minDistance) {
									nearestCloneIndex = cloneIndex;
									nearestCloneTokenPosition = pair.leftBegin;
									minDistance = -distance;
								}
							} else {
								if (distance < minDistance) {
									nearestCloneIndex = cloneIndex;
									nearestCloneTokenPosition = pair.leftBegin;
									minDistance = distance;
								}
							}
						}
					}
					if (nearestCloneIndex != -1) {
						//text.setTopIndex(nearestCloneIndex);
						TextPane.this.textScrollRequest = new ScrollRequest(nearestCloneIndex);
						
						if (initialTopPosition == -1) {
							initialTopPosition = nearestCloneIndex;
							initialTokenPosition = nearestCloneTokenPosition;
						}
					}
				}
			}
		} finally {
			if (textVisible) {
				text.setVisible(true);
				lineNumber.setVisible(true);
			}
		}
	}
	
	void dispose() {
		fileNameLabel.dispose();
		lineNumberAndText.dispose();
		text.dispose();
		lineNumber.dispose();
	}

	public SearchboxListener getSearchboxListener() {
		return new SearchboxListener() {
			public void searchCanceled(SearchboxData data) {
				searchingIndex = -1;
				searchingText = null;
			}
			public void searchForward(SearchboxData data) {
				searchingText = data.text;
				if (textString != null && text != null) {
					int index = text.getCaretOffset();
					if (index >= 0) {
						if (index == searchingIndex) {
							++index;
						}
					} else {
						index = 0;
					}
					int pos;
					if (data.isIgnoreCase) {
						String searchingTextLower = searchingText.toLowerCase();
						pos = textStringLower.indexOf(searchingTextLower, index);
					} else {
						pos = textString.indexOf(searchingText, index);
					}
					if (pos >= 0) {
						text.setSelection(pos, pos + searchingText.length());
						searchingIndex = text.getCaretOffset();
						GC gc = new GC(lineNumber);
						try {
							TextPane.this.redrawLineNumber(gc, true);
						} finally {
							gc.dispose();
						}
						for (TextPaneScrollListener listener : TextPane.this.listeners) {
							listener.textScrolled();
						}
					}
				}
			}
			public void searchBackward(SearchboxData data) {
				searchingText = data.text;
				if (textString != null && text != null) {
					int index = text.getCaretOffset();
					if (index >= 0) {
						if (index == searchingIndex) {
							--index;
						}
					} else {
						index = textString.length() - 1;
					}
					index -= searchingText.length();
					if (index >= 0) {
						int pos;
						if (data.isIgnoreCase) {
							String searchingTextLower = searchingText.toLowerCase();
							pos = textStringLower.lastIndexOf(searchingTextLower, index);
						} else {
							pos = textString.lastIndexOf(searchingText, index);
						}
						if (pos >= 0) {
							text.setSelection(pos, pos + searchingText.length());
							searchingIndex = text.getCaretOffset();
							GC gc = new GC(lineNumber);
							try {
								TextPane.this.redrawLineNumber(gc, true);
							} finally {
								gc.dispose();
							}
							for (TextPaneScrollListener listener : TextPane.this.listeners) {
								listener.textScrolled();
							}
						}
					}
				}
			}
		};
	}
}

