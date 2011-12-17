package model;

public class CodeFragment implements Comparable<CodeFragment> {
	public final int file;
	public final int begin;
	public final int end;
	public CodeFragment(int leftFile, int leftBegin, int leftEnd) {
		this.file = leftFile;
		this.begin = leftBegin;
		this.end = leftEnd;
	}
	
	public boolean equals(Object o) {
		if (o instanceof CodeFragment) {
			return ((CodeFragment)o).equals(this);
		}
		return false;
	}
	public int compareTo(CodeFragment right) {
		int d = file - right.file;
		if (d != 0) {
			return d;
		}
		d = begin - right.begin;
		if (d != 0) {
			return d;
		}
		d = end - right.end;
		if (d != 0) {
			return d;
		}
		return 0;
	}
	public boolean equals(CodeFragment right) {
		if (file == right.file && begin == right.begin && end == right.end) {
			return true;
		}
		return false;
	}
	public int hashCode() {
		return file + begin + end;
	}
}
