package model;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;

import res.Messages;

import utility.StringUtil;
import utility.TemporaryFileManager;

public class ClonesetMetricModel {
	private static final String[] predefinedMetricNames = new String[] {
		"LEN", "POP", "NIF", "RAD", "RNR", "TKS", "LOOP", "COND", "McCabe"
	};
	
	public static String fieldIDToName(int field) throws IndexOutOfBoundsException {
		return predefinedMetricNames[field];
	}
	
	public static int fieldNameToID(String name) throws IndexOutOfBoundsException {
		for (int field = 0; field < predefinedMetricNames.length; ++field) {
			if (predefinedMetricNames[field].equals(name)) {
				return field;
			}
		}
		throw new IndexOutOfBoundsException();
	}
	
	public static String[] getFieldNames() {
		return predefinedMetricNames.clone();
	}
	
	private static final String GEMXCLONESETMETRIC_TEMPFILE = TemporaryFileManager.createTemporaryFileName();

	private FileChannel cloneSetDataStore = null;
	
	private long maxCloneSetID;
	private long cloneSetIDCount;
	private int fields;
	private boolean[] isFloatingPoints;
	private String[] metricNames;
	private MetricsSummaryData summaryData;
	
	public void readCloneSetMetricFile(String path, long maxCloneSetID) throws DataFileReadError, IOException {
		this.maxCloneSetID = maxCloneSetID;
		cloneSetIDCount = 0;
		
		{
			File f = new File(GEMXCLONESETMETRIC_TEMPFILE);
			f.deleteOnExit();
			RandomAccessFile raFile = new RandomAccessFile(f, "rw"); //$NON-NLS-1$
			cloneSetDataStore = raFile.getChannel();
			cloneSetDataStore.truncate(0);
		}
		
		BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(path), "UTF8")); //$NON-NLS-1$
		String line;
		line = reader.readLine();
		String supposedTitleLine = "CID" + "\t" + StringUtil.join(ClonesetMetricModel.getFieldNames(), "\t");
		if (! line.equals(supposedTitleLine)) { //$NON-NLS-1$
			throw new DataFileReadError("invalid clone-set metric file"); //$NON-NLS-1$
		}
		
		String[] ss = StringUtil.split(line, '\t');
		fields = ss.length - 1;
		metricNames = new String[fields];
		for (int i = 0; i < fields; ++i) {
			metricNames[i] = ss[i + 1];
		}
		isFloatingPoints = new boolean[fields];
		for (int i = 0; i < fields; ++i) {
			isFloatingPoints[i] = false;
		}
		
		ByteBuffer bbuffer = ByteBuffer.allocate(8/* size of double */ * fields);
		bbuffer.order(ByteOrder.LITTLE_ENDIAN);
		{
			ByteBuffer clearBuffer = ByteBuffer.allocate(8/* size of double */ * fields);
			clearBuffer.order(ByteOrder.LITTLE_ENDIAN);
			clearBuffer.putDouble(-1.0);
			for (int i = 1; i < fields; ++i) {
				clearBuffer.putDouble(0.0);
			}
			long pos = 0;
			for (long d = 0; d < maxCloneSetID + 1; ++d) {
				cloneSetDataStore.position(pos);
				clearBuffer.rewind();
				cloneSetDataStore.write(clearBuffer);
				pos += 8 * fields;
			}
		}
		double[] values = new double[fields];
		while (true) {
			line = reader.readLine();
			if (line == null) {
				throw new DataFileReadError(Messages.getString("gemx.CloneSetMetricModel.S_INVALID_CLONE_SET_METRIC_FILE")); //$NON-NLS-1$
			}
			String[] subs = StringUtil.split(line, '\t');
			if (subs[0].equals("ave.")) { //$NON-NLS-1$
				break;
			}
			if (subs.length != fields + 1) {
				throw new DataFileReadError(Messages.getString("gemx.CloneSetMetricModel.S_INVALID_CLONE_SET_METRIC_FILE")); //$NON-NLS-1$
			}
			long cloneSetID = Long.parseLong(subs[0]);
			if (! (0 <= cloneSetID && cloneSetID <= maxCloneSetID)) {
				throw new DataFileReadError(Messages.getString("gemx.CloneSetMetricModel.S_INVALID_CLONE_SET_ID_IN_CLONE_SET_METRIC_FILE")); //$NON-NLS-1$
			}
			++cloneSetIDCount;
			for (int i = 0; i < fields; ++i) {
				double v = Double.parseDouble(subs[i + 1]);
				if (Math.floor(v) != v) {
					isFloatingPoints[i] = true;
				}
				values[i] = v;
			}
			
			// write data
			long csPosition = cloneSetID * 8/* size of double */ * fields;
			bbuffer.clear();
			cloneSetDataStore.position(csPosition);
			cloneSetDataStore.read(bbuffer);
			double clen = bbuffer.getDouble(0);
			if (clen == -1.0) {
				bbuffer.clear();
				for (int i = 0; i < fields; ++i) {
					bbuffer.putDouble(values[i]);
				}
				bbuffer.rewind();
				cloneSetDataStore.position(csPosition);
				cloneSetDataStore.write(bbuffer);
			}
		}
		summaryData = new MetricsSummaryData();
		summaryData.read(fields, reader, line);
		
		reader.close();
	}
	
	public int getFieldCount() {
		return fields;
	}
	
	long getCloneSetIDCount() {
		return cloneSetIDCount;
	}
	
	long getMaxCloneSetID() {
		return maxCloneSetID;
	}
	
	public double getNthMetricDataOfCloneSet(long cloneSetID, int fieldIndex) throws IndexOutOfBoundsException {
		if (cloneSetID > maxCloneSetID) {
			throw new IndexOutOfBoundsException();
		}
		if (! (0 <= fieldIndex && fieldIndex < fields)) {
			throw new IndexOutOfBoundsException();
		}
		
		ByteBuffer bbuffer = ByteBuffer.allocate(8/* size of double */);
		bbuffer.order(ByteOrder.LITTLE_ENDIAN);
		
		try {
			long csPosition = (cloneSetID * fields + fieldIndex) * 8/* size of double */;
			
			bbuffer.clear();
			cloneSetDataStore.position(csPosition);
			cloneSetDataStore.read(bbuffer);
			bbuffer.rewind();
			
			double value = bbuffer.getDouble(0);
			return value;
		} catch (IOException e) {
			return 0.0;
		}
	}
	
	private double[] getMetricDataOfCloneSet_i(long cloneSetID) throws IndexOutOfBoundsException {
		if (cloneSetID > maxCloneSetID) {
			throw new IndexOutOfBoundsException();
		}
		
		ByteBuffer bbuffer = ByteBuffer.allocate(8/* size of double */ * fields);
		bbuffer.order(ByteOrder.LITTLE_ENDIAN);
		
		try {
			long csPosition = cloneSetID * 8/* size of double */ * fields;
			
			bbuffer.clear();
			cloneSetDataStore.position(csPosition);
			cloneSetDataStore.read(bbuffer);
			bbuffer.rewind();
			
			double[] values = new double[fields];
			for (int i = 0; i < fields; ++i) {
				values[i] = bbuffer.getDouble(i * 8/* size of double */);
			}
			
			return values;
		} catch (IOException e) {
			double[] values = new double[fields];
			return values;
		}
	}
	
	public double[] getMetricDataOfCloneSet(long cloneSetID) throws IndexOutOfBoundsException {
		return getMetricDataOfCloneSet_i(cloneSetID);
	}
	
	public void dispose() {
		if (cloneSetDataStore != null) {
			try {
				cloneSetDataStore.close();
			} catch (IOException e) {
				// can do nothing
			}
		}
	}

	public boolean isFlotingPoint(int metricIndex) {
		assert 0 <= metricIndex && metricIndex < isFloatingPoints.length;
		return isFloatingPoints[metricIndex];
	}
	
	public boolean[] getFlotingPointValueMap() {
		return isFloatingPoints.clone();
	}
	
	public String getMetricName(int metricIndex) {
		assert 0 <= metricIndex && metricIndex < metricNames.length;
		return metricNames[metricIndex];
	}
	
	public String[] getMetricNames() {
		return metricNames.clone();
	}

	public MetricsSummaryData getSummaryData() {
		return summaryData.clone();
	}
	
	public double getNthRelativeMetricDataOfCloneSet(long cloneSetID, int fieldIndex) throws IndexOutOfBoundsException {
		double value = getNthMetricDataOfCloneSet(cloneSetID, fieldIndex);
		return summaryData.toRelativeValue(fieldIndex, value);
	}
	
	public double[] getRelativeMetricDataOfCloneSet(long cloneSetID) throws IndexOutOfBoundsException {
		if (! (0 <= cloneSetID && cloneSetID <= maxCloneSetID)) {
			throw new IndexOutOfBoundsException();
		}
		double[] values = getMetricDataOfCloneSet_i(cloneSetID);
		double[] vs = new double[fields];
		for (int i = 0; i < fields; ++i) {
			vs[i] = summaryData.toRelativeValue(i, values[i]);
		}
		return vs;
	}
}
