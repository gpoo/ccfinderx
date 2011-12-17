package model;

public class CloneSet {
	public final long id;
	public final int length;
	
	public CloneSet(long id, int length) {
		this.id = id;
		this.length = length;
	}
	
	public boolean equals(Object o) {
		if (o instanceof CloneSet) {
			return ((CloneSet)o).equals(this);
		}
		return false;
	}
	public boolean equals(CloneSet right) {
		return id == right.id;
	}
	
	public int hashCode() {
		return (int)id;
	}
}
