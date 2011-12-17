package utility;

import java.util.Arrays;

public class SetOperators {
	public static gnu.trove.TLongHashSet getUnion(gnu.trove.TLongHashSet a, gnu.trove.TLongHashSet b) {
		gnu.trove.TLongHashSet left, right;
		if (a.size() < b.size()) {
			left = a;
			right = b;
		} else {
			left = b;
			right = a;
		}
		gnu.trove.TLongHashSet result = new gnu.trove.TLongHashSet();
		for (gnu.trove.TLongIterator i = left.iterator(); i.hasNext(); ) {
			long value = i.next();
			if (right.contains(value)) {
				result.add(value);
			}
		}
		return result;
	}
	public static int[] getUnion(int[] a, int[] b) {
		// the arguments a and b should be sorted.
		int[] left, right;
		if (a.length > b.length) {
			left = a;
			right = b;
		} else {
			left = b;
			right = a;
		}
		gnu.trove.TIntHashSet result = new gnu.trove.TIntHashSet();
		for (int i = 0; i < left.length; ++i) {
			int value = left[i];
			int j = Arrays.binarySearch(right, value);
			if (j >= 0) {
				result.add(value);
			}
		}
		int[] r = result.toArray();
		Arrays.sort(r);
		return r;
	}
}
