package utility;

public class StringChecker {
	public static boolean is_ascii_nocontrol(String str) {
		final int len = str.length();
		for (int i = 0; i < len; ) {
			final int ch = str.codePointAt(i);
			if (! (0x20 <= ch && ch <= 0x7f)) {
				return false;
			}
			i += Character.charCount(ch);
		}
		return true;
	}
	public static boolean is_utf8_nocontrol(String str) {
		final int len = str.length();
		for (int i = 0; i < len; ) {
			final int ch = str.codePointAt(i);
			if (ch < 128) {
				if (ch < 0x20 || ch == 0x7f) {
					return false;
				}
			}
			i += Character.charCount(ch);
		}
		return true;
	}
	public static boolean is_name(String str) {
		final int len = str.length();
		if (len == 0) {
			return false;
		}
		int i = 0;
		{
			final int ch = str.codePointAt(i);
			if (! (ch == '_' || 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z')) {
				return false;
			}
			i += Character.charCount(ch);
		}
		while (i < len) {
			final int ch = str.codePointAt(i);
			if (! (ch == '_' || 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || '0' <= ch && ch <= '9')) {
				return false;
			}
			i += Character.charCount(ch);
		}
		return true;
	}
}
