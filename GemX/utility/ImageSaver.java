package utility;

import org.eclipse.swt.SWT;
import org.eclipse.swt.SWTException;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.ImageData;
import org.eclipse.swt.graphics.ImageLoader;

public class ImageSaver {
	public void saveImage(String filePath, Image image) throws java.io.IOException {
		assert filePath != null;
		assert image != null;
		ImageLoader loader = new ImageLoader();
		loader.data = new ImageData[] { image.getImageData() };
		try {
			loader.save(filePath, SWT.IMAGE_PNG);
		} catch (SWTException e) {
			switch (e.code) {
			case SWT.ERROR_INVALID_IMAGE:
				assert false;
			case SWT.ERROR_IO:
				throw new java.io.IOException();
			case SWT.ERROR_UNSUPPORTED_FORMAT:
				assert e.code != SWT.ERROR_UNSUPPORTED_FORMAT;
			}
		}
	}
}
