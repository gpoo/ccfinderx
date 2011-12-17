package model;

import java.io.BufferedReader;
import java.io.IOException;

import res.Messages;
import utility.StringUtil;

public class MetricsSummaryData {
	private Double[] totalValues;
	private double[] aveValues;
	private double[] minValues;
	private double[] maxValues;
	
	public MetricsSummaryData clone() {
		MetricsSummaryData d = new MetricsSummaryData();
		d.totalValues = this.totalValues.clone();
		d.aveValues = this.aveValues.clone();
		d.minValues = this.minValues.clone();
		d.maxValues = this.maxValues.clone();
		return d;
	}
	
	public void read(int countOfFields, BufferedReader reader, String prefetchedLine) throws IOException, DataFileReadError {
		String line = prefetchedLine;
		if (line == null) {
			line = reader.readLine();
		}

		final String statisticFieldNames[] = new String[] { "total", "ave.", "min.", "max." };
		{
			if (line == null) {
				line = reader.readLine();
				if (line == null) {
					throw new DataFileReadError("Summary values missing");
				}
			}
			String[] subs = StringUtil.split(line, '\t');
			if (subs[0].equals(statisticFieldNames[0])) {
				if (subs.length != countOfFields + 1) {
					throw new DataFileReadError("invalid file metric file"); //$NON-NLS-1$
				}
				if (! subs[0].equals(statisticFieldNames[0])) {
					throw new DataFileReadError(Messages.getString("gemx.CloneSetMetricModel.S_SUMMARY_VALUES_MISSING")); //$NON-NLS-1$
				}
				Double[] vs = utility.DoublesParser.parseDoublesNullable(subs, 1, subs.length);
				totalValues = vs;
				line = null;
			} else {
				totalValues = new Double[subs.length - 1];
			}
		}
		for (int si = 1; si < 4; ++si) {
			if (line == null) {
				line = reader.readLine();
				if (line == null) {
					throw new DataFileReadError("Summary values missing");
				}
			}
			String[] subs = StringUtil.split(line, '\t');
			if (subs.length != countOfFields + 1) {
				throw new DataFileReadError("invalid file metric file"); //$NON-NLS-1$
			}
			if (! subs[0].equals(statisticFieldNames[si])) {
				throw new DataFileReadError(Messages.getString("gemx.CloneSetMetricModel.S_SUMMARY_VALUES_MISSING")); //$NON-NLS-1$
			}
			double[] vs = utility.DoublesParser.parseDoubles(subs, 1, subs.length);
			switch (si) {
			case 1:
				aveValues = vs;
				break;
			case 2:
				minValues = vs;
				break;
			case 3:
				maxValues = vs;
				break;
			}
			line = null;
		}
	}
	
	public Double[] getTotalValues() {
		return totalValues.clone();
	}
	
	public double[] getAverageValues() {
		return aveValues.clone();
	}
	
	public double[] getMinValues() {
		return minValues.clone();
	}
	
	public double[] getMaxValues() {
		return maxValues.clone();
	}
	
	public double[] getDepths() {
		int countOfFields = totalValues.length;
		double[] vs = new double[countOfFields];
		for (int i = 0; i < countOfFields; ++i) {
			vs[i] = maxValues[i] - minValues[i];
			if (vs[i] == 0.0) {
				vs[i] = 1.0;
			}
		}
		return vs;
	}
	
	public double toRelativeValue(int fieldIndex, double value) {
		double depth = maxValues[fieldIndex] - minValues[fieldIndex];
		if (depth == 0.0) {
			return 0.5;
		} else {
			return (value - minValues[fieldIndex]) / depth;
		}
	}
}

