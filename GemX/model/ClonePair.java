package model;

public class ClonePair implements Comparable<ClonePair> {
	public final int leftFile;
	public final int leftBegin;
	public final int leftEnd;
	public final int rightFile;
	public final int rightBegin;
	public final int rightEnd;
	public final long classID;
	
	public ClonePair(int leftFile, int leftBegin, int leftEnd, int rightFile, int rightBegin, int rightEnd, long classID) {
		this.leftFile = leftFile;
		this.leftBegin = leftBegin;
		this.leftEnd = leftEnd;
		this.rightFile = rightFile;
		this.rightBegin = rightBegin;
		this.rightEnd = rightEnd;
		this.classID = classID;
	}
	
	public boolean equals(Object o) {
		if (o instanceof ClonePair) {
			return ((ClonePair)o).equals(this);
		}
		return false;
	}
	public int compareTo(ClonePair right) {
		int d = leftFile - right.leftFile;
		if (d != 0) {
			return d;
		}
		d = rightFile - right.rightFile;
		if (d != 0) {
			return d;
		}
		d = leftBegin - right.leftBegin;
		if (d != 0) {
			return d;
		}
		d = leftEnd - right.leftEnd;
		if (d != 0) {
			return d;
		}
		d = rightBegin - right.rightBegin;
		if (d != 0) {
			return d;
		}
		d = rightEnd - right.rightEnd;
		if (d != 0) {
			return d;
		}
		if (classID < right.classID) {
			return -1;
		} else if (classID > right.classID) {
			return 1;
		}
		return 0;
	}
	public boolean equals(ClonePair right) {
		if (leftFile == right.leftFile && leftBegin == right.leftBegin && leftEnd == right.leftEnd) {
			if (rightFile == right.rightFile && rightBegin == right.rightBegin && rightEnd == right.rightEnd) {
				if (classID == right.classID) {
					return true;
				}
			}
		}
		return false;
	}
	public int hashCode() {
		return leftFile + leftBegin + leftEnd + rightFile + rightBegin + rightEnd + (int)classID;
	}
	public ClonePair switchCodeLeftRight() {
		return new ClonePair(this.rightFile, this.rightBegin, this.rightEnd, 
				this.leftFile, this.leftBegin, this.leftEnd, 
				this.classID);
	}
	public CodeFragment getLeftCodeFragment() {
		return new CodeFragment(leftFile, leftBegin, leftEnd);
	}
	public CodeFragment getRightCodeFragment() {
		return new CodeFragment(rightFile, rightBegin, rightEnd);
	}
}
