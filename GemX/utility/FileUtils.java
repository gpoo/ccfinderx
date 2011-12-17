package utility;

import java.io.*;
import java.util.ArrayList;

public class FileUtils {
	public static void fileCopy(String fromFile, String toFile) throws FileNotFoundException, IOException {
		final int BUFSIZE = 4096;

		FileInputStream in = new FileInputStream(fromFile);
		FileOutputStream out = new FileOutputStream(toFile);
		byte buff[] = new byte[BUFSIZE];
		int len;

		while ((len = in.read(buff, 0, BUFSIZE)) != -1) {
			out.write(buff, 0, len);
		}
	}
	
	public static ArrayList<String> readLines(String fileName) throws IOException {
		ArrayList<String> lines = new ArrayList<String>();
		BufferedReader reader = new BufferedReader(new FileReader(fileName));
		String line;
		while ((line = reader.readLine()) != null) {
			lines.add(line);
		}
		reader.close();
		return lines;
	}
}
