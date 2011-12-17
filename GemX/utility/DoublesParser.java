package utility;

public class DoublesParser {
	public static Double[] parseDoublesNullable(String[] numStrs, int start, int end) {
		assert start >= 0;
		assert end <= numStrs.length;
		Double[] vs = new Double[end - start];
		int j = 0;
		for (int i = start; i < end; ++i) {
			String s = numStrs[i];
			if (s.equals("-")) {
				vs[j] = null;
			} else {
				double v = Double.parseDouble(s);
				vs[j] = v;
			}
			++j;
		}
		return vs;
	}
	public static double[] parseDoubles(String[] numStrs, int start, int end) {
		assert start >= 0;
		assert end <= numStrs.length;
		double[] vs = new double[end - start];
		int j = 0;
		for (int i = start; i < end; ++i) {
			String s = numStrs[i];
			double v = Double.parseDouble(s);
			vs[j] = v;
			++j;
		}
		return vs;
	}
}
