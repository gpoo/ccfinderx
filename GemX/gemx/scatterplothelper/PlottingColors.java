package gemx.scatterplothelper;

import java.util.HashMap;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Display;

import utility.JsonHelper;

public class PlottingColors {
	private static final Color colors[] = new Color[230];
	private static final Color cloneAreaBackground[] = new Color[1];
	private static final Color borderBackground[] = new Color[1];
	private static final Color nocloneAreaColor[] = new Color[1];
	private static final Color selectedFileColor[] = new Color[1];
	private static final Color rulerBackground[] = new Color[1];
	private static final Color maskedFileBackground[] = new Color[1];
	private static final Color directoryNamePopupBackground[] = new Color[1];
	
	private static final RGB[] defaultColors = new RGB[] { 
			new RGB(115, 115, 230), new RGB(0, 0, 0), new RGB(200, 100, 100) 
	};
	private static final RGB defaultCloneAreaBackground = new RGB(0xff, 0xff, 0xff);
	private static final RGB defaultBorderBackground  = new RGB(49, 103, 69);
	private static final RGB defaultNocloneAreaColor = new RGB(196, 175, 162); // Shironezumi
	private static final RGB defaultSelectedFileColor = new RGB(0x59, 0xb9, 0xc6); // Shinbashi Iro
	private static final RGB defaultRulerBackground = new RGB(0x70, 0x6a, 0x5a); // Negishi Iro
	private static final RGB defaultMaskedFileBackground = new RGB(0xff, 0xe6, 0x53); // tanpopo Iro
	private static final RGB defaultDirectoryNamePopupBackground = new RGB(0xf8, 0xf4, 0xe6); // Zouge Iro
	
	public static Color getCloneAreaBackgorund() {
		return cloneAreaBackground[0];
	}

	public static Color getBorderBackground() {
		return borderBackground[0];
	}

	public static Color getNocloneAreaColor() {
		return nocloneAreaColor[0];
	}
	
	public static Color getSelectedFileColor() {
		return selectedFileColor[0];
	}
	
	public static Color getRulerBackground() {
		return rulerBackground[0];
	}
	
	public static Color getMaskedFileBackground() {
		return maskedFileBackground[0];
	}
	
	public static Color getDirectoryNamePopupBackground() {
		return directoryNamePopupBackground[0];
	}
	
	public static void reloadColor(Display display, HashMap<String, Object> settings) {
		dispose();
		initialize(display, settings);
	}
	
	public static void initialize(Display display, HashMap<String, Object> settings) {
		{
			RGB[] rgbs = JsonHelper.readRGBArrayFromSettings(settings, "/clonescatterplot/bar_colors/"); //$NON-NLS-1$
			if (! (rgbs != null && rgbs.length >= 2 && rgbs.length <= 10)) {
				rgbs = defaultColors;
			}
			resources.MetricColors.generateColorsFromRGBSamples(display, colors, rgbs);
		}
		
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/clonescatterplot/background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultCloneAreaBackground;
			}
			cloneAreaBackground[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/clonescatterplot/border_background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultBorderBackground;
			}
			borderBackground[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/clonescatterplot/ruler_background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultRulerBackground;
			}
			rulerBackground[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/clonescatterplot/noclone_area_background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultNocloneAreaColor;
			}
			nocloneAreaColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/clonescatterplot/selected_file_frame/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultSelectedFileColor;
			}
			selectedFileColor[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/clonescatterplot/masked_file_background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultMaskedFileBackground;
			}
			maskedFileBackground[0] = new Color(display, rgb);
		}
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/clonescatterplot/directory_name_popup_background/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultDirectoryNamePopupBackground;
			}
			directoryNamePopupBackground[0] = new Color(display, rgb);
		}
	}
	
	public static Color getColor(double value) {
		assert 0.0 <= value && value <= 1.0;
		int index = (int)(colors.length * value);
		if (index >= colors.length) {
			index = colors.length - 1;
		}
		else if (index < 0) {
			index = 0;
		}
		return colors[index];
	}
	
	public static void dispose() {
		for (int i = 0; i < colors.length; ++i) {
			colors[i].dispose();
		}
		cloneAreaBackground[0].dispose();
		borderBackground[0].dispose();
		rulerBackground[0].dispose();
		nocloneAreaColor[0].dispose();
		selectedFileColor[0].dispose();
		directoryNamePopupBackground[0].dispose();
	}
}
