package utility;

public class PrepToken {
	public final int beginIndex;
	public final int endIndex;

//debug 
//	public final String str; // this field is for debug purpose

	public final boolean isReservedWord;
	
	public static PrepToken create(int beginRow, int beginColumn, int beginIndex, int endRow, int endColumn, int endIndex, String str) {
		str = str.intern();
		if (str.length() >= 2 && str.charAt(0) == 'r' && str.charAt(1) == '_') {
			return new PrepToken(beginRow, beginColumn, beginIndex, endRow, endColumn, endIndex, str.intern(), true); // calling intern() with expecting that the reserved word will appear many times.
		} else {
			return new PrepToken(beginRow, beginColumn, beginIndex, endRow, endColumn, endIndex, str, false);
		}
	}
	
	private PrepToken(int beginRow, int beginColumn, int beginIndex, int endRow, int endColumn, int endIndex, String str, boolean isReservedWord) {
		this.beginIndex = beginIndex;
		this.endIndex = endIndex;
//debug 
//		this.str = str;
		this.isReservedWord = isReservedWord;
	}
	
	@Override
	public boolean equals(Object o) {
		if (o instanceof PrepToken) {
			return ((PrepToken)o).equals(this);
		}
		return false;
	}
	
	public boolean equals(PrepToken right) {
//debug
//		return this.beginIndex == right.beginIndex
//			&& this.endIndex == right.endIndex
//			&& this.str.equals(right.str);
		
		return this.beginIndex == right.beginIndex
			&& this.endIndex == right.endIndex;
	}
	
	@Override
	public int hashCode() {
//debug
//		return beginIndex * 7 + endIndex + str.hashCode();
		
		return beginIndex * 7 + endIndex;
	}
	
	@Override
	public String toString() {
		StringBuffer buffer = new StringBuffer();
		buffer.append("<"); //$NON-NLS-1$
		buffer.append(beginIndex);
		buffer.append(","); //$NON-NLS-1$
		buffer.append(endIndex);

//debug
//		buffer.append(","); //$NON-NLS-1$
//		buffer.append(str);
		buffer.append(">"); //$NON-NLS-1$

		return buffer.toString();
	}
}
