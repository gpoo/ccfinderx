package model;

public class SourceFile {
	public final int id;
	public final String path;
	public final int size; // size in token
	
	public SourceFile(int id, String path, int size) {
		this.id = id;
		this.path = path;
		this.size = size;
	}
	public boolean equals(SourceFile right) {
		if (right.id == this.id && right.path.equals(this.path) && right.size == this.size) {
			return true;
		}
		return false;
	}
	public boolean equals(Object o) {
		if (o instanceof SourceFile) {
			return ((SourceFile)o).equals(this);
		}
		return false;
	}
	public int hashCode() {
		return id + path.hashCode() + size;
	}
}
