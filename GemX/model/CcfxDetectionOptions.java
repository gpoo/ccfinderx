package model;

import java.util.*;

import utility.StringUtil;
import utility.StringChecker;
import utility.Pair;

public class CcfxDetectionOptions {
	static private String filedNameOfMinLengthOfClone = "b";
	static private String fieldNameOfShaperLevel = "s";
	static private String fieldNameOfUseParameterUnification = "u";
	static private String fieldNameOfMinTokenSetSize = "t";
	static private String filedNameofPostfix = "preprocessed_file_postfix";
	
	protected ArrayList<Pair<String, String>> keyValues;

	public CcfxDetectionOptions() {
		this.keyValues = new ArrayList<Pair<String, String>>();
	}
	
	public CcfxDetectionOptions(ArrayList<Pair<String, String>> otherValues) {
		this.keyValues = new ArrayList<Pair<String, String>>();
		for (Pair<String, String> item : otherValues) {
			this.keyValues.add(new Pair<String, String>(item.first, item.second));
		}
	}
	
	protected void add(String key, String value) {
		this.keyValues.add(new Pair<String, String>(key, value));
	}
	
	public CcfxDetectionOptions(
			int minLengthOfClone, int shaperLevel, 
			boolean useParameterUnification, int minTokenSetSize, ArrayList<Pair<String, String>> otherValues) {
		this(otherValues);
		add(filedNameOfMinLengthOfClone, String.valueOf(minLengthOfClone));
		add(fieldNameOfShaperLevel, String.valueOf(shaperLevel));
		add(fieldNameOfUseParameterUnification, useParameterUnification ? "+" : "-");
		add(fieldNameOfMinTokenSetSize, String.valueOf(minTokenSetSize));
	}
	
	public String[] toLines() {
		ArrayList<String> buffer = new ArrayList<String>();
		for (Pair<String, String> e : keyValues) {
			String line = String.format("%s\t%s", e.first, e.second);
			buffer.add(line);
		}
		return (String[])buffer.toArray();
	}
	
	public static CcfxDetectionOptions fromStrings(String[] lines) throws DataFileReadError {
		int minLengthOfClone = 0;
		int shaperLevel = 0;
		boolean useParameterUnification = false;
		int minTokenSetSize = 0;
		ArrayList<Pair<String, String>> otherValues = new ArrayList<Pair<String, String>>();
		for (int i = 0; i < lines.length; ++i) {
			String[] fields = StringUtil.split(lines[i], '\t');
			if (fields == null || fields.length != 2) {
				throw new DataFileReadError("Invalid option lines");
			}
			final String name = fields[0];
			final String value = fields[1];
			if (! StringChecker.is_name(name)) {
				throw new DataFileReadError("Invalid option name");
			}
			if (! StringChecker.is_utf8_nocontrol(value)) {
				throw new DataFileReadError("Invalid option value");
			}
			if (name.equals(filedNameOfMinLengthOfClone)) {
				try {
					minLengthOfClone = Integer.parseInt(value);
				} catch (NumberFormatException e) {
					throw new DataFileReadError("Invalid value for option -" + filedNameOfMinLengthOfClone);
				}
			} else if (name.equals(fieldNameOfShaperLevel)) {
				try {
					shaperLevel = Integer.parseInt(value);
				} catch (NumberFormatException e) {
					throw new DataFileReadError("Invalid value for option -" + fieldNameOfShaperLevel);
				}
			} else if (name.equals(fieldNameOfUseParameterUnification)) {
				useParameterUnification = value.equals("-");
			} else if (name.equals(fieldNameOfMinTokenSetSize)) {
				try {
					minTokenSetSize = Integer.parseInt(value);
				} catch (NumberFormatException e) {
					throw new DataFileReadError("Invalid value for option -" + fieldNameOfMinTokenSetSize);
				}
			} else {
				otherValues.add(new Pair<String, String>(name, value));
			}
		}
		return new CcfxDetectionOptions(minLengthOfClone, shaperLevel, useParameterUnification, minTokenSetSize, otherValues);
	}
	
	public String getDetector() {
		return "ccfx"; //$NON-NLS-1$
	}
	
	private int indexOf(String key) {
		for (int i = 0; i < keyValues.size(); ++i) {
			Pair<String, String> e = keyValues.get(i);
			if (e.first.equals(key)) {
				return i;
			}
		}
		return -1;
	}
	
	private int lastIndexOf(String key) {
		int foundI = -1;
		for (int i = 0; i < keyValues.size(); ++i) {
			Pair<String, String> e = keyValues.get(i);
			if (e.first.equals(key)) {
				foundI = i;
			}
		}
		return foundI;
	}
	
	public boolean hasMinimumCloneLength() {
		return indexOf(filedNameOfMinLengthOfClone) >= 0;
	}
	
	public int getMinimumCloneLength() {
		int index = lastIndexOf(filedNameOfMinLengthOfClone);
		if (index >= 0) {
			try {
				return Integer.parseInt(keyValues.get(index).second);
			} catch (NumberFormatException e) {
			}
		}
		return 0;
	}
	
	public boolean hasShaperLevel() {
		return indexOf(fieldNameOfShaperLevel) >= 0;
	}
	
	public int getShaperLevel() {
		int index = lastIndexOf(fieldNameOfShaperLevel);
		if (index >= 0) {
			try {
				return Integer.parseInt(keyValues.get(index).second);
			} catch (NumberFormatException e) {
			}
		}
		return 0;
	}
	
	public boolean hasUseParameterUnification() {
		return indexOf(fieldNameOfUseParameterUnification) >= 0;
	}
	
	public boolean isUseParameterUnification() {
		int index = lastIndexOf(fieldNameOfUseParameterUnification);
		if (index >= 0) {
			return keyValues.get(index).second.equals("+");
		}
		return false;
	}
	
	public boolean hasMinimumTokenSetSize() {
		return indexOf(fieldNameOfMinTokenSetSize) >= 0;
	}
	
	public int getMinimumTokenSetSize() {
		int index = lastIndexOf(fieldNameOfMinTokenSetSize);
		if (index >= 0) {
			try {
				return Integer.parseInt(keyValues.get(index).second);
			} catch (NumberFormatException e) {
			}
		}
		return 0;
	}
	
	public String getPostfix() {
		int index = lastIndexOf(filedNameofPostfix);
		if (index >= 0) {
			return keyValues.get(index).second;
		}
		return null;
	}
	
	public String[] get(String key) {
		ArrayList<String> vs = new ArrayList<String>();
		for (Pair<String, String> kv : keyValues) {
			if (kv.first.equals(key)) {
				vs.add(kv.second);
			}
		}
		return vs.toArray(new String[0]);
	}
	
	public static class Mutable extends CcfxDetectionOptions {
		public void setMinimumCloneLength(int minLengthOfClone) {
			add(filedNameOfMinLengthOfClone, String.valueOf(minLengthOfClone));
		}
		public void setShaperLevel(int shaperLevel) {
			add(fieldNameOfShaperLevel, String.valueOf(shaperLevel));
		}
		public void setUseParameterUnification(boolean useParameterUnification) {
			add(fieldNameOfUseParameterUnification, useParameterUnification ? "+" : "-");
		}
		public void setMinTokenSetSize(int minTokenSetSize) {
			add(fieldNameOfMinTokenSetSize, String.valueOf(minTokenSetSize));
		}
		public void addOption(String name, String value) throws DataFileReadError {
			if (! StringChecker.is_name(name)) {
				throw new DataFileReadError("Invalid option name");
			}
			if (! StringChecker.is_utf8_nocontrol(value)) {
				throw new DataFileReadError("Invalid option value");
			}
			add(name, value);
		}
		public CcfxDetectionOptions toImmutable() {
			return new CcfxDetectionOptions(this.keyValues);
		}
	}
}
