package ccfinderx;

public class CCFinderX {
	public native void setModuleDirectory(String path);
	public native int invokeCCFinderX(String[] args);
	public native int[] getVersion();
	public native void openOfficialSiteTop(String pageSuffix);
	public native void openOfficialSiteUserRgistrationPage(String pageSuffix);
	public native void openOfficialSiteDocumentPage(String pageSuffix, String pageFileName);
	public native byte[] openPrepFile(String fileName, String suffix);
	public native void clearPrepFileCacheState();
	public native String getPythonInterpreterPath();
	public native String getApplicationDataPath();
	public native int getCurrentProcessId();
	public native boolean isProcessAlive(int processId);
	
	static {
		System.loadLibrary("_CCFinderXLib"); //$NON-NLS-1$
	}
	
	public static CCFinderX theInstance = new CCFinderX();
}
