package gemx;

import gemx.scatterplothelper.*;
import gnu.trove.TIntArrayList;

import java.util.*;

import model.*;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.ScrolledComposite;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.*;

import res.Messages;

public class ScatterPlotPane implements FileSelectionListener, CloneSelectionListener {
	private static class DirectoryBoxPosition {
		public final String path;
		public final int beginPos;
		public final int endPos;
		public final int beginFileIndex;
		public final int endFileIndex;
		public DirectoryBoxPosition(String path, int beginPos, int endPos, int beginFileIndex, int endFileIndex) {
			this.path = path;
			this.beginPos = beginPos;
			this.endPos = endPos;
			this.beginFileIndex = beginFileIndex;
			this.endFileIndex = endFileIndex;
		}
		public boolean isInsideOrEqualTo(DirectoryBoxPosition right) {
			if (this.beginFileIndex < right.beginFileIndex) {
				return false;
			} else if (this.beginFileIndex == right.beginFileIndex) {
				return this.endFileIndex <= right.endFileIndex;
			} else { // this.beginFileIndex > right.beginFileIndex
				return this.endFileIndex <= right.endFileIndex;
			}
		}
		public boolean isOutsideOrEqualTo(DirectoryBoxPosition right) {
			return right.isInsideOrEqualTo(this);
		}
		public boolean isNestingWithOrEqualTo(DirectoryBoxPosition right) {
			if (this.beginFileIndex < right.beginFileIndex) {
				return this.endFileIndex >= right.endFileIndex;
			} else if (this.beginFileIndex == right.beginFileIndex) {
				return true;
			} else { // this.beginFileIndex > right.beginFileIndex
				return this.endFileIndex <= right.endFileIndex;
			}
		}
	}
	private static class DirectoryBoxPositionComparator implements Comparator<DirectoryBoxPosition> {
		public int compare(DirectoryBoxPosition left, DirectoryBoxPosition right) {
			/*
			 * if the two directoris are nesting, then (inner one) < (outer one).
			 * otherwise, (one that appears before) < (one that appears after).
			 */
			if (left.beginFileIndex < right.beginFileIndex) {
				// left is outer of right or appers before right
				return -1; 
			} else if (left.beginFileIndex == right.beginFileIndex) {
				if (left.endFileIndex < right.endFileIndex) {
					// left is inner of right
					return 1;
				} else if (left.endFileIndex == right.endFileIndex) {
					// left is equal to right
					return 0;
				} else {
					// left is outer of right
					return -1;
				}
			} else {
				// left is inner of right or appears after right
				return 1;
			}
		}
	}
	
	protected MainWindow mainWindow;

	private Composite sc;
	private ScrolledComposite drawPanel;

	protected Canvas canvas;
	private Menu popupMenu;

	private Button ckFileBoundary;
	private Button ckFileGap;
	private Button ckDirectoryBox;
	private Combo cmColoringMetricName;
	private Button ckCloneSpace;
	private boolean showCloneSpace;
	
	private Image image;
	private int imageBaseSize;
	private int imageEnlarge;
	private DirectoryBoxPosition[] directoryBoxPositions;

	private Display display;

	private long[] fileStartPos;

	private Point curMousePosition;
	private Point dragStart;
	private Point dragCurrent;
	private Rectangle dragRect;
	boolean directoryLabelShownInLowerRight;

	private long zoom;
	private int[] selectedIndex;
	private ClonePair[] selectedClonePairs;

	private Model viewedModel;
	private ClonesetMetricModel viewedCloneSetMetricModel;
	private CloneSetMetricExtractor cloneSetMetricExtractor;
	
	private boolean showFileBoundary = true;
	private boolean showFileGap = true;
	private boolean showDirectoryBox = true;
	private int coloringMetric = -1; // -1 means not colored with any metric
	
	private String strNotApplicable = "N/A"; //$NON-NLS-1$
	
	public Control getControl() {
		return sc;
	}

	public void addListener(int eventType, Listener listener) {
		assert eventType == SWT.FocusIn;
		this.canvas.addListener(eventType, listener);
	}
	
	public boolean isShowFileBoundary() {
		return showFileBoundary;
	}
	
	public void setShowFileBoundary(boolean toBeShown) {
		showFileBoundary = toBeShown;
		rebuildImage();
		assert canvas != null;
		canvas.redraw();
	}
	
	public boolean isShowFileGap() {
		return showFileGap;
	}
	
	public void setShowFileGap(boolean toBeShown) {
		showFileGap = toBeShown;
		rebuildImage();
		assert canvas != null;
		canvas.redraw();
	}
	
	public boolean isShowDirectoryBox() {
		return showDirectoryBox;
	}
	
	public void setShowDirectoryBox(boolean toBeShown) {
		showDirectoryBox = toBeShown;
		rebuildImage();
		assert canvas != null;
		canvas.redraw();
	}

	public void setColoringMetric(int metricID) {
		coloringMetric = metricID;
		if (metricID == -1 || viewedCloneSetMetricModel == null) {
			cloneSetMetricExtractor = null;
		} else {
			try {
				cloneSetMetricExtractor = CloneSetMetricExtractor.newCloneSetMetricExtractorByID(viewedCloneSetMetricModel, metricID);
			} catch (IndexOutOfBoundsException e) {
				cloneSetMetricExtractor = null;
			}
		}
		rebuildImage();
		assert canvas != null;
		canvas.redraw();
	}
	
	public int getColoringMetric() {
		return coloringMetric;
	}
	
	public boolean isShowCloneSpace() {
		return showCloneSpace;
	}
	
	public void setShowCloneSpace(boolean toBeShown) {
		showCloneSpace = toBeShown;
		rebuildImage();
		assert canvas != null;
		canvas.redraw();
	}

//	private static double distance(Point p1, Point p2) {
//		int dx = p1.x - p2.x;
//		int dy = p1.y - p2.y;
//		return Math.sqrt(dx * dx + dy * dy);
//	}
	
	public ScatterPlotPane(Composite parent, int maxSize, boolean bAddResetScopeItemToContextMenu, 
			MainWindow mainWindow) {
		this.mainWindow = mainWindow;
		this.display = parent.getDisplay();
		Shell shell = parent.getShell();
		
		sc = new Composite(parent, SWT.NONE);
		{
			GridLayout layout = new GridLayout(1, false);
			layout.marginHeight = 0;
			layout.marginWidth = 0;
			sc.setLayout(layout);
		}
		Composite buttonPanel = new Composite(sc, SWT.NONE);
		{
			GridLayout layout = new GridLayout(6, false);
			layout.marginHeight = 0;
			layout.marginWidth = 0;
			buttonPanel.setLayout(layout);
		}
		buttonPanel.setLayoutData(new GridData(GridData.HORIZONTAL_ALIGN_FILL));
		{
			ckFileBoundary = new Button(buttonPanel, SWT.CHECK);
			ckFileBoundary.setText(Messages.getString("gemx.ScatterPlotPane.S_FILE_BOUNDARY")); //$NON-NLS-1$
			ckFileBoundary.setEnabled(true);
			ckFileBoundary.setSelection(showFileBoundary);
			ckFileBoundary.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ScatterPlotPane.this.setShowFileBoundary(ScatterPlotPane.this.ckFileBoundary.getSelection());
				}
			});
			ckFileGap = new Button(buttonPanel, SWT.CHECK);
			ckFileGap.setText(Messages.getString("gemx.ScatterPlotPane.S_FILE_GAP")); //$NON-NLS-1$
			ckFileGap.setEnabled(true);
			ckFileGap.setSelection(showFileGap);
			ckFileGap.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ScatterPlotPane.this.setShowFileGap(ScatterPlotPane.this.ckFileGap.getSelection());
				}
			});
			ckDirectoryBox = new Button(buttonPanel, SWT.CHECK);
			ckDirectoryBox.setText(Messages.getString("gemx.ScatterPlotPane.S_DIRECTORY_BOX")); //$NON-NLS-1$
			ckDirectoryBox.setEnabled(true);
			ckDirectoryBox.setSelection(showDirectoryBox);
			ckDirectoryBox.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ScatterPlotPane.this.setShowDirectoryBox(ScatterPlotPane.this.ckDirectoryBox.getSelection());
				}
			});
			
			ckCloneSpace = new Button(buttonPanel, SWT.CHECK);
			ckCloneSpace.setText(Messages.getString("gemx.ScatterPlotPane.S_DIAGONALIZE_FILL")); //$NON-NLS-1$
			ckCloneSpace.setEnabled(true);
			showCloneSpace = true;
			ckCloneSpace.setSelection(showCloneSpace);
			ckCloneSpace.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					ScatterPlotPane.this.setShowCloneSpace(ScatterPlotPane.this.ckCloneSpace.getSelection());
				}
			});
			
			Label lbColoringMetricName = new Label(buttonPanel, SWT.NONE);
			lbColoringMetricName.setText(Messages.getString("gemx.ScatterPlotPane.S_COLORING_METRICS")); //$NON-NLS-1$
			
			cmColoringMetricName = new Combo(buttonPanel, SWT.DROP_DOWN | SWT.READ_ONLY);
			cmColoringMetricName.add(strNotApplicable);
			cmColoringMetricName.setText(strNotApplicable);
			cmColoringMetricName.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					String name = cmColoringMetricName.getText();
					int metricID = -1;
					if (! name.equals(strNotApplicable)) {
						try {
							metricID = model.ClonesetMetricModel.fieldNameToID(name);
						} catch (IndexOutOfBoundsException e2) {
							// do mothing
						}
					}
					ScatterPlotPane.this.setColoringMetric(metricID);
				}
			});
			
		}
		
		imageBaseSize = maxSize;
		imageEnlarge = 1;
		image = new Image(display, maxSize, maxSize);
		{
			Color green = PlottingColors.getCloneAreaBackgorund();
			GC gc = new GC(image);
			try {
				gc.setBackground(green);
				gc.fillRectangle(0, 0, maxSize, maxSize);
			} finally {
				gc.dispose();
			}
		}
		
		drawPanel = new ScrolledComposite(sc, SWT.V_SCROLL | SWT.H_SCROLL);
		drawPanel.setLayoutData(new GridData(GridData.FILL_HORIZONTAL | GridData.FILL_VERTICAL));
		canvas = new Canvas(drawPanel, SWT.NONE);
		canvas.addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				redraw(e.gc);
			}
		});
		canvas.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				boolean rightClick = e.button == 3;
				if (rightClick) {
					canvas.setMenu(popupMenu);
					return;
				} else if (e.button == 1) {
					canvas.forceFocus();
					dragStart = new Point(e.x, e.y);
					dragCurrent = dragStart;
					dragRect = null;
					redraw(new GC(canvas));
				}
			}

			public void mouseUp(MouseEvent e) {
				if (e.button == 1) {
					if (dragStart != null) {
						dragRect = new Rectangle(dragStart.x, dragStart.y,
								dragCurrent.x - dragStart.x, dragCurrent.y
										- dragStart.y);
						dragStart = null;
						dragCurrent = null;
						selectingByMouse(dragRect);
					}
					redraw(new GC(canvas));
				}
			}
		});
		canvas.addMouseMoveListener(new MouseMoveListener() {
			public void mouseMove(MouseEvent e) {
				ScatterPlotPane.this.curMousePosition = new Point(e.x, e.y);
				boolean haveToRedraw = false;
				
				if (dragStart != null) {
					dragCurrent = new Point(e.x, e.y);
					haveToRedraw = true;
				}
				
				boolean altKeyDown = (e.stateMask & SWT.ALT) != 0;
				boolean mouseInUpperLeft = e.x >= e.y;
				if (! altKeyDown && mouseInUpperLeft != directoryLabelShownInLowerRight) {
					directoryLabelShownInLowerRight = mouseInUpperLeft;
					haveToRedraw = true;
				}
				
				if (haveToRedraw) {
					redraw(new GC(canvas));
				}
			}
		});
		drawPanel.setContent(canvas);
		int drawSize;
		if (! ScatterPlotPane.this.mainWindow.getMain().settingResizeScatterPlot) {
			drawSize = maxSize;
			if (drawSize < 100) {
				drawSize = 100;
			}
		} else {
			drawSize = 100;
		}
		drawPanel.setMinSize(drawSize, drawSize);
		drawPanel.setExpandHorizontal(true);
		drawPanel.setExpandVertical(true);
		drawPanel.addControlListener(new ControlAdapter() {
			public void controlResized(ControlEvent e) {
				if (ScatterPlotPane.this.mainWindow.getMain().settingResizeScatterPlot) {
					final Rectangle rect = ScatterPlotPane.this.drawPanel.getBounds();
					int shorter = rect.width;
					if (shorter > rect.height) {
						shorter = rect.height;
					}
					int drawSize = shorter * imageEnlarge;
					if (drawSize <= 0) {
						drawSize = 1;
					}
					ScatterPlotPane.this.drawPanel.setMinSize(rect.width, rect.height);
					
					imageBaseSize = shorter;
					Rectangle imageR = image.getBounds();
					if (imageR.width != drawSize || imageR.height != drawSize) {
						image.dispose();
						image = new Image(display, drawSize, drawSize);
						{
							Color green = PlottingColors.getCloneAreaBackgorund();
							GC gc = new GC(image);
							try {
								gc.setBackground(green);
								gc.fillRectangle(0, 0, drawSize, drawSize);
							} finally {
								gc.dispose();
							}
						}
						rebuildImage();
						redraw(new GC(canvas));
					}
				}
			}
	     });

		{
			Menu pmenu = new Menu(shell, SWT.POP_UP);

			{
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.ScatterPlotPane.M_SELECT_INCLUDING_DIRECTORY_BOX")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						if (directoryBoxPositions != null && ScatterPlotPane.this.curMousePosition != null) {
							Point mousePos = ScatterPlotPane.this.curMousePosition;
							if (mousePos.x > mousePos.y) {
								mousePos = new Point(mousePos.y, mousePos.x);
							}
							DirectoryBoxPosition smallestDBP = null;
							for (DirectoryBoxPosition dbp : directoryBoxPositions) {
								if (dbp.beginPos <= mousePos.x && mousePos.y <= dbp.endPos) {
									if (smallestDBP == null || smallestDBP.endPos - smallestDBP.beginPos > dbp.endPos - dbp.beginPos) { 
										smallestDBP = dbp;
									}
								}
							}
							if (smallestDBP != null) {
								int[] indices = new int[smallestDBP.endFileIndex - smallestDBP.beginFileIndex];
								for (int i = 0; i < indices.length; ++i) {
									indices[i] = smallestDBP.beginFileIndex + i;
								}
								selectedIndex = indices;
								ScatterPlotPane.this.setSelection(selectedIndex);
								ScatterPlotPane.this.mainWindow.setFileSelection(selectedIndex, ScatterPlotPane.this);
							}
						}
					}
				});
			}
			new MenuItem(pmenu, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.ScatterPlotPane.M_FIT_SCOPE_TO_SELECTED_FILES")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						if (selectedIndex != null) {
							ScatterPlotPane.this.mainWindow
									.fitScopeToFiles(selectedIndex);
						}
					}
				});
			}
			new MenuItem(pmenu, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.ScatterPlotPane.M_POP_SCOPE")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						ScatterPlotPane.this.mainWindow.popScope();
					}
				});
			}
			
			if (bAddResetScopeItemToContextMenu) {
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.ScatterPlotPane.M_RESET_SCOPE")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						ScatterPlotPane.this.mainWindow.resetScope();
					}
				});
			}
			
			new MenuItem(pmenu, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.ScatterPlotPane.M_RESIZE_DRAWING_AREA")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						ScatterPlotPane.this.mainWindow.resizeScatterPlotDrawingAreaToWindow(1);
					}
				});
			}
			{
				MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.ScatterPlotPane.S_RESIZE_DRAWING_AREA_2")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						ScatterPlotPane.this.mainWindow.resizeScatterPlotDrawingAreaToWindow(2);
					}
				});
			}

			popupMenu = pmenu;
		}
	}
	
	public void resizeDrawingAreaToWindow(Model model, int times) {
		Rectangle r = drawPanel.getBounds();
		int shorter = r.width;
		if (shorter > r.height) {
			shorter = r.height;
		}
		int drawSize = shorter * times;
		if (drawSize < 100) {
			drawSize = 100;
		}
		
		imageBaseSize = shorter;
		imageEnlarge = times;
		assert drawSize == imageBaseSize * imageEnlarge;
		
		Rectangle imageR = image.getBounds();
		if (imageR.width != drawSize || imageR.height != drawSize) {
			image.dispose();
			image = new Image(display, drawSize, drawSize);
			{
				Color green = PlottingColors.getCloneAreaBackgorund();
				GC gc = new GC(image);
				try {
					gc.setBackground(green);
					gc.fillRectangle(0, 0, drawSize, drawSize);
				} finally {
					gc.dispose();
				}
			}
			rebuildImage();
			//redraw(new GC(canvas));
		}
		
		drawPanel.setMinSize(drawSize, drawSize);
	}

	public void resizeDrawingAreaToWindow(Model model, CloneSetMetricExtractor metricExtractor, int times) {
		Rectangle r = drawPanel.getBounds();
		int shorter = r.width;
		if (shorter > r.height) {
			shorter = r.height;
		}
		int drawSize = shorter * times;
		if (drawSize < 100) {
			drawSize = 100;
		}
		
		imageBaseSize = shorter;
		imageEnlarge = times;
		assert drawSize == imageBaseSize * imageEnlarge;
		
		Rectangle imageR = image.getBounds();
		if (imageR.width != drawSize || imageR.height != drawSize) {
			image.dispose();
			image = new Image(display, drawSize, drawSize);
			{
				Color green = PlottingColors.getCloneAreaBackgorund();
				GC gc = new GC(image);
				try {
					gc.setBackground(green);
					gc.fillRectangle(0, 0, drawSize, drawSize);
				} finally {
					gc.dispose();
				}
			}
			rebuildImage();
			//redraw(new GC(canvas));
		}
		
		drawPanel.setMinSize(drawSize, drawSize);
	}

	private static class FileAndOffset {
		public int fileIndex;
		public int offset;
		public FileAndOffset(int fileIndex, int offset) {
			this.fileIndex = fileIndex;
			this.offset = offset;
		}
	}
	private FileAndOffset getFileAndOffset(int x) {
		return getFileAndOffset(x, true);
	}
	private FileAndOffset getFileAndOffset(int x, boolean rounding) {
		long lxb = zoom * (long) x;
		int xFileBegin = Arrays.binarySearch(fileStartPos, lxb);
		if (rounding) {
			if (xFileBegin == -1) {
				xFileBegin = 0;
			} else if (xFileBegin < 0) {
				xFileBegin = -xFileBegin - 2;
			}
			if (xFileBegin > fileStartPos.length - 2) {
				// the last one is a terminator, not a file.
				xFileBegin = fileStartPos.length - 2;
			}
		} else {
			if (xFileBegin == -1) return null;
			if (xFileBegin < 0) {
				xFileBegin = -xFileBegin - 2;
			}
			if (xFileBegin > fileStartPos.length - 2) return null;
		}
		
		long lxbOffset = lxb - fileStartPos[xFileBegin];
		long fileSize = fileStartPos[xFileBegin + 1] - fileStartPos[xFileBegin];
		if (lxbOffset < 0) {
			lxbOffset = 0;
		} else if (lxbOffset > fileSize) {
			lxbOffset = fileSize;
		}
		return new FileAndOffset(xFileBegin, (int)lxbOffset);
	}
	
	private void selectingByMouse(Rectangle rect) {
		if (fileStartPos == null || fileStartPos.length == 0) {
			return;
		}
		
		//int xMouseDown = rect.x;
		int xBegin = rect.x;
		int xEnd = rect.x + rect.width;
		if (rect.width < 0) {
			int tmp = xBegin;
			xBegin = xEnd;
			xEnd = tmp;
		}

		boolean isEmptyRect = rect.width <= 2 && rect.height <= 2 && getFileAndOffset(xBegin, false) == null;
		
		FileAndOffset xb = getFileAndOffset(xBegin);
		FileAndOffset xe = getFileAndOffset(xEnd);
		
		//int yMouseDown = rect.y;
		int yBegin = rect.y;
		int yEnd = rect.y + rect.height;
		if (rect.height < 0) {
			int tmp = yBegin;
			yBegin = yEnd;
			yEnd = tmp;
		}

		FileAndOffset yb = getFileAndOffset(yBegin);
		FileAndOffset ye = getFileAndOffset(yEnd);
		
		//int xMouseDownFile = (xMouseDown == xBegin) ? xb.fileIndex : xe.fileIndex;
		int xFileBegin = xb.fileIndex;
		int xFileEnd = xe.fileIndex;
		//int yMouseDownFile = (yMouseDown == yBegin) ? yb.fileIndex : ye.fileIndex;
		int yFileBegin = yb.fileIndex;
		int yFileEnd = ye.fileIndex;
		
		int[] selectedIndex;
		if (isEmptyRect) {
			selectedIndex = new int[0];
		} else {
			assert xFileBegin <= xFileEnd;
			assert yFileBegin <= yFileEnd;
			if (xFileBegin <= yFileBegin && yFileBegin <= xFileEnd
					|| yFileBegin <= xFileBegin && xFileBegin <= yFileEnd) {
				// overlapped
				int b = xFileBegin < yFileBegin ? xFileBegin : yFileBegin;
				int e = xFileEnd > yFileEnd ? xFileEnd : yFileEnd;
				selectedIndex = new int[e - b + 1];
				for (int i = 0; i <= e - b; ++i) {
					selectedIndex[i] = b + i;
				}
			} else {
				// not overlapped
				selectedIndex = new int[xFileEnd - xFileBegin + 1 + yFileEnd - yFileBegin + 1];
				int p = 0;
				for (int i = 0; i < xFileEnd - xFileBegin + 1; ++i) {
					selectedIndex[p] = xFileBegin + i;
					++p;
				}
				for (int i = 0; i < yFileEnd - yFileBegin + 1; ++i) {
					selectedIndex[p] = yFileBegin + i;
					++p;
				}
			}
		}

		this.selectedIndex = selectedIndex;
		// this.setSelection(selectedIndex); ’¼Œã‚ÌsetCloneSelectionByFileAndOffset‚ÅÄ•`‰æ‚³‚ê‚é‚Ì‚ÅŒÄ‚Ño‚³‚È‚¢
		mainWindow.setFileSelection(selectedIndex, null); // ’¼Œã‚ÌsetCloneSelectionByFileAndOffset‚ÅÄ•`‰æ‚³‚ê‚é‚Ì‚Ånull‚É‚µ‚Ä‚¨‚­

		mainWindow.setCloneSelectionByFlieAndOffset(
				yFileBegin, yb.offset, ye.offset, 
				xFileBegin, xb.offset, xe.offset);
	}

	private void drawLineMarker(GC gc, int x1, int y1, int x2, int y2) {
		gc.setLineWidth(3);
		if (x2 - x1 < 4 && y2 - y1 < 4) {
			gc.fillRectangle(x1, y1, 4, 4);
		} else {
			double r = Math.sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) / 2.0;
			int w = (int)(r * 2);
			double cx = (x2 + x1) / 2.0;
			double cy = (y2 + y1) / 2.0;
			int x = (int)(cx - r);
			int y = (int)(cy - r);
			gc.drawOval(x, y, w, w);
		}
	}
	
	private void redraw(GC gc) {
		Rectangle canvasRect = canvas.getClientArea();
		Rectangle imageRect = image.getBounds();
		
		gc.setBackground(PlottingColors.getBorderBackground());
		if (imageRect.width < canvasRect.width) {
			gc.fillRectangle(imageRect.width, 0, canvasRect.width - imageRect.width, imageRect.height);
		}
		if (imageRect.height < canvasRect.height) {
			gc.fillRectangle(0, imageRect.height, canvasRect.width, canvasRect.height - imageRect.height);
		}
		
		gc.drawImage(image, 0, 0);
		
		//Color darkCyan = display.getSystemColor(SWT.COLOR_DARK_CYAN);
		Color black = display.getSystemColor(SWT.COLOR_BLACK);
		Color directoryNamePopupBackgroundColor = PlottingColors.getDirectoryNamePopupBackground();
		gc.setForeground(PlottingColors.getSelectedFileColor());
		gc.setBackground(PlottingColors.getSelectedFileColor());
		gc.setLineStyle(SWT.LINE_SOLID);

		// draw selected file area
		if (selectedIndex != null) {
			gc.setLineWidth(3);
			int i = 0;
			ArrayList<Point> list = new ArrayList<Point>();
			while (i < selectedIndex.length) {
				int j = i + 1;
				while (j < selectedIndex.length
						&& selectedIndex[j] == selectedIndex[j - 1] + 1) {
					++j;
				}
				Point region = new Point(selectedIndex[i],
						selectedIndex[j - 1] + 1);
				list.add(region);
				i = j;
			}
			Point[] regions = list.toArray(new Point[] {});
			for (Point yRegion : regions) {
				int y1 = (int) (fileStartPos[yRegion.x] / zoom);
				int y2 = (int) (fileStartPos[yRegion.y] / zoom);
				for (Point xRegion : regions) {
					int x1 = (int) (fileStartPos[xRegion.x] / zoom);
					int x2 = (int) (fileStartPos[xRegion.y] / zoom);
					gc.setAdvanced(true);
					if (gc.getAdvanced()) {
						gc.setAlpha(48);
						gc.fillRectangle(x1, y1, x2 - x1, y2 - y1);
						gc.setAlpha(255);
						gc.setAdvanced(false);
					}
					gc.drawRectangle(x1, y1, x2 - x1, y2 - y1);
				}
			}
		}

		// draw selected clone pairs
		if (selectedClonePairs != null) {
			gc.setForeground(PlottingColors.getSelectedFileColor());
			gc.setBackground(PlottingColors.getSelectedFileColor());
			for (ClonePair p : selectedClonePairs) {
				int x1 = (int) ((fileStartPos[p.rightFile] + p.rightBegin) / zoom);
				int x2 = (int) ((fileStartPos[p.rightFile] + p.rightEnd) / zoom);
				int y1 = (int) ((fileStartPos[p.leftFile] + p.leftBegin) / zoom);
				int y2 = (int) ((fileStartPos[p.leftFile] + p.leftEnd) / zoom);
				drawLineMarker(gc, x1, y1, x2, y2);
			}
		}

		// draw directory box label
		if (showDirectoryBox && directoryBoxPositions != null) {
			ArrayList<DirectoryBoxPosition> directoryNest = new ArrayList<DirectoryBoxPosition>();
			for (int dbpIndex = 0; dbpIndex < directoryBoxPositions.length; ++dbpIndex) {
				DirectoryBoxPosition dbp = directoryBoxPositions[dbpIndex];
				while (directoryNest.size() != 0 && ! dbp.isInsideOrEqualTo(directoryNest.get(directoryNest.size() - 1))) {
					directoryNest.remove(directoryNest.size() - 1);
				}
				final Point extent = gc.stringExtent(dbp.path);
				int boxSize = dbp.endPos - dbp.beginPos;
				boolean boxEnoughLarge = boxSize >= extent.y;
				boolean outsideIsFarEnough = false;
				if (directoryNest.size() == 0) {
					outsideIsFarEnough = true;
				} else {
					DirectoryBoxPosition dbpOuter = directoryNest.get(directoryNest.size() - 1);
					if (dbpOuter.beginPos + extent.y <= dbp.beginPos
							|| dbp.endPos + extent.y <= dbpOuter.endPos) {
						outsideIsFarEnough = true;
					}
				}
				directoryNest.add(dbp);
				if (boxEnoughLarge && outsideIsFarEnough) {
					int x;
					int y;
					final int offset = 2;
					if (directoryLabelShownInLowerRight) {
						x = dbp.beginPos - extent.x;
						if (x < 0) {
							x = 0;
						} else if (x > imageRect.width) {
							x = imageRect.width - extent.x;
						}
						y = dbp.endPos;
						x -= offset;
						y += offset;
					} else {
						x = dbp.endPos;
						y = dbp.beginPos - extent.y;
						if (y < 0) {
							y = 0;
						} else if (y > imageRect.height) {
							y = imageRect.height - extent.y;
						}
						x += offset;
						y -= offset;
					}
					gc.setAdvanced(true);
					if (gc.getAdvanced()) {
						gc.setAntialias(SWT.ON);
						gc.setBackground(directoryNamePopupBackgroundColor);
						gc.setAlpha(208);
						int round = extent.y;
						gc.fillRoundRectangle(x - round / 2, y, extent.x + round, extent.y, round, round);
						gc.setAlpha(255);
					} else {
						gc.drawText(dbp.path, x + 1, y + 1, true);
						gc.drawText(dbp.path, x - 1, y - 1, true);
					}
					gc.setForeground(black);
					gc.drawText(dbp.path, x, y, true);
					if (gc.getAdvanced()) {
						gc.setAntialias(SWT.DEFAULT);
						gc.setAdvanced(false);
					}
				}
			}
		}
		
		// draw dragging area
		{
			gc.setForeground(black);
			gc.setLineWidth(1);
			if (dragStart != null) {
				gc.setLineStyle(SWT.LINE_SOLID);
				gc.drawRectangle(dragStart.x, dragStart.y, dragCurrent.x
						- dragStart.x, dragCurrent.y - dragStart.y);
			} else if (dragRect != null) {
				gc.setLineStyle(SWT.LINE_DOT);
				gc.drawRectangle(dragRect);
			}
		}
	}
	
	private static int countFileSeparator_i(String str, String separator) {
		int count = 0;
		int i = 0;
		while (i < str.length()) {
			int p = str.indexOf(separator, i);
			if (p < 0) {
				return count;
			}
			++count;
			i = p + separator.length();
		}
		return count; // dummy 
	}
	private static int countFileSeparator(String str) {
		return countFileSeparator_i(str, "\\") + countFileSeparator_i(str, "/"); //$NON-NLS-1$ //$NON-NLS-2$
	}
	private static int commonLength(String strA, String strB) {
		int i = 0;
		while (i < strA.length() && i < strB.length()) {
			if (strA.charAt(i) != strB.charAt(i)) {
				break; // while
			}
			++i;
		}
		return i;
	}
	private static String[] splitPath(String str) {
		ArrayList<String> ary = new ArrayList<String>();
		int i = 0;
		while (i < str.length()) {
			int p1 = str.indexOf("\\", i); //$NON-NLS-1$
			int p2 = str.indexOf("/", i); //$NON-NLS-1$
			int p;
			if (p1 < 0) {
				p = p2;
			} else if (p2 < 0) {
				p = p1;
			} else {
				p = p1 < p2 ? p1 : p2;
			}
			if (p < 0) {
				return ary.toArray(new String[0]);
			}
			ary.add(str.substring(i, p));
			i = p + 1;
		}
		return ary.toArray(new String[0]); // dummy
	}

	private void draw_gaps(GC gc, int axisSize, int[] gaps) {
		Color gray = display.getSystemColor(SWT.COLOR_GRAY);
		
		// draw file gaps
		if (showFileGap) {
			gc.setLineWidth(1);
			gc.setForeground(gray);
			gc.setLineStyle(SWT.LINE_DASH);
			for (int index : gaps) {
				long size = fileStartPos[index];
				int pos = (int) (size / zoom);
				gc.drawLine(pos, 0, pos, axisSize);
				gc.drawLine(0, pos, axisSize, pos);
			}
		}
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
	
	private void drawFileRelatedFigures(GC gc, Model data, int axisSize) {
		Color gray = display.getSystemColor(SWT.COLOR_GRAY);
		Color darkGray = display.getSystemColor(SWT.COLOR_DARK_GRAY);
		Color fiteredFileBackground = PlottingColors.getMaskedFileBackground();
		
		final int files = data.getFileCount();
		assert files >= 0;
		{
			gc.setForeground(fiteredFileBackground);
			gc.setBackground(fiteredFileBackground);
			long size = 0;
			for (int i = 0; i < files; ++i) {
				final SourceFile sourceFile = data.getFile(i);
				boolean filtered = isFileFiltered(data, sourceFile.id);
				
				fileStartPos[i] = size;
				int pos = (int) (size / zoom);
				if (filtered) {
					int fileSize = (int)((size + sourceFile.size) / zoom - pos);
					gc.fillRectangle(pos, 0, fileSize, axisSize);
					gc.fillRectangle(0, pos, axisSize, fileSize);
				}
				size += sourceFile.size;
			}
		}
		
		TIntArrayList gaps = new TIntArrayList();
		{
			gc.setForeground(gray);
			gc.setLineWidth(1);
			gc.setLineStyle(SWT.LINE_SOLID);
			gc.drawLine(0, 0, axisSize, axisSize);
			int lastPos = 0;
			long size = 0;
			int lastFileID = -1;
			TIntArrayList pathPartIndices = new TIntArrayList();
			String lastPath = ""; //$NON-NLS-1$
			ArrayList<DirectoryBoxPosition> directoryAry = new ArrayList<DirectoryBoxPosition>();
			
			for (int i = 0; i < files; ++i) {
				final SourceFile sourceFile = data.getFile(i);
				
				fileStartPos[i] = size;
				int pos = (int) (size / zoom);
				boolean isGap = false;
				if (i > 0 && sourceFile.id != lastFileID + 1) {
					gaps.add(i);
					isGap = true;
				}
				
				if (showFileBoundary && (! showFileGap || ! isGap)) {
					// draw file separator
					if (pos > lastPos) {
						gc.setLineWidth(1);
						gc.setForeground(gray);
						gc.drawLine(pos, 0, pos, axisSize);
						gc.drawLine(0, pos, axisSize, pos);
						lastPos = pos;
					}
				}
				
				if (showDirectoryBox) {
					// draw directory box and assign directoryBoxPositions
					gc.setLineWidth(2);
					gc.setForeground(darkGray);
					
					String commonPath = sourceFile.path.substring(0, commonLength(sourceFile.path, lastPath));
					int commonPathDepth = countFileSeparator(commonPath);
					while (commonPathDepth < pathPartIndices.size() - 1) {
						int last = pathPartIndices.size() - 1;
						int beginI = pathPartIndices.get(last);
						int beginPos = (int)(fileStartPos[beginI] / zoom);
						{
							String firstPath = data.getFile(beginI).path;
							String commonPathWithinTheBox = lastPath.substring(0, commonLength(lastPath, firstPath));
							String commonPathWithNeighbors = ""; //$NON-NLS-1$
							if (beginI - 1 > 0) {
								String nPath = data.getFile(beginI - 1).path;
								String ctn = nPath.substring(0, commonLength(nPath, firstPath));
								if (ctn.length() > commonPathWithNeighbors.length()) {
									commonPathWithNeighbors = ctn;
								}
							}
							if (i + 1 < files) {
								String nPath = data.getFile(i + 1).path;
								String ctn = nPath.substring(0, commonLength(nPath, lastPath));
								if (ctn.length() > commonPathWithNeighbors.length()) {
									commonPathWithNeighbors = ctn;
								}
							}
							String pathStringUniqueForTheBox = null;
							{
								String[] pathpartsCommonWithinTheBox = splitPath(commonPathWithinTheBox);
								String[] pathpartsCommonWithNeighbors = splitPath(commonPathWithNeighbors);
								if (pathpartsCommonWithNeighbors.length < pathpartsCommonWithinTheBox.length) {
									pathStringUniqueForTheBox = utility.StringUtil.join(pathpartsCommonWithinTheBox, pathpartsCommonWithNeighbors.length, 
											pathpartsCommonWithinTheBox.length, java.io.File.separator);
								} else {
									pathStringUniqueForTheBox = pathpartsCommonWithinTheBox[pathpartsCommonWithinTheBox.length - 1];
								}
							}
							DirectoryBoxPosition dbp = new DirectoryBoxPosition(pathStringUniqueForTheBox,
									beginPos, pos,
									beginI, i);
							directoryAry.add(dbp);
						}
						gc.drawRectangle(beginPos, beginPos, pos - beginPos, pos - beginPos);
						pathPartIndices.remove(last);
					}
					int pathDepth = countFileSeparator(sourceFile.path);
					while (! (pathDepth < pathPartIndices.size())) {
						pathPartIndices.add(i);
					}
					lastPath = sourceFile.path;
				}
				
				Collections.sort(directoryAry, new DirectoryBoxPositionComparator());
				DirectoryBoxPosition[] dbps = directoryAry.toArray(new DirectoryBoxPosition[0]);
				directoryBoxPositions = dbps;
				
				size += sourceFile.size;
				lastFileID = sourceFile.id;
			}
			
			int pos = (int) (size / zoom);
			if (showFileBoundary) {
				// draw file separator
				gc.setLineWidth(1);
				gc.setForeground(gray);
				gc.drawLine(pos, 0, pos, axisSize);
				gc.drawLine(0, pos, axisSize, pos);
			}
			
			if (showDirectoryBox) {
				// draw directory box
				gc.setLineWidth(2);
				gc.setForeground(darkGray);
				while (pathPartIndices.size() > 0) {
					int last = pathPartIndices.size() - 1;
					int beginI = pathPartIndices.get(last);
					pathPartIndices.remove(last);
					int beginPos = (int)(fileStartPos[beginI] / zoom);
					gc.drawRectangle(beginPos, beginPos, pos - beginPos, pos - beginPos);
				}
			}
		}
		
		draw_gaps(gc, axisSize, gaps.toNativeArray());
	}

	private long[]/* fileStartPos */ calcFileStartPos(Model data) {
		// draw file separators
		int files = data.getFileCount();
		assert files >= 0;
		fileStartPos = new long[files + 1];
		long size = 0;
		for (int i = 0; i < files; ++i) {
			fileStartPos[i] = size;
			SourceFile sourceFile = data.getFile(i);
			size += sourceFile.size;
		}
		fileStartPos[files] = size;
		return fileStartPos;
	}
	
	private void fillBackground(GC gc, int imageRectWidth, int imageRectHeight, int axisSize) {
		Color white = PlottingColors.getCloneAreaBackgorund();
		Color green = PlottingColors.getBorderBackground();
		
		// fill background
		gc.setBackground(white);
		gc.fillRectangle(0, 0, axisSize, axisSize);
		gc.setBackground(green);
		gc.fillRectangle(axisSize, 0, imageRectWidth, axisSize);
		gc.fillRectangle(0, axisSize, imageRectWidth, imageRectHeight);
	}
	
	private int[] calcNocloneArea(Model data, long[] fileStartPos) {
		int files = fileStartPos.length - 1;
		int[] nocloneAreaData = new int[files];
		int upperBoundaryIndex = 0;
		for (int leftFile = 0; leftFile < files; ++leftFile) {
			if (leftFile > upperBoundaryIndex) {
				upperBoundaryIndex = leftFile;
			}
			ClonePair[] pairs = data.getClonePairsOfFile(leftFile, leftFile, files);
			assert pairs != null;
			int i = 0;
			while (i < pairs.length) {
				ClonePair p = pairs[i];
				assert p != null;
				int rightFile = p.rightFile;
				if (rightFile + 1 > upperBoundaryIndex) {
					upperBoundaryIndex = rightFile + 1;
				}
				while (i < pairs.length && pairs[i].rightFile + 1 <= upperBoundaryIndex) {
					++i;
				}
			}
			nocloneAreaData[leftFile] = upperBoundaryIndex;
		}
		return nocloneAreaData;
	}
	
	protected void drawOtherBackground(GC gc, long[] fileStartPos, Model data, long zoom) {
		if (showCloneSpace) {
			int[] nocloneAreaData = calcNocloneArea(data, fileStartPos);
			
			Color yellow = PlottingColors.getNocloneAreaColor();
			
			int files = fileStartPos.length - 1;
			for (int leftFile = 0; leftFile < files; ++leftFile) {
				int upperBoundaryIndex = nocloneAreaData[leftFile];
				if (upperBoundaryIndex < files) {
					int x = (int)(fileStartPos[upperBoundaryIndex] / zoom);
					int xe = (int)(fileStartPos[files] / zoom);
					int y = (int)(fileStartPos[leftFile] / zoom);
					int ye = (int)(fileStartPos[leftFile + 1] / zoom);
					if (xe - x > 0 && ye - y > 0) {
						gc.setBackground(yellow);
						gc.fillRectangle(x, y, xe - x, ye - y);
						gc.fillRectangle(y, x, ye - y, xe - x);
					}
				}
			}
		}
	}

	private void drawClones(GC gc, Rectangle imageRect, int axisSize) {
		if (viewedModel == null) {
			return;
		}
		
		Model data = viewedModel;
		Color black = display.getSystemColor(SWT.COLOR_BLACK);
		
		int cap = gc.getLineCap();
		gc.setLineCap(SWT.CAP_ROUND);
		gc.setAdvanced(true);
		boolean advanced = gc.getAdvanced();
		try {
			if (data.getFileCount() == 0) {
				fillBackground(gc, imageRect.width, imageRect.height, 0);
			}
			else {
				int files = fileStartPos.length - 1;
				
				// draw clones
				gc.setBackground(black);
				gc.setLineWidth(2);
				gc.setLineStyle(SWT.LINE_SOLID);
				
				for (int leftFile = 0; leftFile < files; ++leftFile) {
					long leftSize = fileStartPos[leftFile + 1] - fileStartPos[leftFile];
					long yBase = fileStartPos[leftFile];
					ClonePair[] pairs = data.getClonePairsOfFile(leftFile);
					int i = 0;
					while (i < pairs.length) {
						ClonePair p = pairs[i];
						int rightFile = p.rightFile;
						long rightSize = fileStartPos[rightFile + 1] - fileStartPos[rightFile];
						long xBase = fileStartPos[rightFile];
						int x1 = (int) ((xBase + p.rightBegin) / zoom);
						int y1 = (int) ((yBase + p.leftBegin) / zoom);
						if (leftSize < zoom * 4 && rightSize < zoom * 4) { // if (leftSize / zoom < 4 && rightSize / zoom < 4) {
							if (cloneSetMetricExtractor != null) {
								Color c = PlottingColors.getColor(cloneSetMetricExtractor.getRatio(p.classID));
								gc.setBackground(c);
							} else {
								gc.setBackground(black);
							}
							gc.fillRectangle(x1, y1, 3, 3);
							while (i < pairs.length && pairs[i].rightFile == rightFile) {
								++i;
							}
						} else {
							if (cloneSetMetricExtractor != null) {
								Color c = PlottingColors.getColor(cloneSetMetricExtractor.getRatio(p.classID));
								gc.setForeground(c);
							}
							else {
								gc.setForeground(black);
							}
							int x2 = (int) ((xBase + p.rightEnd) / zoom);
							int y2 = (int) ((yBase + p.leftEnd) / zoom);
							if (advanced) {
								gc.setAntialias(SWT.ON);
							}
							gc.setLineWidth(2);
							gc.drawLine(x1, y1, x2, y2);
							if (advanced) {
								gc.setAntialias(SWT.DEFAULT);
							}
							++i;
						}
					}
				}
			}
		} finally {
			if (advanced) {
				gc.setAdvanced(false);
			}
			gc.setLineCap(cap);
		}
	}

	public void saveImage(String filePath) throws java.io.IOException {
		if (this.image != null) {
			new utility.ImageSaver().saveImage(filePath, image);
		} else {
			Image emptyImage =  new Image(display, 0, 0);
			new utility.ImageSaver().saveImage(filePath, emptyImage);
		}
	}
	
	protected void rebuildImage() {
		if (viewedModel == null) {
			return;
		}
		
		Model data = viewedModel;
		
		assert image != null;
		assert display != null;
		Rectangle imageRect = image.getBounds();
		int drawSize = imageRect.width;
		assert imageRect.width == imageRect.height;
		
		directoryBoxPositions = null;
		
		long[] fileStartPos = calcFileStartPos(data);

		long originalSize = data.totalSizeOfFiles();
		
		// determin zoom and axis size
		zoom = (originalSize + drawSize - 1) / drawSize;
		if (zoom == 0) {
			zoom = 1;
		}
		int axisSize = (int) (originalSize / zoom);
		
		GC gc = new GC(image);
		try {
			fillBackground(gc, imageRect.width, imageRect.height, axisSize);
			drawOtherBackground(gc, fileStartPos, data, zoom);
			drawFileRelatedFigures(gc, data, axisSize); // assign directoryBoxPositions
			drawClones(gc, imageRect, axisSize);
		} finally {
			gc.dispose();
		}
	}
	
	public void updateModel(Model data) {
		viewedModel = data;
		viewedCloneSetMetricModel = null;
		cloneSetMetricExtractor = null;
		
		if (cmColoringMetricName != null) {
			cmColoringMetricName.removeAll();
			cmColoringMetricName.add(strNotApplicable);
			cmColoringMetricName.setText(strNotApplicable);
		}
			
		fileStartPos = null;
		selectedIndex = null;
		selectedClonePairs = null;
		dragRect = null;
		
		rebuildImage();

		assert canvas != null;
		redraw(new GC(canvas));
	}

	public void updateModel(Model data, ClonesetMetricModel metricModelData) {
		final String defaultMetric = "RNR"; //$NON-NLS-1$
		
		viewedModel = data;
		assert viewedCloneSetMetricModel != null;
		viewedCloneSetMetricModel = metricModelData;
		{
			coloringMetric = -1;
			try {
				coloringMetric = model.ClonesetMetricModel.fieldNameToID(defaultMetric);
			} catch (IndexOutOfBoundsException e2) {
				// do mothing
			}
			cloneSetMetricExtractor = CloneSetMetricExtractor.newCloneSetMetricExtractorByID(viewedCloneSetMetricModel, coloringMetric);
		}	
		if (cmColoringMetricName != null) {
			cmColoringMetricName.removeAll();
			cmColoringMetricName.add(strNotApplicable);
			String[] metricNames = ClonesetMetricModel.getFieldNames();
			for (String name : metricNames) {
				cmColoringMetricName.add(name);
			}
			cmColoringMetricName.setText(defaultMetric);
		}
		
		fileStartPos = null;
		selectedIndex = null;
		selectedClonePairs = null;
		dragRect = null;
		
		rebuildImage();

		assert canvas != null;
		redraw(new GC(canvas));
	}
	
	public void setSelection(int[] selectedIndex) {
		this.selectedIndex = selectedIndex.clone();
		
		assert canvas != null;
		redraw(new GC(canvas));
	}

	public void setCloneSelection(long[] cloneSetIDs, CloneSelectionListener src) {
		if (src == this) {
			return;
		}
		
		selectedClonePairs = viewedModel.getClonePairsOfCloneSets(cloneSetIDs);

		assert canvas != null;
		redraw(new GC(canvas));
	}
}
