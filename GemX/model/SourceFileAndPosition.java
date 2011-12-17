package model;

public class SourceFileAndPosition extends SourceFile {
	public final long beginPosition;
	public final long endPosition;
	
	public SourceFileAndPosition(int id, String path, int size, long beginPosition, long endPosition) {
		super(id, path, size);
		this.beginPosition = beginPosition;
		this.endPosition = endPosition;
	}
}
