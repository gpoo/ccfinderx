package utility;

public class IntMinMax {
	public int min;
	public int max;
	public IntMinMax(int initValue) {
		this.min = initValue;
		this.max = initValue;
	}
	public IntMinMax(IntMinMax rhs) {
		this.min = rhs.min;
		this.max = rhs.max;
	}
	public void init(int newValue) {
		this.min = newValue;
		this.max = newValue;
	}
	public void update(int newValue) {
		if (newValue < min) {
			min = newValue;
		}
		if (newValue > max) {
			max = newValue;
		}
	}
}
