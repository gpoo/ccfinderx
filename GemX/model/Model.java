package model;

import gnu.trove.*;

import java.io.*;
import java.nio.*;
import java.nio.channels.*;
import java.util.*;

import ccfinderx.CCFinderX;

import utility.LittleEndianReader;
import utility.SetOperators;
import utility.TemporaryFileManager;
import utility.StringUtil;

public class Model {
	private final String GEMXCLONESETDATA_TEMPFILE = TemporaryFileManager.createTemporaryFileName();
	private final String SCOPE_TEMPFILE_FORMAT = TemporaryFileManager.createTemporaryFileName("gemxscope%d", ".ccfxd.tmp"); //$NON-NLS-1$ //$NON-NLS-2$
	private static final int[] exceptedCcfxVersion = { 10, 2 };

	private String cloneDataFilePath = null;
	
	private CcfxDetectionOptions detectionOption = null;

	private String preprocessScript = null;

	private ArrayList<SourceFileAndPosition> sourceFiles = null;

	private TIntObjectHashMap<ArrayList<String>> sourceFileRemarks = null;
	
	private long totalFileSize;

	private FileChannel cloneSetDataStore = null;

	private long maxCloneSetID = 0;
	private long cloneSetIDCount = 0;
	
	private int[] fileIdToFileIndex = null;
	private int maxFileID = 0;
	
	private static int tempFileCounter = 0;
	
	private String commonPath = ""; //$NON-NLS-1$
	
	private boolean hasSortedFileList = false;

	public void dispose() {
		if (cloneSetDataStore != null) {
			try {
				cloneSetDataStore.close();
				
				File f = new File(GEMXCLONESETDATA_TEMPFILE);
				f.delete();
			} catch (IOException e) {
				// can do nothing
			}
		}
	}

	public CcfxDetectionOptions getDetectionOption() {
		return detectionOption;
	}
	
	public String getPreprocessScript() {
		return preprocessScript;
	}
	
	public String getCloneDataFilePath() {
		return cloneDataFilePath;
	}

	public int getFileCount() {
		if (sourceFiles == null || sourceFiles.size() == 0) {
			return 0;
		}

		return sourceFiles.size();
	}
	
	public String[] getFiles() {
		String[] files = new String[sourceFiles.size()];
		for (int i = 0; i < sourceFiles.size(); ++i) {
			files[i] = sourceFiles.get(i).path;
		}
		return files;
	}

	public long totalSizeOfFiles() {
		return totalFileSize;
	}

	public int findFile(SourceFile file) {
		for (int i = 0; i < sourceFiles.size(); ++i) {
			SourceFile f = sourceFiles.get(i);
			if (file.equals(f)) {
				return i;
			}
		}
		return -1; // not found
	}
	
	public SourceFile getFile(int index) throws IndexOutOfBoundsException {
		if (!(0 <= index && index < sourceFiles.size())) {
			throw new IndexOutOfBoundsException();
		}

		return sourceFiles.get(index);
	}

	public int[] getFileIDFromFileIndex(int[] indices) throws IndexOutOfBoundsException {
		int[] ids = new int[indices.length];
		for (int i = 0; i < indices.length; ++i) {
			int index = indices[i];
			if (!(0 <= index && index < sourceFiles.size())) {
				throw new IndexOutOfBoundsException();
			}
			SourceFile f = sourceFiles.get(index);
			ids[i] = f.id;
		}
		return ids;
	}
	
	public int[] getFileIndexFromFileID(int[] ids) {
		TIntIntHashMap id2Index = new TIntIntHashMap();
		{
			int index = 0;
			for (SourceFileAndPosition sfap : sourceFiles) {
				id2Index.put(sfap.id, index);
				++index;
			}
		}
		
		TIntArrayList list = new TIntArrayList();
		for (int id : ids) {
			int index = id2Index.get(id);
			if (index >= 0) {
				list.add(index);
			}
		}
		
		return list.toNativeArray();
	}
	
	public boolean containsFileRemark(int fileID) {
		return sourceFileRemarks.contains(fileID);
	}
	
	public ArrayList<String> getFileRemarkFromFileID(int fileID) {
		if (sourceFileRemarks.contains(fileID)) {
			return (ArrayList<String>)sourceFileRemarks.get(fileID);
		} else {
			return null;
		}
	}
	
	public int getClonePairCountOfFile(int index) throws IndexOutOfBoundsException {
		if (!(0 <= index && index < sourceFiles.size())) {
			throw new IndexOutOfBoundsException();
		}

		SourceFileAndPosition i = sourceFiles.get(index);
		return (int) ((i.endPosition - i.beginPosition) / 32);
	}

	public int[] getNeighborFiles(int[] fileIndices, boolean includeThemselves) throws IndexOutOfBoundsException {
		TIntHashSet neighborFileIndices = new TIntHashSet();
		
		for (int fi = 0; fi < fileIndices.length; ++fi) {
			int index = fileIndices[fi];
			if (!(0 <= index && index < sourceFiles.size())) {
				throw new IndexOutOfBoundsException();
			}
			SourceFileAndPosition fA = sourceFiles.get(index);
			
			try {
				RandomAccessFile raFile = new RandomAccessFile(cloneDataFilePath, "r"); //$NON-NLS-1$
				FileChannel channel = raFile.getChannel();
				try {
					long position = fA.beginPosition;
					channel.position(position);
					ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
					bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
					while ((position = channel.position()) < fA.endPosition) {
						bbuffer32.clear();
						channel.read(bbuffer32);
						int rf = bbuffer32.getInt(12);
						int idx = fileIdToFileIndex[rf];
						neighborFileIndices.add(idx);
					}
				} finally {
					channel.close();
				}
			} catch (IOException e) {
				return null;
			}
		}
		
		if (includeThemselves) {
			for (int i = 0; i < fileIndices.length; ++i) {
				neighborFileIndices.add(fileIndices[i]);
			}
		}
		int[] indices = neighborFileIndices.toArray();
		Arrays.sort(indices);
		return indices;
	}
	
	public long[] getCloneSetIDsIncludedFile(int index) throws IndexOutOfBoundsException {
		if (!(0 <= index && index < sourceFiles.size())) {
			throw new IndexOutOfBoundsException();
		}
		SourceFileAndPosition fA = sourceFiles.get(index);
		
		TLongHashSet idSet = new TLongHashSet();
		
		try {
			RandomAccessFile raFile = new RandomAccessFile(cloneDataFilePath, "r"); //$NON-NLS-1$
			FileChannel channel = raFile.getChannel();
			try {
				long position = fA.beginPosition;
				channel.position(position);
				ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
				bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
				while ((position = channel.position()) < fA.endPosition) {
					bbuffer32.clear();
					channel.read(bbuffer32);
					long classID = bbuffer32.getLong(24);
					idSet.add(classID);
				}
			} finally {
				channel.close();
			}
		} catch (IOException e) {
			return null;
		}
		
		long[] ids = idSet.toArray();
		Arrays.sort(ids);
		return ids;
	}
	
	public long[] getCloneSetIDsIncludedFiles(int[] indices) throws IndexOutOfBoundsException {
		int[] fileIDs = indices.clone();
		Arrays.sort(fileIDs);
		
		TLongHashSet idSet = new TLongHashSet();
		
		try {
			RandomAccessFile raFile = new RandomAccessFile(cloneDataFilePath, "r"); //$NON-NLS-1$
			FileChannel channel = raFile.getChannel();
			try {
				for (int index : fileIDs) {
					if (!(0 <= index && index < sourceFiles.size())) {
						throw new IndexOutOfBoundsException();
					}
					SourceFileAndPosition fA = sourceFiles.get(index);
					
					long position = fA.beginPosition;
					channel.position(position);
					ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
					bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
					while ((position = channel.position()) < fA.endPosition) {
						bbuffer32.clear();
						channel.read(bbuffer32);
						long classID = bbuffer32.getLong(24);
						idSet.add(classID);
					}
				}
			} finally {
				channel.close();
			}
		} catch (IOException e) {
			return null;
		}
		
		long[] ids = idSet.toArray();
		Arrays.sort(ids);
		return ids;
	}
	
	public long[] getCloneSetIDsCommonlyIncludedFiles(int[] indices) throws IndexOutOfBoundsException {
		if (indices.length == 1) {
			return getCloneSetIDsIncludedFile(indices[0]);
		}
		
		int[] fileIDs = indices.clone();
		Arrays.sort(fileIDs);
		
		TLongHashSet idSet0 = new TLongHashSet();
		try {
			RandomAccessFile raFile = new RandomAccessFile(cloneDataFilePath, "r"); //$NON-NLS-1$
			FileChannel channel = raFile.getChannel();
			try {
				{
					int index = fileIDs[0];
					if (!(0 <= index && index < sourceFiles.size())) {
						throw new IndexOutOfBoundsException();
					}
					SourceFileAndPosition fA = sourceFiles.get(index);
					
					long position = fA.beginPosition;
					channel.position(position);
					ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
					bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
					while ((position = channel.position()) < fA.endPosition) {
						bbuffer32.clear();
						channel.read(bbuffer32);
						long classID = bbuffer32.getLong(24);
						idSet0.add(classID);
					}
				}
				for (int i = 1; i < fileIDs.length; ++i) {
					TLongHashSet idSet = new TLongHashSet();
					int index = fileIDs[i];
					if (!(0 <= index && index < sourceFiles.size())) {
						throw new IndexOutOfBoundsException();
					}
					SourceFileAndPosition fA = sourceFiles.get(index);
					
					long position = fA.beginPosition;
					channel.position(position);
					ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
					bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
					while ((position = channel.position()) < fA.endPosition) {
						bbuffer32.clear();
						channel.read(bbuffer32);
						long classID = bbuffer32.getLong(24);
						idSet.add(classID);
					}
					
					//long[] ids = idSet.toArray();
					idSet0 = SetOperators.getUnion(idSet0, idSet);
					if (idSet0.size() == 0) {
						return new long[0]; // return an empty set
					}
				}
			} finally {
				channel.close();
			}
		} catch (IOException e) {
			return null;
		}
		
		long[] ids = idSet0.toArray();
		Arrays.sort(ids);
		return ids;
	}
	
	public int getCloneSetCountOfFile(int index) throws IndexOutOfBoundsException {
		if (!(0 <= index && index < sourceFiles.size())) {
			throw new IndexOutOfBoundsException();
		}
		SourceFileAndPosition fA = sourceFiles.get(index);
		TLongHashSet ids = new TLongHashSet();
		try {
			RandomAccessFile raFile = new RandomAccessFile(cloneDataFilePath, "r"); //$NON-NLS-1$
			FileChannel channel = raFile.getChannel();
			try {
				long position = fA.beginPosition;
				channel.position(position);
				ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
				bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
				while ((position = channel.position()) < fA.endPosition) {
					bbuffer32.clear();
					channel.read(bbuffer32);
					long classID = bbuffer32.getLong(24);
					ids.add(classID);
				}
				return ids.size();
			} finally {
				channel.close();
			}
		} catch (IOException e) {
			return 0;
		}
	}
	
	public ClonePair[] getClonePairsOfFile(int index) throws IndexOutOfBoundsException {
		if (!(0 <= index && index < sourceFiles.size())) {
			throw new IndexOutOfBoundsException();
		}
		ArrayList<ClonePair> list = new ArrayList<ClonePair>();
		SourceFileAndPosition fA = sourceFiles.get(index);
		try {
			RandomAccessFile raFile = new RandomAccessFile(cloneDataFilePath,
					"r"); //$NON-NLS-1$
			FileChannel channel = raFile.getChannel();
			try {
				long position = fA.beginPosition;
				channel.position(position);
				ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
				bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
				while ((position = channel.position()) < fA.endPosition) {
					bbuffer32.clear();
					channel.read(bbuffer32);
					int lf = bbuffer32.getInt(0);
					int leftFileIndex = fileIdToFileIndex[lf];
					int lb = bbuffer32.getInt(4);
					int le = bbuffer32.getInt(8);
					int rf = bbuffer32.getInt(12);
					int rightFileIndex = fileIdToFileIndex[rf];
					int rb = bbuffer32.getInt(16);
					int re = bbuffer32.getInt(20);
					long classID = bbuffer32.getLong(24);
					ClonePair pair = new ClonePair(leftFileIndex, lb, le, rightFileIndex, rb, re,
							classID);
					list.add(pair);
				}
				return list.toArray(new ClonePair[] {});
			} finally {
				channel.close();
			}
		} catch (IOException e) {
			return null;
		}
	}

	public ClonePair[] getClonePairsOfFile(int index, int rightIndexBegin, int rightIndexEnd) throws IndexOutOfBoundsException {
		if (!(0 <= index && index < sourceFiles.size())) {
			throw new IndexOutOfBoundsException();
		}
		ArrayList<ClonePair> list = new ArrayList<ClonePair>();
		SourceFileAndPosition fA = sourceFiles.get(index);
		try {
			RandomAccessFile raFile = new RandomAccessFile(cloneDataFilePath, "r"); //$NON-NLS-1$
			FileChannel channel = raFile.getChannel();
			try {
				long position = fA.beginPosition;
				channel.position(position);
				ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
				bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
				while ((position = channel.position()) < fA.endPosition) {
					bbuffer32.clear();
					channel.read(bbuffer32);
					int lf = bbuffer32.getInt(0);
					int leftFileIndex = fileIdToFileIndex[lf];
					int lb = bbuffer32.getInt(4);
					int le = bbuffer32.getInt(8);
					int rf = bbuffer32.getInt(12);
					int rightFileIndex = fileIdToFileIndex[rf];
					if (rightIndexBegin <= rightFileIndex && rightFileIndex < rightIndexEnd) {
						int rb = bbuffer32.getInt(16);
						int re = bbuffer32.getInt(20);
						long classID = bbuffer32.getLong(24);
						ClonePair pair = new ClonePair(leftFileIndex, lb, le, rightFileIndex, rb, re,
								classID);
						list.add(pair);
					}
				}
				return list.toArray(new ClonePair[] {});
			} finally {
				channel.close();
			}
		} catch (IOException e) {
			return null;
		}
	}
	
	public int[] findFilesUnderDirectory(String dir) {
		TIntArrayList fileIndice = new TIntArrayList();
		for (int i = 0; i < sourceFiles.size(); ++i) {
			SourceFileAndPosition source = sourceFiles.get(i);
			if (source.path.startsWith(dir)) {
				fileIndice.add(i);
			}
		}
		return fileIndice.toNativeArray();
	}

	private String getDir(String pathFileName) {
		int pos = pathFileName.lastIndexOf('\\');
		int pos2 = pathFileName.lastIndexOf('/');
		if (pos2 > pos) {
			pos = pos2;
		}
		if (pos >= 0) {
			return pathFileName.substring(0, pos + 1);
		} else {
			return ""; //$NON-NLS-1$
		}
	}
	
	public void readCloneDataFile(String path) throws DataFileReadError,
			IOException {
		dispose();
		
		sourceFiles = new ArrayList<SourceFileAndPosition>();
		sourceFileRemarks = new TIntObjectHashMap<ArrayList<String>>();
		cloneDataFilePath = path;
		totalFileSize = 0;
		cloneSetDataStore = null;
		maxCloneSetID = -1;
		cloneSetIDCount = 0;
		fileIdToFileIndex = null;
		maxFileID = -1;
		commonPath = ""; //$NON-NLS-1$
		hasSortedFileList = true;
		
		String rootDirPath = getDir(path);

		try {
			LittleEndianReader ler = new LittleEndianReader();

			{
				File f = new File(GEMXCLONESETDATA_TEMPFILE);
				RandomAccessFile raFile = new RandomAccessFile(f, "rw"); //$NON-NLS-1$
				cloneSetDataStore = raFile.getChannel();
				cloneSetDataStore.truncate(0);
			}

			RandomAccessFile raFile = new RandomAccessFile(path, "r"); //$NON-NLS-1$
			FileChannel channel = raFile.getChannel();

			// check magic number
			{
				ByteBuffer buffer = ByteBuffer.allocate(8);
				channel.read(buffer);
				byte[] ary = buffer.array();

				String b = new String(ary);
				if (!b.equals("ccfxraw0")) { //$NON-NLS-1$
					throw new DataFileReadError("Invalid file"); //$NON-NLS-1$
				}
			}

			// check version and format 
			{
				int v1 = ler.readInt(channel);
				int v2 = ler.readInt(channel);
				/* int v3 = */ ler.readInt(channel);
				
				ByteBuffer buffer = ByteBuffer.allocate(4);
				channel.read(buffer);
				byte[] ary = buffer.array();
				String b = new String(ary);
				if (b.equals("pa:d")) { //$NON-NLS-1$
					if (!(v1 == exceptedCcfxVersion[0] && v2 == exceptedCcfxVersion[1])) { // version check will be done to the first two numbers. the last number will be omitted.
						throw new DataFileReadError("Version mismatch"); //$NON-NLS-1$
					}
				} else {
					throw new DataFileReadError("Invalid format"); //$NON-NLS-1$
				}
			}

			// read options
			{
				CcfxDetectionOptions.Mutable options = new CcfxDetectionOptions.Mutable();
				
				while (true) {
					String line = ler.readUtf8StringUntil(channel, (byte) 0xa/* \n */);
					if (line.length() == 0) {
						break;
					}
					final String[] fields = StringUtil.split(line, '\t');
					if (fields == null || fields.length != 2) {
						throw new DataFileReadError("Invalid option data"); //$NON-NLS-1$
					}
					final String name = fields[0];
					final String value = fields[1];
					options.addOption(name, value);
				}
				this.detectionOption = options.toImmutable();
			}
			
			// read preprocess script
			preprocessScript = ler.readUtf8StringUntil(channel, (byte) 0xa/* \n */);

			// read source file data
			TIntArrayList id2idx = new TIntArrayList();
			int lastId = -1;
			int fileCount = 0;
			while (true) {
				String filePath = ler.readUtf8StringUntil(channel, (byte) 0xa/* \n */);
				if (filePath.length() == 0) {
					int fileId = ler.readInt(channel);
					int length = ler.readInt(channel);
					if (! (fileId == 0 && length == 0)) {
						throw new DataFileReadError("Invalid file terminator"); //$NON-NLS-1$
					}
					break; // while
				}
				String s;
				if (rootDirPath.length() > 0 && 
						filePath.length() >= 2 && ((s = filePath.substring(0, 2)).equals("./") || s.equals(".\\"))) { //$NON-NLS-1$ //$NON-NLS-2$
					filePath = rootDirPath + filePath.substring(2);
				}
				if (fileCount == 0) {
					commonPath = filePath;
				} else {
					commonPath = calcCommonPath(commonPath, filePath);
				}
				int fileId = ler.readInt(channel);
				if (! (lastId < fileId)) {
					//throw new CloneDataFileReadError("Invalid file order"); //$NON-NLS-1$
				}
				int length = ler.readInt(channel);
				SourceFileAndPosition source = new SourceFileAndPosition(fileId, filePath, length, 0, 0);
				sourceFiles.add(source);
				while (! (fileId < id2idx.size())) {
					id2idx.add(-1); 
				}
				id2idx.set(fileId, fileCount);
				if (fileId < lastId) {
					hasSortedFileList = false;
				}
				if (fileId > maxFileID) {
					maxFileID = fileId;
				}
				lastId = fileId;
				totalFileSize += length;
				++fileCount;
			}
			//SourceFileAndPosition terminator = new SourceFileAndPosition(0, "", 0, -1, -1); //$NON-NLS-1$
			//sourceFiles.add(terminator);
			fileIdToFileIndex = new int[id2idx.size()];
			int p = 0;
			for (int i = 0; i < id2idx.size(); ++i) {
				fileIdToFileIndex[p] = id2idx.get(i);
				++p;
			}
			
			// read source file remarks
			{
				while (true) {
					String remarkText = ler.readUtf8StringUntil(channel, (byte) 0xa/* \n */);
					if (remarkText.length() == 0) {
						int fileId = ler.readInt(channel);
						if (fileId != 0) {
							throw new DataFileReadError("Invalid file remark terminator"); //$NON-NLS-1$
						}
						break; // while true
					}
					int fileId = ler.readInt(channel);
					{
						ArrayList<String> remarks = null;
						if (! sourceFileRemarks.contains(fileId)) {
							remarks = new ArrayList<String>();
							sourceFileRemarks.put(fileId, remarks);
						} else {
							remarks = (ArrayList<String>)sourceFileRemarks.get(fileId);
						}
						remarks.add(remarkText);
					}
				}
			}

			// read clone data
			{
				int leftFile = -1;
				long fileSize = channel.size();
				long position;
				ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
				bbuffer8.order(ByteOrder.LITTLE_ENDIAN);
				ByteBuffer bbuffer32 = ByteBuffer.allocate(32);
				bbuffer32.order(ByteOrder.LITTLE_ENDIAN);
				while ((position = channel.position()) < fileSize) {
					bbuffer32.clear();
					channel.read(bbuffer32);
					bbuffer32.rewind();
					{
						int leftId = bbuffer32.getInt();
						if (leftId == 0/* terminator */) {
							break; // while
						}
						int lf;
						if (!(0 <= leftId && leftId < fileIdToFileIndex.length && (lf = fileIdToFileIndex[leftId]) != -1)) {
							throw new DataFileReadError("Invalid file id"); //$NON-NLS-1$
						}
						if (lf != leftFile) {
							if (leftFile != -1) {
								SourceFileAndPosition s = sourceFiles.get(leftFile);
								sourceFiles.set(leftFile, new SourceFileAndPosition(s.id, s.path, s.size, s.beginPosition, position));
							}
							{
								SourceFileAndPosition s = sourceFiles.get(lf);
								sourceFiles.set(lf, new SourceFileAndPosition(s.id, s.path, s.size, position, -1));
							}
							leftFile = lf;
						}
					}
					{
						int rightId = bbuffer32.getInt(12);
						if (!(0 <= rightId && rightId < fileIdToFileIndex.length && fileIdToFileIndex[rightId] != -1)) {
							throw new DataFileReadError("Invalid file id"); //$NON-NLS-1$
						}
					}
					
					// make clone set data
					{
						bbuffer32.rewind();
						int begin = bbuffer32.getInt(4);
						int end = bbuffer32.getInt(8);
						int length = end - begin;
						long cloneSetID = bbuffer32.getLong(24);
						long csPosition = cloneSetID * (4/* length */+ 4/* fileIndex */);

						// expand data store
						if (maxCloneSetID < cloneSetID) {
							bbuffer8.clear();
							bbuffer8.putInt(-1); // length
							bbuffer8.putInt(-1); // fileIndex
							for (long d = maxCloneSetID + 1; d < cloneSetID; ++d) {
								long pos = d * (4 + 4);
								cloneSetDataStore.position(pos);
								bbuffer8.rewind();
								cloneSetDataStore.write(bbuffer8);
							}
						}

						// write data
						{
							bbuffer8.clear();
							cloneSetDataStore.position(csPosition);
							cloneSetDataStore.read(bbuffer8);
							int clen = bbuffer8.getInt(0);
							if (clen == -1) {
								bbuffer8.clear();
								bbuffer8.putInt(length);
								bbuffer8.putInt(leftFile);
								bbuffer8.rewind();
								cloneSetDataStore.position(csPosition);
								cloneSetDataStore.write(bbuffer8);
								++cloneSetIDCount;
							}
						}

						if (maxCloneSetID < cloneSetID) {
							maxCloneSetID = cloneSetID;
						}
					}
				}
				if (leftFile != -1) {
					SourceFileAndPosition s = sourceFiles.get(leftFile);
					sourceFiles.set(leftFile, new SourceFileAndPosition(s.id, s.path, s.size, s.beginPosition, position));
				}
			}

			// read clone set remarks
			{
				while (true) {
					String remarkText = ler.readUtf8StringUntil(channel, (byte) 0xa/* \n */);
					if (remarkText.length() == 0) {
						long cloneId = ler.readLong(channel);
						if (cloneId != 0) {
							throw new DataFileReadError("Invalid clone-set remark terminator"); //$NON-NLS-1$
						}
						break; // while true
					}
					/* long cloneId = */ ler.readLong(channel);
					
					// store clone-set remark, e.g., "cloneSetRemarkTable[cloneId] = remarkText;"
				}
			}

			channel.close();
		} catch (DataFileReadError e) {
			sourceFiles = null;
			cloneDataFilePath = null;
			cloneSetDataStore = null;
			throw e;
		} catch (IOException e) {
			sourceFiles = null;
			cloneDataFilePath = null;
			cloneSetDataStore = null;
			throw e;
		}
	}
	
	private String calcCommonPath(String a, String b) {
		int lastPathSepPos = -1;
		for (int i = 0; i < a.length() && i < b.length(); ++i) {
			char c = a.charAt(i);
			char d = b.charAt(i);
			if (c == d) {
				if (c == '\\' || c == '/') {
					lastPathSepPos = i;
				}
			} else {
				break; // for i
			}
		}
		if (lastPathSepPos >= 0) {
			return a.substring(0, lastPathSepPos + 1);
		} else {
			return ""; //$NON-NLS-1$
		}
	}

	public Model fitScopeToFileIDs(String fileIdList) throws CCFinderXInvocationException {
		String tempFile = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		String[] args = { 
				"S",  //$NON-NLS-1$
				this.cloneDataFilePath,
				"-o", tempFile, //$NON-NLS-1$
				"-fi", fileIdList  //$NON-NLS-1$
				};
		++tempFileCounter;
		CCFinderX ccfx = CCFinderX.theInstance;
		int r = ccfx.invokeCCFinderX(args);
		if (r != 0) {
			throw new CCFinderXInvocationException();
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile);
		Model newModel = new Model();
		try {
			newModel.readCloneDataFile(tempFile);
		} catch (IOException e) {
			throw new CCFinderXInvocationException();
		}
		
		return newModel;
	}
	
	public Model fitScopeToFiles(int[] selectedFiles) throws CCFinderXInvocationException {
		int[] files = selectedFiles.clone();
		for (int i = 0; i < files.length; ++i) {
			files[i] = sourceFiles.get(selectedFiles[i]).id;
		}
		Arrays.sort(files);
		
		StringBuilder buf = new StringBuilder();
		int i = 0;
		while (i < files.length) {
			int p = i;
			while (p + 1 < files.length && files[p + 1] == files[p] + 1) {
				++p;
			}
			buf.append(p != i ? String.format("%d-%d,", files[i], files[p])
					: String.format("%d,", files[i]));
			i = p + 1;
		}
		String tempFile = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		final String[] args = { 
				"S",  //$NON-NLS-1$
				this.cloneDataFilePath,
				"-o", tempFile, //$NON-NLS-1$
				"-f", buf.subSequence(0, buf.length() - 1).toString()  //$NON-NLS-1$
				};
		++tempFileCounter;
		CCFinderX ccfx = CCFinderX.theInstance;
		int r = ccfx.invokeCCFinderX(args);
		if (r != 0) {
			throw new CCFinderXInvocationException();
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile);
		Model newModel = new Model();
		try {
			newModel.readCloneDataFile(tempFile);
		} catch (IOException e) {
			throw new CCFinderXInvocationException();
		}
		
		return newModel;
	}

	public Model fitScopeToFilesExceptFor(int[] selectedFiles) {
		int[] files = selectedFiles.clone();
		for (int i = 0; i < files.length; ++i) {
			files[i] = sourceFiles.get(selectedFiles[i]).id;
		}
		Arrays.sort(files);
		
		StringBuffer buf = new StringBuffer();
		int i = 0;
		while (i < files.length) {
			int p = i;
			while (p + 1 < files.length && files[p + 1] == files[p] + 1) {
				++p;
			}
			buf.append(p != i ? String.format("%d-%d,", files[i], files[p])
					: String.format("%d,", files[i]));
			i = p + 1;
		}
		String tempFile = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		String[] args = { 
				"S",  //$NON-NLS-1$
				this.cloneDataFilePath,
				"-o", tempFile, //$NON-NLS-1$
				"-!f", buf.subSequence(0, buf.length() - 1).toString()  //$NON-NLS-1$
				};
		++tempFileCounter;
		CCFinderX ccfx = CCFinderX.theInstance;
		int r = ccfx.invokeCCFinderX(args);
		if (r != 0) {
			throw new CCFinderXInvocationException();
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile);
		Model newModel = new Model();
		try {
			newModel.readCloneDataFile(tempFile);
		} catch (IOException e) {
			throw new CCFinderXInvocationException();
		}
		
		return newModel;
	}

	public Model fitScopeToClones(String cloneIdList) throws CCFinderXInvocationException {
		String tempFile = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		String[] args = { 
				"S",  //$NON-NLS-1$
				this.cloneDataFilePath,
				"-o", tempFile, //$NON-NLS-1$
				"-ci", cloneIdList  //$NON-NLS-1$
				};
		++tempFileCounter;
		CCFinderX ccfx = CCFinderX.theInstance;
		int r = ccfx.invokeCCFinderX(args);
		if (r != 0) {
			throw new CCFinderXInvocationException();
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile);
		Model newModel = new Model();
		try {
			newModel.readCloneDataFile(tempFile);
		} catch (IOException e) {
			throw new CCFinderXInvocationException();
		}
		
		return newModel;
	}
	
	public Model fitScopeToClones(long[] selectedCloneSetIDs) {
		long[] ids = selectedCloneSetIDs.clone();
		Arrays.sort(ids);
		
		String tempFile2 = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		++tempFileCounter;
		{
		    BufferedWriter writer = null;
		    try {
		        writer = new BufferedWriter(new OutputStreamWriter(
		                                        new FileOutputStream(tempFile2))); 
				for (int i = 0; i < ids.length; ++i) {
					writer.write(String.valueOf(ids[i]));
					writer.newLine();
				}
		    } catch (FileNotFoundException e) {
				throw new CCFinderXInvocationException();
		    } catch (IOException e) {
				throw new CCFinderXInvocationException();
		    } finally {
		        try {
		            if (writer != null) {
		                writer.flush();
		                writer.close();
		            }
		        } catch (Exception e) {
		        }
		    }
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile2);
		
		String tempFile = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		++tempFileCounter;
		String[] args = { 
				"S",  //$NON-NLS-1$
				this.cloneDataFilePath,
				"-o", tempFile, //$NON-NLS-1$
				"-ci", tempFile2, //$NON-NLS-1$
				};
		CCFinderX ccfx = CCFinderX.theInstance;
		int r = ccfx.invokeCCFinderX(args);
		if (r != 0) {
			throw new CCFinderXInvocationException();
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile);
		Model newModel = new Model();
		try {
			newModel.readCloneDataFile(tempFile);
		} catch (IOException e) {
			throw new CCFinderXInvocationException();
		}

		return newModel;
	}
	
	public Model fitScopeToClonesExceptFor(long[] selectedCloneSetIDs) {
		long[] ids = selectedCloneSetIDs.clone();
		Arrays.sort(ids);
		
		String tempFile2 = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		++tempFileCounter;
		{
		    BufferedWriter writer = null;
		    try {
		        writer = new BufferedWriter(new OutputStreamWriter(
		                                        new FileOutputStream(tempFile2))); 
				for (int i = 0; i < ids.length; ++i) {
					writer.write(String.valueOf(ids[i]));
					writer.newLine();
				}
		    } catch (FileNotFoundException e) {
				throw new CCFinderXInvocationException();
		    } catch (IOException e) {
				throw new CCFinderXInvocationException();
		    } finally {
		        try {
		            if (writer != null) {
		                writer.flush();
		                writer.close();
		            }
		        } catch (Exception e) {
		        }
		    }
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile2);
		
		String tempFile = new Formatter(new StringBuilder()).format(SCOPE_TEMPFILE_FORMAT, tempFileCounter).toString();
		++tempFileCounter;
		String[] args = { 
				"S",  //$NON-NLS-1$
				this.cloneDataFilePath,
				"-o", tempFile, //$NON-NLS-1$
				"-!ci", tempFile2, //$NON-NLS-1$
				};
		CCFinderX ccfx = CCFinderX.theInstance;
		int r = ccfx.invokeCCFinderX(args);
		if (r != 0) {
			throw new CCFinderXInvocationException();
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFile);
		Model newModel = new Model();
		try {
			newModel.readCloneDataFile(tempFile);
		} catch (IOException e) {
			throw new CCFinderXInvocationException();
		}

		return newModel;
	}
	
	public long getCloneSetCount() {
		return cloneSetIDCount;
	}

	public long getMaxCloneSetID() {
		return maxCloneSetID;
	}

	public int getMaxFileID() {
		return maxFileID;
	}
	
	public long[] getAvailableCloneSetID(long[] cloneSetIDs) {
		ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
		bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

		try {
			TLongHashSet availables = new TLongHashSet();
			
			for (long id : cloneSetIDs) {
				if (id <= maxCloneSetID) {
					cloneSetDataStore.position(id * (4/* length */+ 4/* fileIndex */));
					bbuffer8.clear();
					cloneSetDataStore.read(bbuffer8);
					bbuffer8.rewind();
					int length = bbuffer8.getInt(0);
					if (length >= 0) {
						availables.add(id);
					}
				}
			}
			assert availables.size() <= cloneSetIDs.length;

			return availables.toArray();
		} catch (IOException e) {
			return null;
		}
	}
	
	public CloneSet[] getCloneSets(int maxSize) {
		ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
		bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

		try {
			cloneSetDataStore.position(0);
			ArrayList<CloneSet> list = new ArrayList<CloneSet>();
			long id = 0;
			while (id <= maxCloneSetID && list.size() < maxSize) {
				bbuffer8.clear();
				cloneSetDataStore.read(bbuffer8);
				bbuffer8.rewind();
				int length = bbuffer8.getInt(0);
				if (length < 0 && length != -1) {
					System.err.println("ng"); //$NON-NLS-1$
				}
				if (length >= 0) {
					CloneSet cs = new CloneSet(id, length);
					list.add(cs);
				}
				++id;
			}
			if (id <= maxCloneSetID && list.size() == maxSize) {
				CloneSet andMore = new CloneSet(-1, -1);
				list.add(andMore);
			}

			return list.toArray(new CloneSet[] {});
		} catch (IOException e) {
			return null;
		}
	}

	public CloneSet getCloneSet(long cloneSetID) {
		ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
		bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

		try {
			cloneSetDataStore.position(cloneSetID
					* (4/* length */+ 4/* fileIndex */));
			bbuffer8.clear();
			cloneSetDataStore.read(bbuffer8);
			bbuffer8.rewind();
			int length = bbuffer8.getInt();
			return new CloneSet(cloneSetID, length);
		} catch (IOException e) {
			return null;
		}
	}

	public ClonePair[] getClonePairsOfCloneSets(long[] cloneSetIDs) {
		long[] ids = cloneSetIDs.clone();
		Arrays.sort(ids);

		TIntArrayList fileToBeSearched = new TIntArrayList();

		{
			ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
			bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

			try {
				for (long cloneSetID : ids) {
					if (cloneSetID <= this.maxCloneSetID) {
						cloneSetDataStore.position(cloneSetID * (4/* length */+ 4/* fileIndex */));
						bbuffer8.clear();
						cloneSetDataStore.read(bbuffer8);
						int fileIndex = bbuffer8.getInt(4);
						fileToBeSearched.add(fileIndex);
					}
				}
			} catch (IOException e) {
				return null;
			}
		}

		ArrayList<ClonePair> clonePairs = new ArrayList<ClonePair>();
		TIntHashSet files = new TIntHashSet();
		while (!fileToBeSearched.isEmpty()) {
			TIntArrayList fileNewlyFound = new TIntArrayList();
			for (int i = 0; i < fileToBeSearched.size(); ++i) {
				int fileI = fileToBeSearched.get(i);
				files.add(fileI);
				int fileIndex = fileI;
				ClonePair[] pairs = getClonePairsOfFile(fileIndex);
				for (ClonePair p : pairs) {
					if (Arrays.binarySearch(ids, p.classID) >= 0) {
						clonePairs.add(p);
						int leftI = p.leftFile;
						boolean leftNewlyFound = files.add(leftI);
						if (leftNewlyFound) {
							fileNewlyFound.add(leftI);
						}
						if (p.rightFile != p.leftFile) {
							int rightI = p.rightFile;
							boolean rightNewlyFound = files.add(rightI);
							if (rightNewlyFound) {
								fileNewlyFound.add(rightI);
							}
						}
					}
				}
			}
			fileToBeSearched = fileNewlyFound;
		}
		
		return clonePairs.toArray(new ClonePair[] {});
	}
	
	public int getAnyRelatedFileOfCloneSet(long cloneSetID) {
		ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
		bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

		try {
			cloneSetDataStore.position(cloneSetID * (4/* length */+ 4/* fileIndex */));
			bbuffer8.clear();
			cloneSetDataStore.read(bbuffer8);
			int fileIndex = bbuffer8.getInt(4);
			return fileIndex;
		} catch (IOException e) {
			return -1; // error
		}
	}
	
	private int[] get_related_files(TIntArrayList fileToBeSearched, long[] cloneSets, boolean includes) {
		TIntHashSet files = new TIntHashSet();
		while (!fileToBeSearched.isEmpty()) {
			TIntArrayList fileNewlyFound = new TIntArrayList();
			for (int i = 0; i < fileToBeSearched.size(); ++i) {
				int fileI = fileToBeSearched.get(i);
				files.add(fileI);
				int fileIndex = fileI;
				ClonePair[] pairs = getClonePairsOfFile(fileIndex);
				for (ClonePair p : pairs) {
					if (includes && Arrays.binarySearch(cloneSets, p.classID) >= 0
							|| ! includes && Arrays.binarySearch(cloneSets, p.classID) < 0) {
						int leftI = p.leftFile;
						boolean leftNewlyFound = files.add(leftI);
						if (leftNewlyFound) {
							fileNewlyFound.add(leftI);
						}
						if (p.rightFile != p.leftFile) {
							int rightI = p.rightFile;
							boolean rightNewlyFound = files.add(rightI);
							if (rightNewlyFound) {
								fileNewlyFound.add(rightI);
							}
						}
					}
				}
			}
			fileToBeSearched = fileNewlyFound;
		}

		int[] fileIndices = files.toArray();
		Arrays.sort(fileIndices);

		return fileIndices;
	}
	
	public int[] getRelatedFilesOfCloneSetsExceptFor(long[] cloneSetIDs) {
		long[] ids = cloneSetIDs.clone();
		Arrays.sort(ids);

		TIntArrayList fileToBeSearched = new TIntArrayList();

		{
			ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
			bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

			try {
				for (long id = 0; id < maxCloneSetID; ++id) {
					if (Arrays.binarySearch(ids, id) < 0) {
						cloneSetDataStore.position(id * (4/* length */+ 4/* fileIndex */));
						bbuffer8.clear();
						cloneSetDataStore.read(bbuffer8);
						int length = bbuffer8.getInt(0);
						if (length >= 0) {
							int fileIndex = bbuffer8.getInt(4);
							fileToBeSearched.add(fileIndex);
						}
					}
				}
			} catch (IOException e) {
				return null;
			}
		}
		
		return get_related_files(fileToBeSearched, ids, false);
	}
	
	public int[] getRelatedFilesOfCloneSets(long[] cloneSetIDs) {
		long[] ids = cloneSetIDs.clone();
		Arrays.sort(ids);

		TIntArrayList fileToBeSearched = new TIntArrayList();

		{
			ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
			bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

			try {
				for (long cloneSetID : ids) {
					cloneSetDataStore.position(cloneSetID
							* (4/* length */+ 4/* fileIndex */));
					bbuffer8.clear();
					cloneSetDataStore.read(bbuffer8);
					int fileIndex = bbuffer8.getInt(4);
					fileToBeSearched.add(fileIndex);
				}
			} catch (IOException e) {
				return null;
			}
		}

		return get_related_files(fileToBeSearched, ids, true);
	}

	public int[] getCommonRelatedFilesOfCloneSets(long[] cloneSetIDs) {
		if (cloneSetIDs.length == 0) {
			return new int[0];
		}
		
		long[] ids = cloneSetIDs.clone();
		Arrays.sort(ids);

		TIntArrayList fileToBeSearched = new TIntArrayList();

		{
			ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
			bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

			try {
				for (long cloneSetID : ids) {
					cloneSetDataStore.position(cloneSetID
							* (4/* length */+ 4/* fileIndex */));
					bbuffer8.clear();
					cloneSetDataStore.read(bbuffer8);
					int fileIndex = bbuffer8.getInt(4);
					fileToBeSearched.add(fileIndex);
				}
			} catch (IOException e) {
				return null;
			}
		}

		{
			int[] fileIDs0 = get_related_files(fileToBeSearched, new long[] { ids[0] }, true);
			Arrays.sort(fileIDs0);
			for (long id : ids) {
				int[] fileIDs = get_related_files(fileToBeSearched, new long[] { id }, true);
				Arrays.sort(fileIDs);
				fileIDs0 = SetOperators.getUnion(fileIDs0, fileIDs);
				if (fileIDs0.length == 0) {
					return fileIDs0;
				}
			}
			return fileIDs0;
		}
	}

	public boolean[] getCloneSetIDMap() {
		boolean[] map = new boolean[(int)(maxCloneSetID + 1)];
		
		ByteBuffer bbuffer8 = ByteBuffer.allocate(8);
		bbuffer8.order(ByteOrder.LITTLE_ENDIAN);

		try {
			for (long id = 0; id <= maxCloneSetID; ++id) {
				cloneSetDataStore.position(id * (4/* length */+ 4/* fileIndex */));
				bbuffer8.clear();
				cloneSetDataStore.read(bbuffer8);
				int fileIndex = bbuffer8.getInt(4);
				if (fileIndex >= 0) {
					map[(int)id] = true;
				}
			}
		} catch (IOException e) {
			return null;
		}
		
		return map;
	}
	
	public String getCommonFilePath() {
		return commonPath;
	}
	
	public boolean isFileListSorted() {
		return hasSortedFileList;
	}
}
