package utility;

import gnu.trove.TByteArrayList;

import java.io.*;
import java.nio.channels.*;
import java.nio.*;

public class LittleEndianReader {
	private final byte[] buffer4 = new byte[4];
	private final byte[] buffer8 = new byte[8];
	private final ByteBuffer bbuffer1 = ByteBuffer.allocate(1);
	private final ByteBuffer bbuffer4 = ByteBuffer.allocate(4);
	private final ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
	public LittleEndianReader() {
		bbuffer4.order(ByteOrder.LITTLE_ENDIAN);
		bbuffer8.order(ByteOrder.LITTLE_ENDIAN);
	}
	
	public byte readByte(FileChannel c) throws IOException {
		bbuffer1.clear();
        c.read(bbuffer1);
        byte[] ary = bbuffer1.array();
        return ary[0];
	}
	public int readInt(FileChannel c) throws IOException {
		bbuffer4.clear();
		c.read(bbuffer4);
		bbuffer4.rewind();
		return bbuffer4.getInt();
	}
	public long readLong(FileChannel c) throws IOException {
		bbuffer8.clear();
		c.read(bbuffer8);
		bbuffer8.rewind();
		return bbuffer8.getLong();
	}
	public String readUntil(FileChannel c, byte terminatingByte) throws IOException {
		StringBuffer sb = new StringBuffer();
		while (true) {
			bbuffer1.clear();
			int b = c.read(bbuffer1);
			if (b == -1) {
				throw new IOException();
			}
			byte[] ary = bbuffer1.array();
			if (ary[0] == terminatingByte) {
				return sb.toString();
			}
			sb.append((char)ary[0]);
		}
	}
	public String readUtf8StringUntil(FileChannel c, byte terminatingByte) throws IOException {
		TByteArrayList buf = new TByteArrayList();
		while (true) {
			bbuffer1.clear();
			int b = c.read(bbuffer1);
			if (b == -1) {
				throw new IOException();
			}
			byte[] ary = bbuffer1.array();
			if (ary[0] == terminatingByte) {
				byte[] a = buf.toNativeArray();
				try {	
					return new String(a, "UTF-8"); //$NON-NLS-1$
				} catch (UnsupportedEncodingException e) {
					assert false;
					return ""; //$NON-NLS-1$
				}
			}
			buf.add(ary[0]);
		}
	}
	
	public byte readByte(InputStream i) throws IOException {
		int b = i.read();
		if (b == -1) {
			throw new IOException();
		}
		return (byte)b;
	}
	public int readInt(InputStream i) throws IOException {
		int readSize = i.read(buffer4);
		if (readSize != 4) {
			throw new IOException();
		}
		int r = (((buffer4[3] << 8) | buffer4[2] << 8) | buffer4[1] << 8) | buffer4[0];
		return r;
	}
	public long readLong(InputStream i) throws IOException {
		int readSize = i.read(buffer8);
		if (readSize != 8) {
			throw new IOException();
		}
		long r = 0;
		for (int j = 8; j-- > 0; ) {
			r = r << 8;
			r |= buffer8[j];
		}
		return r;
	}
	public String readUntil(InputStream i, byte terminatingByte) throws IOException {
		StringBuffer sb = new StringBuffer();
		while (true) {
			int b = i.read();
			if (b == -1) {
				throw new IOException();
			}
			if ((byte)b == terminatingByte) {
				return sb.toString();
			}
			sb.append((char)b);
		}
	}
	public String readUTF8StringUntil(InputStream i, byte terminatingByte) throws IOException {
		TByteArrayList buf = new TByteArrayList();
		while (true) {
			int b = i.read();
			if (b == -1) {
				throw new IOException();
			}
			if ((byte)b == terminatingByte) {
				byte[] a = buf.toNativeArray();
				try {	
					return new String(a, "UTF-8"); //$NON-NLS-1$
				} catch (UnsupportedEncodingException e) {
					assert false;
					return ""; //$NON-NLS-1$
				}
			}
			buf.add((byte)b);
		}
	}
}
