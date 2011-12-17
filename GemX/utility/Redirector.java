package utility;

import java.io.*;

public class Redirector extends Thread 
{
	private final InputStream is;
	private final PrintStream ps;
	
	public Redirector(InputStream is, PrintStream ps) {
		this.is = is;
		this.ps = ps;
	} 

	public void run() {
		try {
			while(this != null) {
				int ch = is.read();
				if(ch == -1) {
					break;
				}
				ps.print((char)ch);
			}
		} catch (Exception e) {
			e.printStackTrace();
		} 
	}
}
