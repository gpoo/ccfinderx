package constants;

public class CcfxDefaultSettings {
	public static final int initMinimumCloneLength = 50;
	public static final int initMinimumTKS = 12;
	public static final String initPreprocessor = ""; //$NON-NLS-1$
	public static final int initChunkSize = 60;
	public static final String initEncoding = "char"; //$NON-NLS-1$
	public static final int initShaperLevel = 2; // 0 -> don't use, 1 -> easy shaper, 2 -> soft shaper, 3 -> hard shaper.
	public static final boolean initUsePMatch = true;
	public static final boolean initUsePreprocessCache = true;
	public static final int initMaxWorkerThreads = 0; // 0 -> do not use worker threads
	public static final boolean initUsePrescreening = false;
//	static {
//		if (System.getProperty("os.name").equals("Linux")) {
//			initUsePreprocessCache = false; // preprocess packing is currently not working in Ubuntu i386
//		} else {
//			initUsePreprocessCache = true;
//		}
//	}
}
