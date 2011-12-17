package utility;

import java.io.*;

import ccfinderx.CCFinderX;

public class PrepReader {
	private int valueOfHexChar(byte b) throws PrepReaderError {
		if ('0' <= b && b <= '9') {
			return b - '0';
		}
		else if ('a' <= b && b <= 'f') {
			return b - 'a' + 0xa;
		}
		else if ('A' <= b && b <= 'F') {
			return b - 'A' + 0xa;
		}
		else {
			throw new PrepReaderError();
		}
	}
	private int valueOfHexString(byte[] buffer, int begin, int end) throws PrepReaderError {
		int value = 0;
		for (int i = begin; i < end; ++i) {
			value <<= 4;
			value += valueOfHexChar(buffer[i]);
		}
		return value;
	}
	public PrepToken[] read(String sourceFilePath, String postfix) throws FileNotFoundException, IOException, PrepReaderError {
		byte[] buffer = CCFinderX.theInstance.openPrepFile(sourceFilePath, postfix);
		if (buffer == null) {
			throw new IOException();
		}
		
		int newLineCount = 0;
		for (int i = 0; i < buffer.length; ++i) {
			if (buffer[i] == 0x0a) {
				++newLineCount;
			}
		}
		
		PrepToken[] tokens = new PrepToken[newLineCount];
		int i = 0;
		// hex.hex.hex\thex.hex.hex\tstring
		// hex.hex.hex\t+hex\tstring
		int p = 0;
		while (p < buffer.length) {
			int sep0 = p - 1;
			
			while (p < buffer.length && buffer[p] != '.') {
				++p;
			}
			if (! (p < buffer.length)) {
				throw new PrepReaderError();
			}
			int sep1 = p;
			++p;
			
			while (p < buffer.length && buffer[p] != '.') {
				++p;
			}
			if (! (p < buffer.length)) {
				throw new PrepReaderError();
			}
			int sep2 = p;
			++p;
			
			while (p < buffer.length && buffer[p] != '\t') {
				++p;
			}
			if (! (p < buffer.length)) {
				throw new PrepReaderError();
			}
			int sep3 = p;
			++p;
			
			int beginRow = valueOfHexString(buffer, sep0 + 1, sep1);
			int beginCol = valueOfHexString(buffer, sep1 + 1, sep2);
			int beginIndex = valueOfHexString(buffer, sep2 + 1, sep3);
			
			int endRow;
			int endCol;
			int endIndex;
			int sep6;
			if (p < buffer.length && buffer[p] == '+') {
				while (p < buffer.length && buffer[p] != '\t') {
					++p;
				}
				if (! (p < buffer.length)) {
					throw new PrepReaderError();
				}
				sep6 = p;
				++p;

				int w = valueOfHexString(buffer, sep3 + 2, sep6);
				endRow = beginRow;
				endCol = beginCol + w;
				endIndex = beginIndex + w;
			}
			else {
				while (p < buffer.length && buffer[p] != '.') {
					++p;
				}
				if (! (p < buffer.length)) {
					throw new PrepReaderError();
				}
				int sep4 = p;
				++p;
				
				while (p < buffer.length && buffer[p] != '.') {
					++p;
				}
				if (! (p < buffer.length)) {
					throw new PrepReaderError();
				}
				int sep5 = p;
				++p;
				
				while (p < buffer.length && buffer[p] != '\t') {
					++p;
				}
				if (! (p < buffer.length)) {
					throw new PrepReaderError();
				}
				sep6 = p;
				++p;

				endRow = valueOfHexString(buffer, sep3 + 1, sep4);
				endCol = valueOfHexString(buffer, sep4 + 1, sep5);
				endIndex = valueOfHexString(buffer, sep5 + 1, sep6);
			}
			
			while (p < buffer.length && buffer[p] != '\n') {
				++p;
			}
			if (! (p < buffer.length)) {
				throw new PrepReaderError();
			}
			int sep7 = p;
			++p;
			
			int dustLen = 0;
			if (buffer[sep7 - 1] == '\r') {
				dustLen = 1;
			}
				
			String str = new String(buffer, sep6 + 1, sep7 - (sep6 + 1) - dustLen);
			
			tokens[i] = PrepToken.create(beginRow, beginCol, beginIndex, endRow, endCol, endIndex, str);
			++i;
		}
		assert i == tokens.length;
		
		return tokens;
	}
	
	public static void main(String[] args) throws FileNotFoundException, IOException, PrepReaderError {
		{
			File inputFile = new File(args[0]);
			String postfix = args[1];
			PrepReader reader = new PrepReader();
			PrepToken[] tokens = reader.read(inputFile.getParent(), postfix);
			for (int i = 0; i < tokens.length; ++i) {
				System.out.println(tokens[i].toString());
			}
		}
		System.exit(0);
	}
}
