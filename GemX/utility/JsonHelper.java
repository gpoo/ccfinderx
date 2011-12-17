package utility;

import java.util.HashMap;
import java.util.ArrayList;

import org.eclipse.swt.graphics.RGB;

public class JsonHelper {
	public static RGB readRGBFromSettings(HashMap<String, Object> settings, String key) {
		assert key.length() > 0 && key.charAt(key.length() - 1) == '/';
		
		if (settings.containsKey(key)) {
			try {
				String value = (String)settings.get(key);
				RGB rgb = utility.SystemColorManager.getRGBOfColor(value);
				if (rgb != null) {
					return rgb;
				}
			} catch (java.lang.ClassCastException e) {
			}
		}
		
		boolean failure = false;
		final double[] rgb = new double[3];
		try {
			for (int ci = 0; ci < 3; ++ci) {
				final String rkey = String.format("%s[%d]/", key, ci); //$NON-NLS-1$
				if (settings.containsKey(rkey)) {
					Double rvalue = (Double)settings.get(rkey);
					rgb[ci] = rvalue.doubleValue();
				} else {
					failure = true;
					break; // for ci
				}
			}
		} catch (java.lang.ClassCastException e) {
			failure = true;
		}
		if (! failure) {
			final int[] irgb = new int[3];
			for (int ci = 0; ci < 3; ++ci) {
				int ir = (int)Math.floor(256 * rgb[ci]); if (ir < 0) ir = 0; else if (ir >= 256) ir = 256 - 1;
				irgb[ci] = ir;
			}
			return new RGB(irgb[0], irgb[1], irgb[2]);
		} else {
			return null;
		}
	}

	public static RGB[] readRGBArrayFromSettings(HashMap<String, Object> settings, String key) {
		assert key.length() > 0 && key.charAt(key.length() - 1) == '/';
		
		final ArrayList<RGB> rgbs = new ArrayList<RGB>();
		for (int i = 0; true; ++i) {
			String nkey = String.format("%s[%d]/", key, i); //$NON-NLS-1$
			RGB rgb = readRGBFromSettings(settings, nkey);
			if (rgb == null) {
				break; // for i
			}
			rgbs.add(rgb);
		}
		return (RGB[])rgbs.toArray(new RGB[0]);
	}
}
