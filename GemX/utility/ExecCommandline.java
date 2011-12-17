package utility;

import java.io.IOException;
import java.util.ArrayList;

public class ExecCommandline {

	public static int execCommandline(ArrayList<String> args, boolean echoToStderr) throws IOException {
		StringBuilder buf = new StringBuilder();
		buf.append("exec:");
		for (String s : args) {
			buf.append(" ");
			buf.append(s);
		}
		System.err.println(buf.toString());
		
		Runtime rt = Runtime.getRuntime();
		try {
			Process pr = rt.exec(args.toArray(new String[0]));
			try {
				new utility.Redirector(pr.getInputStream(), System.out).start();
				new utility.Redirector(pr.getErrorStream(), System.err).start();
				pr.waitFor();
				int retCode = pr.exitValue();
				if (retCode != 0) {
					throw new IOException();
				}
				return retCode;
			} catch (InterruptedException e) {
				throw new IOException();
			} finally {
				if(pr != null) {
					pr.getErrorStream().close();
					pr.getInputStream().close();
					pr.getOutputStream().close();
					pr.destroy();
				}
			}			
		} catch (IOException e) {
			throw new IOException();
		}
	}

}
