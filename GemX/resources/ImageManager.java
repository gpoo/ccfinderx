package resources;

import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.*;

public class ImageManager {
	private static java.util.HashMap<String, Image> table = new java.util.HashMap<String, Image>();
	
	public static Image loadImage(Display display, String filename) {
		if (table.containsKey(filename)) {
			return table.get(filename);
		}
		try {
			Image image = new Image(display, "res/icons/" + filename);
			table.put(filename, image);
			return image;
		} catch (Exception e) {
			return null;
		}
	}
}
