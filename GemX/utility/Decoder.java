package utility;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CoderMalfunctionError;
import java.nio.charset.CoderResult;

import com.ibm.icu.charset.CharsetProviderICU;

public class Decoder {
	private static StringBuilder replaceSarrogates(CharSequence str) {
		StringBuilder buf = new StringBuilder();
		int strLen = str.length();
		int i = 0; 
		while (i < strLen) {
			char c = str.charAt(i);
			if (0xd800 <= c && c <= 0xdbff) {
				buf.append((char)' ');
				i += 2;
			} else {
				buf.append(c);
				i += 1;
			}
		}
		return buf;
	}
	
	public static boolean isValidEncoding(String encodingName) {
		CharsetProviderICU charsetProvider = new CharsetProviderICU();
		Charset cs = charsetProvider.charsetForName(encodingName);
		return cs != null;
	}
	
	public static String decode(byte[] sequence, String encodingName) {
		Charset cs;
		if (encodingName == null || encodingName.length() == 0 || encodingName.equals("char")) {
			cs = Charset.defaultCharset();
		} else {
			CharsetProviderICU charsetProvider = new CharsetProviderICU();
			cs = charsetProvider.charsetForName(encodingName);
			if (cs == null) { return null; }
		}
		CharsetDecoder dec = cs.newDecoder();
		
		try {
			ByteBuffer input = ByteBuffer.wrap(sequence);
			CharBuffer output = CharBuffer.allocate(sequence.length * 8);
			CoderResult result = dec.decode(input, output, true);
			if (result.isError()) { return null; }
			int oPos = output.position();
			char[] ary = output.array();
			return replaceSarrogates(new String(ary, 0, oPos)).toString();
		} catch (CoderMalfunctionError e) {
			return null;
		}
	}
	
	public static String decode(byte[] sequence) {
		return decode(sequence, "char");
	}
}
