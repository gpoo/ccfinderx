package resources;

import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.*;

import java.util.HashMap;

import utility.JsonHelper;

public class MetricColors {
	private static final int reso = 200;
	private static final Color[] colors = new Color[reso];
	private static final Color[] frameColor = new Color[1];
	
	private static final RGB[] defaultColors = new RGB[] { 
			new RGB(100, 100, 200), new RGB(200, 200, 200), new RGB(200, 100, 100) 
	};
	private static final RGB defaultFrameColorValue = new RGB(192, 192, 192);
	
	public static void reloadColor(Display display, HashMap<String, Object> settings) {
		dispose();
		initialize(display, settings);
	}
	
	public static void generateColorsFromRGBSamples(Display display, Color[] colors, RGB[] rgbs) {
		final int countOfColors = rgbs.length;
		for (int i = 0; i < colors.length; ++i) {
			final int w = colors.length / (countOfColors - 1);
			final int ci = i / w;
			assert ci < colors.length - 1;
			final double p = (i % w) / (double)w;
			final double r = rgbs[ci].red * (1.0 - p) + rgbs[ci + 1].red * p;
			final double g = rgbs[ci].green * (1.0 - p) + rgbs[ci + 1].green * p;
			final double b = rgbs[ci].blue * (1.0 - p) + rgbs[ci + 1].blue * p;
			int ir = (int)Math.floor(r); if (ir < 0) ir = 0; else if (ir >= 256) ir = 256 - 1;
			int ig = (int)Math.floor(g); if (ig < 0) ig = 0; else if (ig >= 256) ig = 256 - 1;
			int ib = (int)Math.floor(b); if (ib < 0) ib = 0; else if (ib >= 256) ib = 256 - 1;
			final Color color = new Color(display, ir, ig, ib);
			colors[i] = color;
		}
	}
	
	public static void initialize(Display display, HashMap<String, Object> settings) {
		{
			RGB rgb = JsonHelper.readRGBFromSettings(settings, "/metrics/frame/"); //$NON-NLS-1$
			if (rgb == null) {
				rgb = defaultFrameColorValue;
			}
			frameColor[0] = new Color(display, rgb);
		}
		
		{
			RGB[] rgbs = JsonHelper.readRGBArrayFromSettings(settings, "/metrics/bar_colors/"); //$NON-NLS-1$
			if (! (rgbs != null && rgbs.length >= 2 && rgbs.length <= 10)) {
				rgbs = defaultColors;
			}
			generateColorsFromRGBSamples(display, colors, rgbs);
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
	
	public static Color getFrameColor() {
		return frameColor[0];
	}
	
	public static void dispose() {
		frameColor[0].dispose();
		
		for (int i = 0; i < colors.length; ++i) {
			colors[i].dispose();
		}
	}
}
