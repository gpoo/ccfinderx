package utility;

import java.io.File;
import java.util.Calendar;

public class TemporaryFileManager {
	private static String baseString;
	private static int count = 0;
	private static String tempDirPath;
	
	static {
		Calendar cal = Calendar.getInstance();
		int yr = cal.get(Calendar.YEAR);
		int mo = cal.get(Calendar.MONTH)+1; 
		int dy = cal.get(Calendar.DAY_OF_MONTH);
		int hr = cal.get(Calendar.HOUR_OF_DAY);
		int mn = cal.get(Calendar.MINUTE);
		int se = cal.get(Calendar.SECOND);
		int msec = cal.get(Calendar.MILLISECOND);
		baseString = String.format("%04d%02d%02d-%02d%02d%02d-%03d", yr, mo, dy, hr, mn, se, msec); //$NON-NLS-1$
		try {
			tempDirPath = System.getenv("CCFINDERX_TEMPORARY_DIRECTORY"); // may returns null
		} catch (SecurityException e) {
			System.err.println("warning: fail to refer an environmen variable");
		}
	}
	
	public static String getFileNameOnTemporaryDirectory(String fileNameWithoutDirectory) {
		String path = "";
		if (tempDirPath != null) {
			char lastChar = tempDirPath.charAt(tempDirPath.length() - 1);
			path = tempDirPath;
			if (lastChar != File.separatorChar) {
				path += File.separatorChar;
			}
		}
		return path + fileNameWithoutDirectory;
	}
	
	public static String createTemporaryFileName() {
		return createTemporaryFileName("gemxtemp", ".tmp"); //$NON-NLS-1$ //$NON-NLS-2$
	}
	
	public static String createTemporaryFileName(String prefix, String extension) {
		assert prefix.indexOf(File.separatorChar) < 0;
		
		String fname;
		synchronized(TemporaryFileManager.class) {
			++count;
			fname = String.format("%s-%s-%d%s", prefix, baseString, new Integer(count), extension);
		}
		String path = "";
		if (tempDirPath != null) {
			char lastChar = tempDirPath.charAt(tempDirPath.length() - 1);
			path = tempDirPath;
			if (lastChar != File.separatorChar) {
				path += File.separatorChar;
			}
		}
		return path + fname;
	}
	
	public static void registerFileToBeRemoved(String fileName) {
		File f = new File(fileName);
		f.deleteOnExit();
	}
	
	public static void dispose() {
	}
}
