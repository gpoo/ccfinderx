package gemx;

import model.ClonePair;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import resources.TextColors;

import java.util.*;

import utility.PrepToken;

public class TextRuler implements ITextRuler {
	//private MainWindow mainWindow;
	private Shell shell;
	private Display display;
	private MultipleTextPane pane;
	private Canvas canvas;
	private Image image;
	private boolean visible;
	private static final int marginHeight = 5;
	private static final int[] dashPattern = new int[] { 4, 4 };
	private ArrayList<Rectangle> frameRectangles = new ArrayList<Rectangle>();
	private int draggedFrameIndex = -1;
	private int focusedTextPaneIndex = -1;
	
	public void changeFocusedTextPane(int index) {
		this.focusedTextPaneIndex = index;
		GC gc = new GC(canvas);
		try {
			if (image != null) {
				gc.drawImage(image, 0, 0);
			}
			redrawFocus(gc);
		} finally {
			gc.dispose();
		}
	}
	
	public int getWidth() {
		if (canvas == null) {
			return 1;
		}
		return canvas.getBounds().width;
	}
	
	public TextRuler(Composite parent, MainWindow mainWindow) {
		//this.mainWindow = mainWindow;
		this.display = parent.getDisplay();
		this.visible = true;
		this.shell = parent.getShell();
		this.draggedFrameIndex = -1;
		
		canvas = new Canvas(parent, SWT.NONE);
		canvas.addPaintListener(new PaintListener() {
			public void paintControl(PaintEvent e) {
				if (visible) {
					//buildImage();
					if (image != null) {
						e.gc.drawImage(image, 0, 0);
					}
					redrawFocus(e.gc);
				}
			}
		});
		canvas.addListener(SWT.Resize, new Listener() {
			public void handleEvent(Event event) {
				if (visible) {
					buildImage();
					GC gc = new GC(canvas);
					try {
						if (image != null) {
							gc.drawImage(image, 0, 0);
						}
						redrawFocus(gc);
					} finally {
						gc.dispose();
					}
				}
			}
		});
		canvas.addMouseListener(new MouseAdapter() {
			public void mouseDown(MouseEvent e) {
				boolean rightClick = e.button == 3;
				if (rightClick) {
					return; // pop-up menu
				} else if (e.button == 1) {
					canvas.forceFocus();
					if ((e.stateMask & SWT.ALT) != 0) {
						// with alt-key
					} else {
						if (frameRectangles.size() > 0) {
							int minDx = -1;
							int minDxFrameIndex = -1;
							for (int i = 0; i < frameRectangles.size(); ++i) {
								final Rectangle rect = frameRectangles.get(i);
								int dx = rect.x + rect.width - e.x;
								if (dx < 0) { dx = -dx; }
								if (minDx == -1 || dx < minDx) {
									minDx = dx;
									minDxFrameIndex = i;
								}
							}
							final Rectangle rect = frameRectangles.get(minDxFrameIndex);
							final int spotSize = (int)(rect.width * 0.3);
							if (rect.y <= e.y && e.y < rect.y + rect.height + spotSize) {
								if (rect.x + rect.width * 0.75 <= e.x && e.x < rect.x + rect.width * 0.85 + spotSize) {
									draggedFrameIndex = minDxFrameIndex;
									final Display display = shell.getDisplay();
									shell.setCursor(display.getSystemCursor(SWT.CURSOR_HAND));
									GC gc = new GC(canvas);
									try {
										if (image != null) {
											gc.drawImage(image, 0, 0);
										}
										TextRuler.this.redrawFocus(gc);
									} finally {
										gc.dispose();
									}
								}
							}
						}
					}
				}
			}

			public void mouseUp(MouseEvent e) {
				if (draggedFrameIndex != -1) {
					shell.setCursor(display.getSystemCursor(SWT.CURSOR_ARROW));
				}
				if (e.button == 1) {
					if (draggedFrameIndex != -1) {
						final int frameIndex = draggedFrameIndex;
						draggedFrameIndex = -1;
						Rectangle clientRect = canvas.getClientArea();
						if (clientRect.height != 0) {
							if (0 <= frameIndex && frameIndex < frameRectangles.size()) {
								TextPane.BeginEnd be = pane.getVisibleTokenRangeOfPane(frameIndex);
								if (be != null) {
									final int maxFileLength = calcMaxFileLength();
									int pos = (e.y - marginHeight) * maxFileLength / clientRect.height;
									pane.setVisibleTokenCenterIndexOfPane(frameIndex, pos);
								}
							}
						}
					}
				}
			}
		});
	}
	
	/* (non-Javadoc)
	 * @see gemx.ITextRuler#isVisible()
	 */
	public boolean isVisible() {
		return visible;
	}
	
	/* (non-Javadoc)
	 * @see gemx.ITextRuler#setVisible(boolean)
	 */
	public void setVisible(boolean visible) {
		if (! this.visible) {
			this.visible = visible;
			update();
		}
	}

	/* (non-Javadoc)
	 * @see gemx.ITextRuler#setTextPane(gemx.MultipleSourceTextPane)
	 */
	public void setTextPane(MultipleTextPane pane) {
		this.pane = pane;
	}
	
	/* (non-Javadoc)
	 * @see gemx.ITextRuler#textScrolled()
	 */
	public void textScrolled() {
		this.updateViewLocationDisplay();
	}
	
	private int calcMaxFileLength() {
		int[] files = pane.getViewedFiles();
		if (files == null) {
			return 1;
		}
		
		int maxFileLength = 0;
		for (int i = 0; i < files.length; ++i) {
			PrepToken[] tokens = pane.getTokens(files[i]);
			if (tokens != null && tokens.length > maxFileLength) {
				maxFileLength = tokens.length;
			}
		}
		if (maxFileLength == 0) {
			maxFileLength = 1;
		}
		
		return maxFileLength;
	}
	
	private void redrawFocus(GC gc) {
		Rectangle clientRect = canvas.getClientArea();
		if (clientRect.width == 0 || clientRect.height == 0) {
			return;
		}
		
		final Color yellow = TextColors.getRulerScrollviewFrame();
		final Color draggedColor = TextColors.getRulerScrollviewDraggingFrame();
		
		final int[] files = pane.getViewedFiles();
		if (files == null || files.length == 0) {
			return;
		}
		
		while (frameRectangles.size() > 0) {
			frameRectangles.remove(frameRectangles.size() - 1);
		}
		
		final int maxFileLength = calcMaxFileLength();
		
		boolean inAdvance = false;
		gc.setAdvanced(true);
		if (gc.getAdvanced()) {
			inAdvance = true;
			gc.setAntialias(SWT.ON);
		}
		gc.setLineStyle(SWT.LINE_SOLID);
		for (int i = 0; i < files.length; ++i) {
			TextPane.BeginEnd be = pane.getVisibleTokenRangeOfPane(i);
			if (be != null) {
				if (inAdvance) {
					gc.setAlpha(128);
				}
				final int lineThick = 2;
				
				Rectangle fileRect = new Rectangle(
						clientRect.x + clientRect.width * i / files.length, clientRect.y + marginHeight,
						clientRect.width / files.length, clientRect.height - marginHeight);
				Rectangle frameRect = new Rectangle((int)(fileRect.x + fileRect.width * 0.05), fileRect.y + be.begin * fileRect.height / maxFileLength,
						(int)(fileRect.width * 0.90), (be.end - be.begin) * fileRect.height / maxFileLength);
				frameRectangles.add(frameRect);
				
				if (frameRect.height <= lineThick * 2) {
					gc.setBackground(i == draggedFrameIndex ? draggedColor : yellow);
					gc.fillRectangle(frameRect.x, frameRect.y, 
							frameRect.width >= lineThick ? frameRect.width : lineThick, 
							frameRect.height >= lineThick ? frameRect.height : lineThick);
				} else {
					final int roundWidth = (int)(frameRect.width * 0.2);
					gc.setLineWidth(lineThick);
					gc.setForeground(i == draggedFrameIndex ? draggedColor : yellow);
					gc.drawRoundRectangle(frameRect.x, frameRect.y, frameRect.width, frameRect.height, roundWidth, roundWidth);
					if (i == this.focusedTextPaneIndex) {
						Rectangle r = new Rectangle((int)(frameRect.x + frameRect.width * 0.88), frameRect.y, 
								(int)(frameRect.width * 0.13), frameRect.height);
						gc.setBackground(i == draggedFrameIndex ? draggedColor : yellow);
						gc.fillRoundRectangle(r.x, r.y, r.width, r.height, roundWidth, roundWidth);
					}
				}
				if (inAdvance) {
					gc.setAlpha(255);
				}
			}
		}
		if (gc.getAdvanced()) {
			gc.setAntialias(SWT.DEFAULT);
		}
		gc.setAdvanced(false);
	}
	
	private static class LineDrawer {
		private GC gc;
		private boolean inAdvance;
		private int maxFileLength;
		private Color gray;
		private Color backgroundColor;
		
		public LineDrawer(GC gc_, boolean inAdvance_, Color gray_, Color backgroundColor_, int maxFileLength_) {
			this.gc = gc_;
			this.inAdvance = inAdvance_;
			this.gray = gray_;
			this.backgroundColor = backgroundColor_;
			this.maxFileLength = maxFileLength_;
		}
	
		public void drawPairCodeFragment(int x0, ClonePair pair, Rectangle fileRect) {
			final int width = (int)(fileRect.width * 0.06) + 1;
			if (inAdvance) {
				int y0 = fileRect.y + fileRect.height * pair.leftBegin / maxFileLength;
				int y1 = fileRect.y + fileRect.height * pair.leftEnd / maxFileLength;
				if (y1 - y0 > width) {
					gc.setLineWidth(width);
					gc.setForeground(gray);
					gc.setLineCap(SWT.CAP_ROUND);
					gc.drawLine(x0, y0 + width/2, x0, y1 - width/2);
					gc.setLineCap(SWT.CAP_FLAT);
				} else {
					gc.setBackground(gray);
					gc.fillOval(x0 - width/2, y0, width, y1 - y0);
				}
			} else {
				gc.setLineWidth(width);
				gc.setForeground(gray);
				gc.drawLine(x0, fileRect.y + fileRect.height * pair.leftBegin / maxFileLength,
						x0, fileRect.y + fileRect.height * pair.leftEnd / maxFileLength);
			}
		}
		
		public void drawBridgeInFile(int x0, ClonePair pair, Rectangle fileRect) {
			gc.setLineWidth(inAdvance ? 1 : 2);
			gc.setForeground(gray);
			int leftCenter = (pair.leftBegin + pair.leftEnd) / 2;
			int rightCenter = (pair.rightBegin + pair.rightEnd) / 2;
			int width = (int)(fileRect.width * (0.05 + (0.4 * (rightCenter - leftCenter) / maxFileLength)) + 1);
			gc.drawArc(
					x0 - width, fileRect.y + fileRect.height * leftCenter / maxFileLength,
					width * 2, fileRect.height * (rightCenter - leftCenter) / maxFileLength,
					270, 180);
		}
		
		public void drawBridgeBetweenFiles(int leftX, int rightX, ClonePair pair, 
				Rectangle leftFileRect, Rectangle rightFileRect) {
			gc.setLineWidth(inAdvance ? 1 : 2);
			Point left = new Point(leftX, leftFileRect.y + leftFileRect.height * (pair.leftBegin + pair.leftEnd) / 2 / maxFileLength);
			Point right = new Point(rightX, rightFileRect.y + rightFileRect.height * (pair.rightBegin + pair.rightEnd) / 2 / maxFileLength);
			gc.setForeground(backgroundColor);
			gc.drawLine(left.x + 1, left.y + 1, right.x + 1, right.y + 1);
			gc.setForeground(gray);
			gc.drawLine(left.x, left.y, right.x, right.y);
		}
	}
	
	private void buildImage() {
		final Rectangle clientRect = canvas.getClientArea();
		if (clientRect.width == 0 || clientRect.height == 0) {
			return;
		}
		Rectangle imageRect;
		if (image != null && ((imageRect = image.getBounds()).width < clientRect.width || imageRect.height < clientRect.height)) {
			image.dispose();
			image = null;
		}
		if (image == null) {
			image = new Image(display, clientRect.width, clientRect.height);
		}
		final int[] files = pane.getViewedFiles();
		
		GC gc = new GC(image);
		try {
			boolean inAdvance = false;
			gc.setAdvanced(true);
			if (gc.getAdvanced()) {
				inAdvance = true;
				gc.setAntialias(SWT.ON);
			}
			Color white = TextColors.getRulerWhite();
			Color backgroundColor = TextColors.getRulerBackground();
			Color gray = TextColors.getRulerGray();
			{
				final int marginHeight = 5;
				
				// fill background
				gc.setBackground(backgroundColor);
				gc.fillRectangle(0, 0, clientRect.width, clientRect.height);
				
				if (files == null || files.length == 0) {
					return;
				}
				
				//// make mapping to index of files[] to file index (of File Table)
				//gnu.trove.TIntIntHashMap fileOrder = new gnu.trove.TIntIntHashMap();
				//for (int i = 0; i < files.length; ++i) {
				//	fileOrder.put(files[i], i);
				//}
				
				// get max length of the files
				int maxFileLength = 0;
				for (int i = 0; i < files.length; ++i) {
					PrepToken[] tokens = pane.getTokens(files[i]);
					if (tokens != null && tokens.length > maxFileLength) {
						maxFileLength = tokens.length;
					}
				}
				maxFileLength += 1;
				
				for (int i = 0; i < files.length; ++i) {
					PrepToken[] tokens = pane.getTokens(files[i]);
					if (tokens != null) {
						int fileLength = tokens.length;
						
						// draw file body
						Rectangle fileRect = new Rectangle(clientRect.x + clientRect.width * i / files.length, clientRect.y + marginHeight,
								clientRect.width / files.length, clientRect.height - marginHeight);
						gc.setForeground(white);
						gc.setLineWidth((int)(fileRect.width * 0.03) + 1);
						int x = (int)(fileRect.x + fileRect.width * 0.31);
						gc.drawLine(x, fileRect.y, x, fileRect.height * (fileLength + 1) / maxFileLength);
					}
				}
					
				final LineDrawer ld = new LineDrawer(gc, inAdvance, gray, backgroundColor, maxFileLength);
				for (int leftI = 0; leftI < files.length; ++leftI) {
					Rectangle fileRect = new Rectangle(clientRect.x + clientRect.width * leftI / files.length, clientRect.y + marginHeight,
							clientRect.width / files.length, clientRect.height - marginHeight);
					
					int leftFile = files[leftI];
					ClonePair[] pairs = pane.getClonePairs(leftFile);
					if (pairs == null) {
						continue; // for leftI
					}
					
					gc.setLineStyle(SWT.LINE_SOLID);
					for (int j = 0; j < pairs.length; ++j) {
						ClonePair pair = pairs[j];
						assert pair.leftFile == leftFile;
						if (pair.rightFile == leftFile) {
							// draw a inner-file clone pair
							int x = (int)(fileRect.x + fileRect.width * 0.415);
							ld.drawPairCodeFragment(x, pair, fileRect);
							
							// draw a bridge between code fragments of the clone pair
							if (pair.leftBegin < pair.rightBegin) {
								ld.drawBridgeInFile(x, pair, fileRect);
							}
						} else {
							// draw a cross-file clone pair
							int x = (int)(fileRect.x + fileRect.width * 0.205);
							ld.drawPairCodeFragment(x, pair, fileRect);
						}
					}
					
					gnu.trove.TLongIntHashMap doneCloneSetIDs = new gnu.trove.TLongIntHashMap();
					for (int rightI = leftI + 1; rightI < files.length; ++rightI) {
						Rectangle rightFileRect = new Rectangle(
								clientRect.x + clientRect.width * rightI / files.length, clientRect.y + marginHeight,
								clientRect.width / files.length, clientRect.height - marginHeight);
						int x = (int)(fileRect.x + fileRect.width * 0.205);
						int xInner = (int)(fileRect.x + fileRect.width * 0.415);
						for (int j = 0; j < pairs.length; ++j) {
							ClonePair pair = pairs[j];
							int rightFile = files[rightI];
							if (pair.rightFile == rightFile) {
								boolean shallDraw = true;
								{
									int distance = doneCloneSetIDs.get(pair.classID);
									if (distance != 0 && distance < rightI - leftI) {
										shallDraw = false;
									}
								}
								if (shallDraw) {
									int distance = rightI - leftI;
									doneCloneSetIDs.put(pair.classID, distance);
									
									// draw a bridge between code fragments of the clone pair
									if (distance >= 2) {
										if (inAdvance) {
											gc.setLineDash(dashPattern);
										} else {
											gc.setLineStyle(SWT.LINE_DASH);
										}
									} else {
										gc.setLineStyle(SWT.LINE_SOLID);
									}
									int rightX = (int)(rightFileRect.x + rightFileRect.width * 0.205);
									int rightXInner = (int)(rightFileRect.x + rightFileRect.width * 0.415);
									if (pair.leftFile != pair.rightFile) {
										ld.drawBridgeBetweenFiles(x, rightX, pair, fileRect, rightFileRect);
									} else {
										ld.drawBridgeBetweenFiles(xInner, rightXInner, pair, fileRect, rightFileRect);
									}
									if (distance >= 2) {
										if (inAdvance) {
											gc.setLineDash(null);
										}
									}
								}
							}
						}
					}
				}
			} 
			if (gc.getAdvanced()) {
				gc.setAntialias(SWT.DEFAULT);
			}
		} finally {
			gc.dispose();
		}
	}

	public void updateViewLocationDisplay() {
		focusedTextPaneIndex = -1;
		if (draggedFrameIndex != -1) {
			shell.setCursor(display.getSystemCursor(SWT.CURSOR_ARROW));
		}
		this.draggedFrameIndex = -1;
		if (visible) {
			GC gc = new GC(this.canvas);
			try {
				if (image != null) {
					gc.drawImage(image, 0, 0);
				}
				this.redrawFocus(gc);
			} finally {
				gc.dispose();
			}
		}
	}
	
	/* (non-Javadoc)
	 * @see gemx.ITextRuler#update()
	 */
	public void update() {
		focusedTextPaneIndex = -1;
		if (draggedFrameIndex != -1) {
			shell.setCursor(display.getSystemCursor(SWT.CURSOR_ARROW));
		}
		this.draggedFrameIndex = -1;
		if (visible) {
			GC gc = new GC(this.canvas);
			try {
				buildImage();
				if (image != null) {
					gc.drawImage(image, 0, 0);
				}
				this.redrawFocus(gc);
			} finally {
				gc.dispose();
			}
		}
	}
}
