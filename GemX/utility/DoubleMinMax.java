package utility;

public class DoubleMinMax {
	public double min;
	public double max;
	public DoubleMinMax(double initValue) {
		this.min = initValue;
		this.max = initValue;
	}
	public DoubleMinMax(IntMinMax rhs) {
		this.min = rhs.min;
		this.max = rhs.max;
	}
	public void init(double newValue) {
		this.min = newValue;
		this.max = newValue;
	}
	public void update(double newValue) {
		if (newValue < min) {
			min = newValue;
		}
		if (newValue > max) {
			max = newValue;
		}
	}
}
