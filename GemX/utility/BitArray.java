package utility;

public final class BitArray implements Cloneable {
	private final byte[] body;
	private final int size;
	
	private static final int bitCountPatterns[];
	static {
		bitCountPatterns = new int[0x100];
		for (int i = 0; i < 0x100; ++i) {
			int bitCount = 0;
			for (int bitIndex = 0; bitIndex < 8; ++bitIndex) {
				if ((i & (1 << bitIndex)) != 0) {
					++bitCount;
				}
			}
			bitCountPatterns[i] = bitCount;
		}
	}
	
	public BitArray(int size) {
		assert size >= 0;
		this.size = size;
		this.body = new byte[(this.size + (8 - 1)) / 8];
	}
	public BitArray(int size, boolean initialValue) {
		this(size);
		if (initialValue) {
			for (int i = 0; i < this.body.length; ++i) {
				this.body[i] = (byte)0xff;
			}
		}
	}
	public Object clone() {
		final BitArray c = new BitArray(this.size);
		for (int i = 0; i < this.body.length; ++i) {
			c.body[i] = this.body[i];
		}
		return c;
	}
	public int length() {
		return this.size;
	}
	public boolean getAt(int index)  throws IndexOutOfBoundsException {
		if (! (0 <= index && index < this.size)) {
			throw new IndexOutOfBoundsException();
		}
		final int bytePos = index / 8;
		final int bitPos = index - (bytePos * 8);
		assert 0 <= bytePos && bytePos < this.size;
		assert 0 <= bitPos && bitPos < 8;
		return (this.body[bytePos] & (1 << bitPos)) != 0;
	}
	public int find(boolean value) {
		return find(value, 0);
	}
	public int find(boolean value, int startIndex) throws IndexOutOfBoundsException {
		if (startIndex == this.size) {
			return -1; // not found
		}
		if (! (0 <= startIndex && startIndex < this.size)) {
			throw new IndexOutOfBoundsException();
		}
		boolean found = false;
		int bytePos = startIndex / 8;
		int bitPos = startIndex - bytePos * 8;
		if (value) {
			while (! found) {
				for (; bytePos < this.body.length; ++bytePos) {
					if (this.body[bytePos] != 0) {
						break; // for bytePos
					}
				}
				if (bytePos == this.body.length) {
					return -1; // not found
				}
				for (; bitPos < 8; ++bitPos) {
					if ((this.body[bytePos] & (1 << bitPos)) != 0) {
						found = true;
						break; // for bitPos
					}
				}
				if (! found) {
					++bytePos;
					bitPos = 0;
				}
			}
		} else {
			while (! found) {
				for (; bytePos < this.body.length; ++bytePos) {
					if (this.body[bytePos] != (byte)0xff) {
						break; // for bytePos
					}
				}
				if (bytePos == this.body.length) {
					return -1; // not found
				}
				for (; bitPos < 8; ++bitPos) {
					if ((this.body[bytePos] & (1 << bitPos)) == 0) {
						found = true;
						break; // for bitPos
					}
				}
				if (! found) {
					++bytePos;
					bitPos = 0;
				}
			}
		}
		assert bitPos < 8;
		
		int foundIndex = bytePos * 8 + bitPos;
		if (foundIndex < this.size) {
			return foundIndex;
		}
		return -1; // not found
	}
	public void setAt(int index, boolean value) throws IndexOutOfBoundsException {
		if (! (0 <= index && index < this.size)) {
			throw new IndexOutOfBoundsException();
		}
		final int bytePos = index / 8;
		final int bitPos = index - (bytePos * 8);
		assert 0 <= bytePos && bytePos < this.size;
		assert 0 <= bitPos && bitPos < 8;
		if (value) {
			this.body[bytePos] |= (1 << bitPos);
		} else {
			this.body[bytePos] &= ~(1 << bitPos);
		}
	}
	public void fill(int beginIndex, int endIndex, boolean value) throws IndexOutOfBoundsException {
		if (beginIndex == endIndex) {
			return;
		}
		
		if (! (0 <= beginIndex && beginIndex <= endIndex && endIndex < this.size)) {
			throw new IndexOutOfBoundsException();
		}
		
		final int beginBytePos = beginIndex / 8;
		final int beginBitPos = beginIndex - (beginBytePos * 8);
		assert 0 <= beginBytePos && beginBytePos < this.size;
		assert 0 <= beginBitPos && beginBitPos < 8;
		
		final int endBytePos = endIndex / 8;
		final int endBitPos = endIndex - (endBytePos * 8);
		assert 0 <= endBytePos && endBytePos < this.size;
		assert 0 <= endBitPos && endBitPos < 8;
		
		if (beginBytePos == endBytePos) {
			if (value) {
				for (int bitPos = beginBitPos; bitPos < endBitPos; ++bitPos) {
					this.body[beginBytePos] |= (1 << bitPos);
				}
			} else {
				for (int bitPos = beginBitPos; bitPos < endBitPos; ++bitPos) {
					this.body[beginBytePos] &= ~(1 << bitPos);
				}
			}
		} else {
			if (value) {
				for (int bitPos = beginBitPos; bitPos < 8; ++bitPos) {
					this.body[beginBytePos] |= (1 << bitPos);
				}
				java.util.Arrays.fill(this.body, beginBytePos + 1, endBytePos, (byte)0xff);
				for (int bitPos = 0; bitPos < endBitPos; ++bitPos) {
					this.body[endBytePos] |= (1 << bitPos);
				}
			} else {
				for (int bitPos = beginBitPos; bitPos < 8; ++bitPos) {
					this.body[beginBytePos] &= ~(1 << bitPos);
				}
				java.util.Arrays.fill(this.body, beginBytePos + 1, endBytePos, (byte)0);
				for (int bitPos = 0; bitPos < endBitPos; ++bitPos) {
					this.body[endBytePos] &= ~(1 << bitPos);
				}
			}
		}
	}
	public int count() {
		if (this.size == 0) {
			return 0;
		}
		
		final int byteSize = (this.size + (8 - 1)) / 8;
		final int lastByteIndex = byteSize - 1;
		
		int bitCount = 0;
		for (int i = 0; i < lastByteIndex; ++i) {
			int unsignedValue = (this.body[i] + 0x100) % 0x100;
			bitCount += bitCountPatterns[unsignedValue];
		}
		final int lastByteBits = this.size - 8 * lastByteIndex;
		for (int i = 0; i < lastByteBits; ++i) {
			if ((this.body[lastByteIndex] & (1 << i)) != 0) {
				bitCount += 1;
			}
		}
		return bitCount;
	}
	
	public static void main(String args[]) {
		test3();
		test2();
		test1();
	}
	private static void test3() {
		final java.util.Random rnd = new java.util.Random(0);

		java.util.Calendar cal1 = java.util.Calendar.getInstance();
		for (int t = 0; t < 5000; ++t) {
			final int size = rnd.nextInt(1000);
			{
				BitArray bita = new BitArray(size);
				
				for (int j = 0; j < size / 3; ++j) {
					int index = rnd.nextInt(size);
					bita.setAt(index, true);
				}
			}
			{
				BitArray bita = new BitArray(size);
				
				for (int j = 0; j < size / 3; ++j) {
					int beginIndex = rnd.nextInt(size);
					int endIndex = rnd.nextInt(size);
					if (endIndex >= beginIndex) {
						boolean value = (rnd.nextInt() / 7) % 2 == 0;
						bita.fill(beginIndex, endIndex, value);
					}
				}
			}
		}
		java.util.Calendar cal2 = java.util.Calendar.getInstance();
		for (int t = 0; t < 5000; ++t) {
			final int size = rnd.nextInt(1000);
			{
				boolean[] boola = new boolean[size];
				
				for (int j = 0; j < size / 3; ++j) {
					int index = rnd.nextInt(size);
					boola[index] = true;
				}
			}
			{
				boolean[] boola = new boolean[size];
				
				for (int j = 0; j < size / 3; ++j) {
					int beginIndex = rnd.nextInt(size);
					int endIndex = rnd.nextInt(size);
					if (endIndex >= beginIndex) {
						boolean value = (rnd.nextInt() / 7) % 2 == 0;
						java.util.Arrays.fill(boola, beginIndex, endIndex, value);
					}
				}
			}
		}
		java.util.Calendar cal3 = java.util.Calendar.getInstance();
		
		long lap1 = cal1.getTimeInMillis();
		long lap2 = cal2.getTimeInMillis();
		long lap3 = cal3.getTimeInMillis();
		
		System.out.println("BitArray: " + String.valueOf(lap2 - lap1) + " milli-sec.");
		System.out.println("boolean[]: " + String.valueOf(lap3 - lap2) + " milli-sec.");
	}
	
	private static int findMismatchIndex(BitArray bita, boolean[] boola) {
		int size = bita.length();
		int size2 = boola.length;
		assert size == size2;
		for (int i = 0; i < size; ++i) {
			boolean valueBit = bita.getAt(i);
			boolean valueBool = boola[i];
			if (valueBit != valueBool) {
				return i;
			}
		}
		return -1;
	}
	private static void test1() {
		final java.util.Random rnd = new java.util.Random(0);

		System.out.println("Test start.");
		
		for (int t = 0; t < 5000; ++t) {
			final int size = rnd.nextInt(1000);
			{
				BitArray bita = new BitArray(size);
				boolean[] boola = new boolean[size];
				
				for (int j = 0; j < size / 3; ++j) {
					int index = rnd.nextInt(size);
					bita.setAt(index, true);
					boola[index] = true;
				}
				
				int mismatchIndex = findMismatchIndex(bita, boola);
				if (mismatchIndex >= 0) {
					boolean valueBit = bita.getAt(mismatchIndex);
					boolean valueBool = boola[mismatchIndex];
					System.out.println("Failed. index = " + String.valueOf(mismatchIndex) 
							+ ", valueBit = " + String.valueOf(valueBit) + ", valueBool = " + String.valueOf(valueBool));
					System.exit(1);
				}
				
				int bitCount = 0;
				for (int i = 0; i < size; ++i) {
					boolean valueBool = boola[i];
					if (valueBool) {
						++bitCount;
					}
				}
				if (bita.count() != bitCount) {
					System.out.println("Failed. bita.count() = " + bita.count()
							+ ", bitCount = " + String.valueOf(bitCount));
					System.exit(1);
				}
				
				int bitCount2 = 0;
				int lastIndex = 0;
				int index = bita.find(true, lastIndex);
				while (index >= 0) {
					++bitCount2;
					for (int j = lastIndex; j < index; ++j) {
						if (boola[j] == true) {
							System.out.println("Failed. invalid value of bita.find() #1");
							System.exit(1);
						}
					}
					if (boola[index] != true) {
						System.out.println("Failed. invalid value of bita.find() #2");
						System.exit(1);
					}
					lastIndex = index + 1;
					index = bita.find(true, index + 1);
				}
				if (bitCount2 != bitCount) {
					System.out.println("Failed. bitCount = " + bitCount
							+ ", bitCount2 = " + bitCount2);
					System.exit(1);
				}
			}
			{
				BitArray bita = new BitArray(size);
				boolean[] boola = new boolean[size];
				
				for (int j = 0; j < size / 3; ++j) {
					int beginIndex = rnd.nextInt(size);
					int endIndex = rnd.nextInt(size);
					if (endIndex >= beginIndex) {
						boolean value = (rnd.nextInt() / 7) % 2 == 0;
						bita.fill(beginIndex, endIndex, value);
						java.util.Arrays.fill(boola, beginIndex, endIndex, value);
					}
				}
				
				int mismatchIndex = findMismatchIndex(bita, boola);
				if (mismatchIndex >= 0) {
					boolean valueBit = bita.getAt(mismatchIndex);
					boolean valueBool = boola[mismatchIndex];
					System.out.println("Failed. index = " + String.valueOf(mismatchIndex) 
							+ ", valueBit = " + String.valueOf(valueBit) + ", valueBool = " + String.valueOf(valueBool));
					System.exit(1);
				}
			}
		}
		System.out.println("Test end.");
	}
	
	private static void test2() {
		  final int ArraySize = 1000000;
		  Runtime runtime = Runtime.getRuntime();

		  long used0 = runtime.totalMemory() - runtime.freeMemory();
		  boolean[] boolArray = new boolean[ArraySize];
		  long used1 = runtime.totalMemory() - runtime.freeMemory();
		  BitArray bitArray = new BitArray(ArraySize);
		  long used2 = runtime.totalMemory() - runtime.freeMemory();
				
		  System.out.println(used1 - used0);
		  System.out.println(used2 - used1);

		  boolArray[ArraySize-1] = true;
		  bitArray.setAt(ArraySize - 1, true);
		}

}
