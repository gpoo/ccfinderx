package utility;

import java.io.File;
import java.io.IOException;

import org.eclipse.swt.widgets.Shell;

public class PythonVersionChecker {
	public static String thePythonInterpreterPath = "";
	
	static {
		thePythonInterpreterPath = ccfinderx.CCFinderX.theInstance.getPythonInterpreterPath();
	}
	
	public static boolean check(Shell shell) {
		if (thePythonInterpreterPath == null || thePythonInterpreterPath.length() == 0) {
			return false;
		}
		String exeDir = utility.ExecutionModuleDirectory.get();
		//System.err.println("exeDir=" + exeDir + ".");
		assert exeDir != null;
		String scriptPath = exeDir + File.separator + "scripts" + File.separator + "pythonversionchecker.py"; //$NON-NLS-1$ //$NON-NLS-2$
		Runtime rt = Runtime.getRuntime();
		try {
			//System.err.println("thePythonInterpreterPath=" + thePythonInterpreterPath + ".");
			//System.err.println("scriptPath=" + scriptPath + ".");
			Process pr = rt.exec(new String[] { thePythonInterpreterPath, scriptPath });
			try {
				new utility.Redirector(pr.getInputStream(), System.out).start();
				new utility.Redirector(pr.getErrorStream(), System.err).start();
				pr.waitFor();
				int retCode = pr.exitValue();
				return retCode == 0;
			} catch (InterruptedException e) {
				return false;
			} finally {
				if(pr != null) {
					pr.getErrorStream().close();
					pr.getInputStream().close();
					pr.getOutputStream().close();
					pr.destroy();
				}
			}
		} catch (IOException e) {
			return false;
		}			
	}
}
