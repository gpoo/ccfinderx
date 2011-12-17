package resources;

import java.util.HashMap;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Display;

import utility.JsonHelper;

public class TextColors {
	private static RGB defaultWhite = new RGB(255, 255, 255);
	private static RGB defaultBlack = new RGB(0, 0, 0);
	private static RGB defaultClonePairColor = new RGB(225, 225, 225);
	private static RGB defaultSelectedClonePairColor = new RGB(225, 247, 250);
	private static RGB defaultReservedWordColor = new RGB(120, 0, 0);
	private static RGB defaultNeglectedTextColor = new RGB(0, 160, 0);
	
	private static RGB defaultRulerScrollviewFrameColor = new RGB(255, 255, 0);
	private static RGB defaultRulerScrollviewDraggingFrameColor = new RGB(208, 208, 0);
	private static RGB defaultRulerWhite = new RGB(255, 255, 255);
	private static RGB defaultRulerBackgroundColor = new RGB(0x70, 0x6a, 0x5a); // Negishi Iro
	private static RGB defaultRulerGray = new RGB(192, 192, 192);
	
	private static Color[] white = new Color[1];
	private static Color[] black = new Color[1];
	private static Color[] clonePairColor = new Color[1];
	private static Color[] selectedClonePairColor = new Color[1];
	private static Color[] reservedWordColor = new Color[1];
	private static Color[] neglectedTextColor = new Color[1];
	
	private static Color[] rulerScrollviewFrameColor = new Color[1];
	private static Color[] rulerScrollviewDraggingFrameColor = new Color[1];
	private static Color[] rulerWhite = new Color[1];
	private static Color[] rulerBackgroundColor = new Color[1];
	private static Color[] rulerGray = new Color[1];
	
	public static Color getWhite() {
		return white[0];
	}
	
	public static Color getBlack() {
		return black[0];
	}
	
	public static Color getClonePair() {
		return clonePairColor[0];
	}
	
	public static Color getSelectedClonePair() {
		return selectedClonePairColor[0];
	}
	
	public static Color getReservedWord() {
		return reservedWordColor[0];
	}
	
	public static Color getNeglectedText() {
		return neglectedTextColor[0];
	}
	
	public static Color getRulerScrollviewFrame() {
		return rulerScrollviewFrameColor[0];
	}
	
	public static Color getRulerScrollviewDraggingFrame() {
		return rulerScrollviewDraggingFrameColor[0];
	}
	
	public static Color getRulerWhite() {
		return rulerWhite[0];
	}
	
	public static Color getRulerBackground() {
		return rulerBackgroundColor[0];
	}
	
	public static Color getRulerGray() {
		return rulerGray[0];
	}

	public static void reloadColor(Display display, HashMap<String, Object> settings) {
		dispose();
		initialize(display, settings);
	}
	
	public static void initialize(Display display, HashMap<String, Object> settings) {
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext/background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultWhite;
			}
			white[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext/foreground/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultBlack;
			}
			black[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext/clonepair_background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultClonePairColor;
			}
			clonePairColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext/selected_clonepair_background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultSelectedClonePairColor;
			}
			selectedClonePairColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext/reservedword_foreground/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultReservedWordColor;
			}
			reservedWordColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext/neglectedtext_foreground/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultNeglectedTextColor;
			}
			neglectedTextColor[0] = new Color(display, rgb);
		}

		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext_ruler/background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultRulerBackgroundColor;
			}
			rulerBackgroundColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext_ruler/scrollview_frame/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultRulerScrollviewFrameColor;
			}
			rulerScrollviewFrameColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext_ruler/scrollview_dragging_frame/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultRulerScrollviewDraggingFrameColor;
			}
			rulerScrollviewDraggingFrameColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext_ruler/filebody_line/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultRulerWhite;
			}
			rulerWhite[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/sourcetext_ruler/clone_line/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultRulerGray;
			}
			rulerGray[0] = new Color(display, rgb);
		}
	}
	
	public static void dispose() {
		white[0].dispose();
		black[0].dispose();
		clonePairColor[0].dispose();
		selectedClonePairColor[0].dispose();
		reservedWordColor[0].dispose();
		neglectedTextColor[0].dispose();
		
		rulerScrollviewFrameColor[0].dispose();
		rulerWhite[0].dispose();
		rulerBackgroundColor[0].dispose();
		rulerGray[0].dispose();
	}
}


