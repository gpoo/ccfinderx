package utility;

public class AssertUniq {
	public AssertUniq(int[] ary) {
		if (ary == null || ary.length == 0) {
			return;
		}
		int lastElement = ary[0];
		for (int i = 1; i < ary.length; ++i) {
			if (! (ary[i] > lastElement)) {
				assert false;
			}
			lastElement = ary[i];
		}
	}
}
