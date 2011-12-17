import gemx.Main;
import gemx.WidgetsFactory;

public class GemXMain {
	public static void main(String[] args) {
		final WidgetsFactory widgetsFactory = new WidgetsFactory();
		if (args.length >= 1 && args[0].equals("--execution-module-directory-debug")) {
			utility.ExecutionModuleDirectory.setDebugMode(true);
			String[] args2 = new String[args.length - 1];
			for (int i = 0; i < args2.length; ++i) {
				args2[i] = args[i + 1];
			}
			args = args2;
		}
		ccfinderx.CCFinderX.theInstance.setModuleDirectory(utility.ExecutionModuleDirectory.get());
		Main.main(args, widgetsFactory);
	}
}
