package gemx;

import gemx.dialogs.*;
import gemx.scatterplothelper.*;

import java.io.*;
import java.util.*;

import model.*;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.dnd.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import customwidgets.*;

import utility.ExecCommandline;
import utility.FileUtils;
import utility.ArrayUtil;
import utility.Picosel;
import utility.TemporaryFileManager;
import utility.TemporaryMouseCursorChanger;

import ccfinderx.CCFinderX;

import res.Messages;

class CCFinderXHelper {
	private String[] preprocessorList;
	private String[] preprocessorHavingExtensionList;
	private final String tempFileName1 = TemporaryFileManager.createTemporaryFileName();
	private MainWindow mainWindow;

	public CCFinderXHelper(MainWindow mainWindow) {
		this.mainWindow = mainWindow;
	}
	
	public String[] getPreprocessScriptList() {
		if (preprocessorList != null) {
			return preprocessorList.clone();
		}

		int r = CCFinderX.theInstance.invokeCCFinderX(new String[] { "F", "-p", "-o", tempFileName1 }); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
		if (r != 0) {
			mainWindow.showErrorMessage("error in invocation of ccfx."); //$NON-NLS-1$
			return null;
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFileName1);

		ArrayList<String> preprocessors;
		try {
			preprocessors = utility.FileUtils.readLines(tempFileName1);
		} catch (IOException e) {
			mainWindow.showErrorMessage(Messages.getString("gemx.MainWindow.S_FILE_IO_ERROR_IN_READING_PREPROCESSOR_LIST")); //$NON-NLS-1$
			return null;
		}

		preprocessorList = preprocessors.toArray(new String[0]);
		return preprocessorList.clone();
	}

	public String[] getPreprocessScriptsRelatedToAnyExtensions() {
		if (preprocessorHavingExtensionList != null) {
			return preprocessorHavingExtensionList.clone();
		}

		String[] preprocessors = getPreprocessScriptList();

		ArrayList<String> preprocessorsHavingExtensions = new ArrayList<String>();
		for (int i = 0; i < preprocessors.length; ++i) {
			String prep = preprocessors[i];
			String[] extensions = getExtensionsRelatingToPreprocessScript(prep);
			if (extensions.length != 0) {
				preprocessorsHavingExtensions.add(prep);
			}
		}
		preprocessorHavingExtensionList = preprocessorsHavingExtensions.toArray(new String[0]);

		return preprocessorHavingExtensionList.clone();
	}

	public ArrayList<String> buildCcfxCommandLineForDetection(String preprocessScript, 
			String encodingName, String fileListPath, int minCloneLength, 
			int minTKS, int shaperLevel, boolean usePMatch, int chunkSize, int maxWorkerThreads,
			String[] preprocessFileDirectories) {
		ArrayList<String> options = new ArrayList<String>();

		assert preprocessScript != null;

		options.add("D"); //$NON-NLS-1$
		options.add(preprocessScript);
		options.addAll(Arrays.asList(new String[] { "-i", fileListPath })); //$NON-NLS-1$
		if (encodingName != null && encodingName.length() > 0) {
			options.addAll(Arrays.asList(new String[] { "-c", encodingName })); //$NON-NLS-1$
		}
		options.addAll(Arrays.asList(new String[] { "-b", String.valueOf(minCloneLength) })); //$NON-NLS-1$
		options.addAll(Arrays.asList(new String[] { "-t", String.valueOf(minTKS) })); //$NON-NLS-1$
		if (shaperLevel >= 0) {
			options.addAll(Arrays.asList(new String[] { "-s", String.valueOf(shaperLevel) })); //$NON-NLS-1$
		}
		options.addAll(Arrays.asList(new String[] { "-u", usePMatch ? "+" : "-" })); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
		for (String p : preprocessFileDirectories) {
			options.addAll(Arrays.asList(new String[] { "-n", p })); //$NON-NLS-1$
		}
		options.addAll(Arrays.asList(new String[] { "-k",  //$NON-NLS-1$
				chunkSize != 0 ? String.valueOf(chunkSize) + "M" : "0" })); //$NON-NLS-1$ //$NON-NLS-2$
		if (maxWorkerThreads > 0) {
			options.add("--threads=" + String.valueOf(maxWorkerThreads)); //$NON-NLS-1$
		}
		options.add("-v"); //$NON-NLS-1$

		return options;
	}
	
	public String[] getExtensionsRelatingToPreprocessScript(String preprocessorName) {
		int r = CCFinderX.theInstance.invokeCCFinderX(new String[] { "F", preprocessorName, "-e", "-o", tempFileName1 }); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
		if (r != 0) {
			mainWindow.showErrorMessage("error in invocation of ccfx."); //$NON-NLS-1$
			return null;
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFileName1);

		ArrayList<String> extensions;
		try {
			extensions = utility.FileUtils.readLines(tempFileName1);
		}
		catch (IOException e) {
			mainWindow.showErrorMessage(Messages.getString("gemx.MainWindow.S_FILE_IO_ERROR_IN_READING_PREPROCESSOR_LIST")); //$NON-NLS-1$
			return null;
		}

		return extensions.toArray(new String[0]);
	}

	public void doPrescreening(String maskedFileList, 
			String fileListFile, String preprocessor, String encodingName,
			String[] preprocessFileDirectoires,
			int maxWorkerThreads) throws IOException {
		String exeDir = utility.ExecutionModuleDirectory.get();
		assert exeDir != null;

		// do preprocessing
		{
			ArrayList<String> options = new ArrayList<String>();
			options.add("D"); //$NON-NLS-1$
			options.add(preprocessor);
			if (preprocessFileDirectoires != null && preprocessFileDirectoires.length > 0) {
				for (String d : preprocessFileDirectoires) {
					options.addAll(Arrays.asList(new String[] { "-n", d })); //$NON-NLS-1$
				}
			}
			options.addAll(Arrays.asList(new String[] { "-p", "-i", fileListFile })); //$NON-NLS-1$ //$NON-NLS-2$
			if (encodingName != null && encodingName.length() > 0) {
				options.addAll(Arrays.asList(new String[] { "-c", encodingName })); //$NON-NLS-1$
			}
			if (maxWorkerThreads > 0) {
				options.add("--threads=" + String.valueOf(maxWorkerThreads)); //$NON-NLS-1$
			}
			options.add("-v"); //$NON-NLS-1$
			CCFinderX.theInstance.invokeCCFinderX(options.toArray(new String[0]));
		}

		String dirPath = exeDir;
		String scriptPath = dirPath + File.separator + "scripts" + File.separator + "prescreening.py"; //$NON-NLS-1$ //$NON-NLS-2$
		ArrayList<String> args = new ArrayList<String>();

		args.add(utility.PythonVersionChecker.thePythonInterpreterPath);
		args.add(scriptPath);
		args.add(preprocessor);
		args.addAll(Arrays.asList(new String[] { "-i", fileListFile })); //$NON-NLS-1$
		args.addAll(Arrays.asList(new String[] { "-o", maskedFileList })); //$NON-NLS-1$
		if (encodingName != null && encodingName.length() > 0) {
			args.addAll(Arrays.asList(new String[] { "-c", encodingName })); //$NON-NLS-1$
		}
		//options.add("-v"); //$NON-NLS-1$

		int retCode = ExecCommandline.execCommandline(args, true);
	}
}

public class MainWindow {
	public static final int SC_NONE = 0;
	public static final int SC_SCOPE_HISTORY_LIST = 1;
	public static final int SC_FILE_TABLE = 2;
	public static final int SC_CLONE_SET_TABLE = 3;
	public static final int SC_SCATTER_PLOT_PANE = 4;
	public static final int SC_SOURCE_PANE = 5;
	public static final int SC_SCRAPBOOK = 6;

	private final String theCloneDataFileName = TemporaryFileManager.getFileNameOnTemporaryDirectory("a.ccfxd");
	private final String tempFileName1 = TemporaryFileManager.createTemporaryFileName();
	private final String tempFileName2 = TemporaryFileManager.createTemporaryFileName();
	private final String tempFileName3 = TemporaryFileManager.createTemporaryFileName();
	private final Main main;

	private Shell shell;
	private Table scopeHistoryList;
	private FileTable fileTable;
	private ClonesetTable cloneSetTable;
	private ScatterPlotPane scatterPlotPane;
	private MultipleTextPane sourcePane;
	private MultipleTextPane theScrapbook;

	private CustomCTabFolder sessionFolder, listFolder, mapFolder;
	private CustomCTabFolder[] sashChildren;
	private int selectedSashChildIndex = -1;

	private Label statusBar;
	private Label preprocessorBar;
	private Label fileInfoBar;
	private Label cloneSetInfoBar;
	private customwidgets.Searchbox searchBox;

	private Model rootModel;
	private Model currentScope;
	private MetricModelsHolder metricModels;
	private ArrayList<ScopeHistoryItem> scopeHistory;

	private long[] currentSelectedCloneSetIDs;
	private int[] currentSelectedFileIndices;

	private WidgetsFactory widgetsFactory;

	public Clipboard clipboard;

	private final CCFinderXHelper ccfxhelper;

	public MainWindow(Main main, WidgetsFactory factory) {
		this.ccfxhelper	= new CCFinderXHelper(this);
		this.main = main;
		this.rootModel = new Model();
		currentScope = rootModel;
		this.metricModels = new MetricModelsHolder(this);
		this.widgetsFactory = factory;
	}

	public void dispose() {
		if (currentScope != rootModel) {
			currentScope.dispose();
		}
		rootModel.dispose();
		currentScope = null;
		metricModels.dispose();
		metricModels = null;
		clipboard.dispose();
		clipboard = null;
	}

	public Main getMain() {
		return main;
	}

	public void showErrorMessage(String message) {
		final MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_ERROR);
		mes.setText("Error - GemX"); //$NON-NLS-1$
		mes.setMessage(message);
		mes.open();
	}

	public int showConfirmationMessage(int status, String message) {
		final MessageBox mes = new MessageBox(shell, status);
		mes.setText("Confirmation - GemX"); //$NON-NLS-1$
		mes.setMessage(message);
		return mes.open();
	}

	private void do_save_clone_data_as() {
		if (scopeHistory == null || scopeHistory.size() == 0) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_NO_CLONE_DATA_TO_BE_SAVED")); //$NON-NLS-1$
			return;
		}

		FileDialog dialog = new FileDialog(shell, SWT.SAVE);
		String[] exts = new String[] { "*.ccfxd", "*.icetd" }; //$NON-NLS-1$ //$NON-NLS-2$
		String[] extNames = new String[] { Messages.getString("gemx.MainWindow.S_CLONE_DATA_FILE") }; //$NON-NLS-1$
		dialog.setFilterExtensions(exts);
		dialog.setFilterNames(extNames);

		dialog.open();

		String dir = dialog.getFilterPath();
		String files[] = dialog.getFileNames();
		if (files.length >= 2) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_SPECIFY_ONE_CLONE_DATA_FILE")); //$NON-NLS-1$
		} else if (files.length == 1) {
			String path = dir + java.io.File.separator + files[0];
			if (new File(path).exists()) {
				if (showConfirmationMessage(SWT.YES | SWT.NO | SWT.ICON_QUESTION,
						Messages.getString("gemx.MainWindow.S_THE_NAMED_FILE_ALREADY_EXISTS_OVERWRITE_IT_P")) != SWT.YES) { //$NON-NLS-1$
					return;
				}
			}
			try {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_WRITING_CLONE_DATA_FILE")); //$NON-NLS-1$

				ScopeHistoryItem shi = scopeHistory.get(scopeHistory.size() - 1);
				TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
				try {
					FileUtils.fileCopy(shi.cloneDataFile, path);
				} finally {
					tmcc.dispose();
				}
			} catch (java.io.IOException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FILE_IO_ERROR_IN_WRITING_CLONE_DATA_FILE") //$NON-NLS-1$
						+ path);
			} finally {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
			}
		}
	}

	private void do_save_file_list_as() {
		if (scopeHistory == null || scopeHistory.size() == 0) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_NO_CLONE_DATA_TO_BE_SAVED")); //$NON-NLS-1$
			return;
		}

		FileDialog dialog = new FileDialog(shell, SWT.SAVE);
		String[] exts = new String[] { "*.*" }; //$NON-NLS-1$
		String[] extNames = new String[] { Messages.getString("gemx.MainWindow.S_FILE_LIST") }; //$NON-NLS-1$
		dialog.setFilterExtensions(exts);
		dialog.setFilterNames(extNames);

		dialog.open();

		String dir = dialog.getFilterPath();
		String files[] = dialog.getFileNames();
		if (files.length >= 2) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_SPECIFY_ONE_FILE")); //$NON-NLS-1$
		} else if (files.length == 1) {
			String path = dir + java.io.File.separator + files[0];
			if (new File(path).exists()) {
				if (showConfirmationMessage(SWT.YES | SWT.NO | SWT.ICON_QUESTION,
						Messages.getString("gemx.MainWindow.S_THE_NAMED_FILE_ALREADY_EXISTS_OVERWRITE_IT_P")) != SWT.YES) { //$NON-NLS-1$
					return;
				}
			}
			
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_WRITING_FILE_LIST")); //$NON-NLS-1$
			try {
				ScopeHistoryItem shi = scopeHistory.get(scopeHistory.size() - 1);
				int r = CCFinderX.theInstance.invokeCCFinderX(new String[] { "P", shi.cloneDataFile, "-ln", "-o", path }); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
				if (r != 0) {
					showErrorMessage("error in invocation of ccfx."); //$NON-NLS-1$
					return;
				}
			} finally {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
			}
		} else {
			return;
		}
	}
	
	private void do_save_scatterplot_as() {
		if (this.scatterPlotPane == null)
			return;
		
		if (scopeHistory == null || scopeHistory.size() == 0) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_NO_CLONE_DATA_TO_BE_SAVED")); //$NON-NLS-1$
			return;
		}

		FileDialog dialog = new FileDialog(shell, SWT.SAVE);
		String[] exts = new String[] { "*.png" }; //$NON-NLS-1$
		String[] extNames = new String[] { "Png file (*.png)" }; //$NON-NLS-1$
		dialog.setFilterExtensions(exts);
		dialog.setFilterNames(extNames);

		dialog.open();

		String dir = dialog.getFilterPath();
		String files[] = dialog.getFileNames();
		if (files.length >= 2) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_SPECIFY_ONE_FILE")); //$NON-NLS-1$
		} else if (files.length == 1) {
			String path = dir + java.io.File.separator + files[0];
			if (new File(path).exists()) {
				if (showConfirmationMessage(SWT.YES | SWT.NO | SWT.ICON_QUESTION,
						Messages.getString("gemx.MainWindow.S_THE_NAMED_FILE_ALREADY_EXISTS_OVERWRITE_IT_P")) != SWT.YES) { //$NON-NLS-1$
					return;
				}
			}
			
			statusBar.setText("> Writing the scatter-plot image to a file"); //$NON-NLS-1$
			try {
					this.scatterPlotPane.saveImage(path);
			} catch (IOException e) {
				showErrorMessage("error in writing an image file"); //$NON-NLS-1$
			} finally {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
			}
		} else {
			return;
		}
		
		
	}

	private String lastDirectoryByDoDetectClones = null;
	private String lastPreprocessorByDoDetectClones = null;
	private int lastMinCloneLengthByDoDetectClones = -1;
	private int lastMinTKSByDoDetectClones = -1;
	private int lastShaperLevel = -1;
	private Boolean lastUsePMatch = null;
	private Boolean lastUsePrescreening = null;

	public void clearDetectConesOptions() {
		lastPreprocessorByDoDetectClones = null;
		lastMinCloneLengthByDoDetectClones = -1;
		lastMinTKSByDoDetectClones = -1;
		lastShaperLevel = -1;
		lastUsePMatch = null;
		lastUsePrescreening = null;
	}

	private void do_re_detect_clones() {
		try {
			if (scopeHistory == null || scopeHistory.size() == 0) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_NO_CLONE_DATA_TO_BE_SAVED")); //$NON-NLS-1$
				return;
			}

			String[] preprocessors = ccfxhelper.getPreprocessScriptList();
			if (preprocessors == null) {
				return;
			}
			String[] preprocessorHavingExtensions = ccfxhelper.getPreprocessScriptsRelatedToAnyExtensions();
			if (preprocessorHavingExtensions == null) {
				return;
			}
			CcfxDetectionOptions detectionOptions = currentScope.getDetectionOption();

			String path = tempFileName1;
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_WRITING_FILE_LIST")); //$NON-NLS-1$

			TemporaryFileManager.registerFileToBeRemoved(tempFileName1);

			ScopeHistoryItem shi = scopeHistory.get(scopeHistory.size() - 1);
			int r = CCFinderX.theInstance.invokeCCFinderX(new String[] { "P", shi.cloneDataFile, "-ln", "-o", path }); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
			if (r != 0) {
				showErrorMessage("error in invocation of ccfx."); //$NON-NLS-1$
				return;
			}

			int fileCount = currentScope.getFileCount();
			String preprocessor = currentScope.getPreprocessScript();

			// identify preprocess file directories
			String[] preprocessFileDirectories = detectionOptions.get("n"); //$NON-NLS-1$

			// set other options
			CcfxDetectionOptions ops = (CcfxDetectionOptions)detectionOptions;
			int minCloneLength = ops.getMinimumCloneLength();
			int minTKS = ops.getMinimumTokenSetSize();
			if (lastShaperLevel == -1) {
				lastShaperLevel = main.settingShaperLevel;
			}
			int shaperLevel = ops.getShaperLevel();
			boolean usePMatch = ops.isUseParameterUnification();
			boolean usePrescreening = main.settingUsePrescreening;
			{
				CloneReDetectionDialog dialog = new CloneReDetectionDialog(shell, CcfxOptionsPanel.PreprocessorForcingModeAlways);
				for (int i = 0; i < preprocessors.length; ++i) {
					dialog.addForcedPreprocessorItem(preprocessors[i]);
				}
				dialog.setText(Messages.getString("gemx.MainWindow.S_CLONE_RE_DETECTION")); //$NON-NLS-1$
				dialog.setForcedPreprocessor(preprocessor);
				dialog.setIdentifiedSourceFiles(fileCount);
				dialog.setMinimumCloneLength(minCloneLength);
				dialog.setMinimumTKS(minTKS);
				dialog.setShaperLevel(shaperLevel);
				dialog.setUsePMatch(usePMatch);
				dialog.setUsePrescreening(usePrescreening);
				int result = dialog.open();
				if (result == SWT.CANCEL) {
					return;
				}
				int ml = dialog.getMinimumCloneLength();
				if (ml != minCloneLength) {
					minCloneLength = ml;
				}
				lastMinCloneLengthByDoDetectClones = ml;
				int mt = dialog.getMinimumTKS();
				if (mt != minTKS) {
					minTKS = mt;
				}
				int sl = dialog.getShaperLevel();
				if (sl != shaperLevel) {
					shaperLevel = sl;
				}
				boolean up = dialog.isUsePMatch();
				if (up != usePMatch) {
					usePMatch = up;
				}
				preprocessor = dialog.getForcedPreprocessor();
				boolean ur = dialog.isUsePrescreening();
				if (ur != usePrescreening) {
					usePrescreening = ur;
				}
			}

			String encodingName = main.settingEncoding;
			
			
			String fileListName = null;
			if (usePrescreening) {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_PRESCREENING")); //$NON-NLS-1$
				TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
				try {
					ccfxhelper.doPrescreening(tempFileName2, tempFileName1, preprocessor, encodingName,
							preprocessFileDirectories, main.settingMaxWorkerThreads);
					usePrescreening = true;
					fileListName = tempFileName2;
				} catch (IOException e) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_ERROR_IN_INVOKING_PRESCREENING_FILTER"));  //$NON-NLS-1$
					return;
				} finally {
					tmcc.dispose();
				}
			} else {
				fileListName = tempFileName1;
			}

			// detect clones
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_DETECTING_CLONES")); //$NON-NLS-1$
			if (! moveToBackupFile(theCloneDataFileName)) { //$NON-NLS-1$
				return;
			}
			{
				ArrayList<String> options = ccfxhelper.buildCcfxCommandLineForDetection(preprocessor, encodingName,
						fileListName, minCloneLength, minTKS, shaperLevel, usePMatch, 
						main.settingChunkSize, main.settingMaxWorkerThreads, preprocessFileDirectories);
				options.addAll(Arrays.asList(new String[] { "-o", theCloneDataFileName }));
				if (usePrescreening) {
					options.addAll(Arrays.asList(new String[] { "-mr", "masked" })); //$NON-NLS-1$ //$NON-NLS-2$
				}

				TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
				try {
					CCFinderX.theInstance.invokeCCFinderX(options.toArray(new String[0]));
				} finally {
					tmcc.dispose();
				}
			}
			TemporaryFileManager.registerFileToBeRemoved(fileListName);
		}
		finally {
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}

		// update clone data
		do_open_clone_data_file_i(theCloneDataFileName);

		if (this.cloneSetTable.isCloneSetSizeTooLarge()) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_DETECTED_CLONES_TOO_MANY")); //$NON-NLS-1$
		}

		if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
			this.add_both_file_and_clone_set_metrics();
		} else {
			if (main.settingCalcFileMetricAlways) {
				add_file_metrics();
			}
			if (main.settingCalcCloneMetricAlways) {
				add_clone_set_metrics();
			}
		}
	}
	
	private boolean moveToBackupFile(String fileName) {
		if (fileName.endsWith(".bak")) { //$NON-NLS-1$
			throw new AssertionError();
		}
		
		File original = new File(fileName);
		if (! original.exists()) {
			return true; // no need to backup
		}
		
		String backupFileName = fileName + ".bak"; //$NON-NLS-1$
		File f = new File(backupFileName);
		int i = 0;
		while (f.exists()) {
			++i;
			f = new File(fileName + ".bak" + String.format("%03d", i)); //$NON-NLS-1$ //$NON-NLS-2$
		}
		boolean done = false;
		try {
			if (original.renameTo(f)) {
				done = true;
			}
		} catch (SecurityException e) {
		}
		if (! done) {
			int r = this.showConfirmationMessage(SWT.OK | SWT.CANCEL,
					String.format(Messages.getString("gemx.MainWindow.S_FAIL_TO_MAKE_BACKUP_FILE_WILL_YOU_DELETE_IT"), fileName)); //$NON-NLS-1$
			if (r == SWT.OK) {
				original.delete();
				done = true;
			}
		}
		return done;
	}

	private void do_detect_clones() {
		try {
			// determin preprocessor
			String preprocessor = null;
			String[] directories;
			
			{
				PreprocessScriptAndDirectoriesDialog dialog = new PreprocessScriptAndDirectoriesDialog(shell);
				dialog.setText(Messages.getString("gemx.MainWindow.S_CLONEDETECTION_STEP1"));   //$NON-NLS-1$

				if (lastDirectoryByDoDetectClones != null) {
					dialog.setLastDirectory(lastDirectoryByDoDetectClones);
				}
				
				String[] preprocessors = ccfxhelper.getPreprocessScriptsRelatedToAnyExtensions();
				if (preprocessors == null) {
					return;
				}
				for (int i = 0; i < preprocessors.length; ++i) {
					dialog.addPreprocessScript(preprocessors[i]);
				}
				if (lastPreprocessorByDoDetectClones == null || lastPreprocessorByDoDetectClones.length() == 0) {
					lastPreprocessorByDoDetectClones = main.settingPreprocessor;
				}
				int prepIndex = -1;
				for (int i = 0; i < preprocessors.length; ++i) {
					if (preprocessors[i].equals(lastPreprocessorByDoDetectClones)) {
						prepIndex = i;
						break; // for i
					}
				}
				if (prepIndex >= 0) {
					dialog.setPreprocessScript(preprocessors[prepIndex]);
				}
				else {
					dialog.setPreprocessScript(preprocessors[0]);
				}
				
				if (! dialog.open()) {
					return;
				}
				
				preprocessor = dialog.getPreprocessScript();
				lastPreprocessorByDoDetectClones = preprocessor;
				
				if (preprocessor == null || preprocessor.length() == 0) {
					return;
				}
				
				directories = dialog.getDirectories();
				if (directories == null || directories.length == 0) {
					return;
				}
			}

			// identify source files
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_SEARCHING_SOURCE_FILES")); //$NON-NLS-1$
			{
				ArrayList<String> args = new ArrayList<String>();
				args.add("F");  //$NON-NLS-1$
				args.add(preprocessor);
				args.addAll(Arrays.asList(new String[] { "-o", tempFileName1 }));  //$NON-NLS-1$
				for (String a : directories) {
					args.add(a);
				}
				
				TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
				try {
					int r = CCFinderX.theInstance.invokeCCFinderX(args.toArray(new String[0]));
					if (r != 0) {
						showErrorMessage("error in invocation of ccfx."); //$NON-NLS-1$
						return;
					}
				} finally {
					tmcc.dispose();
				}
				TemporaryFileManager.registerFileToBeRemoved(tempFileName1);
			}
			ArrayList<String> files;
			try {
				files = utility.FileUtils.readLines(tempFileName1);
			}
			catch (IOException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FILE_IO_ERROR_IN_READING_FILE_LIST")); //$NON-NLS-1$
				return;
			}
			if (files.size() == 0) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_NO_SOURCE_FILES_ARE_FOUND")); //$NON-NLS-1$
				return;
			}
			
			// identify preprocess file directories
			String[] preprocessFileDirectories = null;
			if (main.settingUsePreprocessCache) {
				preprocessFileDirectories = directories.clone();
			}

			// set other options
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_WAITING"));  //$NON-NLS-1$
			if (lastMinCloneLengthByDoDetectClones == -1) {
				lastMinCloneLengthByDoDetectClones = main.settingMinimumCloneLength;
			}
			int minCloneLength = lastMinCloneLengthByDoDetectClones;
			if (lastMinTKSByDoDetectClones == -1) {
				lastMinTKSByDoDetectClones = main.settingMinimumTKS;
			}
			int minTKS = lastMinTKSByDoDetectClones;
			if (lastShaperLevel == -1) {
				lastShaperLevel = main.settingShaperLevel;
			}
			int shaperLevel = lastShaperLevel;
			if (lastUsePMatch == null) {
				lastUsePMatch = new Boolean(main.settingUsePMatch);
			}
			boolean usePMatch = lastUsePMatch.booleanValue();
			if (lastUsePrescreening == null) {
				lastUsePrescreening = new Boolean(main.settingUsePrescreening);
			}
			boolean usePrescreening = lastUsePrescreening.booleanValue();
			{
				CloneDetectionOptionDialog dialog = new CloneDetectionOptionDialog(shell);
				dialog.setText(Messages.getString("gemx.MainWindow.S_CLONEDETECTION_STEP3"));   //$NON-NLS-1$
				dialog.setPreprocessor(preprocessor);
				dialog.setIdentifiedSourceFiles(files.size());
				dialog.setMinimumCloneLength(minCloneLength);
				dialog.setMinimumTKS(minTKS);
				dialog.setShaperLevel(shaperLevel);
				dialog.setUsePMatch(usePMatch);
				dialog.setUsePrescreening(usePrescreening);
				int result = dialog.open();
				if (result == SWT.CANCEL) {
					return;
				}
				int ml = dialog.getMinimumCloneLength();
				if (ml != minCloneLength) {
					minCloneLength = ml;
				}
				lastMinCloneLengthByDoDetectClones = ml;
				int mt = dialog.getMinimumTKS();
				if (mt != minTKS) {
					minTKS = mt;
				}
				int sl = dialog.getShaperLevel();
				if (sl != shaperLevel) {
					shaperLevel = sl;
				}
				boolean up = dialog.isUsePMatch();
				if (up != usePMatch) {
					usePMatch = up;
				}
				boolean ur = dialog.isUsePrescreening();
				if (ur != usePrescreening) {
					usePrescreening = ur;
				}
			}

			{
				BufferedWriter writer = null;
				try {
					writer = new BufferedWriter(new FileWriter(tempFileName1));
					for (int i = 0; i < files.size(); ++i) {
						writer.write(files.get(i));
						writer.newLine();
					}
				} catch (FileNotFoundException e) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_FILE_IO_ERROR_IN_INVOKING_CCFX")); //$NON-NLS-1$
					return;
				} catch (IOException e) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_FILE_IO_ERROR_IN_INVOKING_CCFX")); //$NON-NLS-1$
					return;
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
			
			String encodingName = main.settingEncoding;
			String fileListName = null;
			if (usePrescreening) {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_PRESCREENING"));  //$NON-NLS-1$
				TemporaryMouseCursorChanger wcm = new TemporaryMouseCursorChanger(shell);
				try {
					ccfxhelper.doPrescreening(tempFileName2, tempFileName1, preprocessor, encodingName,
							preprocessFileDirectories, main.settingMaxWorkerThreads);
					usePrescreening = true;
					fileListName = tempFileName2;
				} catch (IOException e) {
					showErrorMessage("error in invoking prescreening filter"); //$NON-NLS-1$
					return;
				} finally {
					wcm.dispose();
				}
			} else {
				fileListName = tempFileName1;
			}

			// detect clones
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_DETECTING_CLONES")); //$NON-NLS-1$
			if (! moveToBackupFile(theCloneDataFileName)) {  //$NON-NLS-1$
				return;
			}
			{
				ArrayList<String> options = ccfxhelper.buildCcfxCommandLineForDetection(preprocessor, encodingName,
						fileListName, minCloneLength, minTKS, shaperLevel, usePMatch, 
						main.settingChunkSize, main.settingMaxWorkerThreads,
						preprocessFileDirectories);
				options.addAll(Arrays.asList(new String[] { "-o", theCloneDataFileName }));
				if (usePrescreening) {
					options.addAll(Arrays.asList(new String[] { "-mr", "masked" }));   //$NON-NLS-1$ //$NON-NLS-2$
				}

				TemporaryMouseCursorChanger wcm = new TemporaryMouseCursorChanger(shell);
				try {
					CCFinderX.theInstance.invokeCCFinderX(options.toArray(new String[0]));
				} finally {
					wcm.dispose();
				}
			}
			TemporaryFileManager.registerFileToBeRemoved(fileListName);
		}
		finally {
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
			update_info_bars();
		}

		// update clone data
		do_open_clone_data_file_i(theCloneDataFileName); //$NON-NLS-1$

		if (this.cloneSetTable.isCloneSetSizeTooLarge()) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_DETECTED_CLONES_TOO_MANY")); //$NON-NLS-1$
		}

		if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
			this.add_both_file_and_clone_set_metrics();
		} else {
			if (main.settingCalcFileMetricAlways) {
				add_file_metrics();
			}
			if (main.settingCalcCloneMetricAlways) {
				add_clone_set_metrics();
			}
		}
	}

	private static String[] lookupPreprocessFileDirectories(String[] sourceFiles) {
		HashSet<String> dirs = new HashSet<String>();
		for (String p : sourceFiles) {
			File d = new File(p).getParentFile();
			while (d != null) {
				dirs.add(d.toString());
				d = d.getParentFile();
			}
		}
		
		ArrayList<String> preprocessFileDirectories = new ArrayList<String>();
		for (String d : dirs) {
			File p = new File(d + File.separator + ".ccfxprepdir");  //$NON-NLS-1$
			if (p.exists() && p.isDirectory()) {
				preprocessFileDirectories.add(p.toString());
			}
		}
		
		return preprocessFileDirectories.toArray(new String[0]);
	}
	
	public void doDetectClonesFromFileList(boolean isFileListInClipboard) {
		try {
			String fileListPath = ""; //$NON-NLS-1$
			if (! isFileListInClipboard) {
				FileDialog dialog = new FileDialog(shell, SWT.OPEN);
				String[] exts = new String[] { "*.*" }; //$NON-NLS-1$
				String[] extNames = new String[] { Messages.getString("gemx.MainWindow.S_FILE_LIST") }; //$NON-NLS-1$
				dialog.setFilterExtensions(exts);
				dialog.setFilterNames(extNames);
				dialog.setText(Messages.getString("gemx.MainWindow.S_CLONEDETECTION_FROMFILELIST_STEP1")); //$NON-NLS-1$

				dialog.open();

				String dir = dialog.getFilterPath();
				String files[] = dialog.getFileNames();
				if (files.length >= 2) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_SPECIFY_ONE_FILE_LIST")); //$NON-NLS-1$
					return;
				} else if (files.length == 1) {
					fileListPath = dir + java.io.File.separator + files[0];
				} else {
					return;
				}
			} else {
				String data = (String)clipboard.getContents(
						TextTransfer.getInstance());
				if (data == null) {
					return;
				}
				BufferedWriter writer = null;
				try {
					writer = new BufferedWriter(new FileWriter(tempFileName3));
					writer.write(data);
				} catch (FileNotFoundException e) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_CANNOT_WRITE_TO_A_TEMPORARY_FILE")); //$NON-NLS-1$
					return;
				} catch (IOException e) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_CANNOT_WRITE_TO_A_TEMPORARY_FILE")); //$NON-NLS-1$
					return;
				} finally {
					try {
						if (writer != null) {
							writer.flush();
							writer.close();
						}
					} catch (Exception e) {
					}
				}				
				TemporaryFileManager.registerFileToBeRemoved(tempFileName3);
				fileListPath = tempFileName3;
			}
			
			// identify preprocess file directories
			String[] preprocessFileDirectories = null;
			if (main.settingUsePreprocessCache) {
				ArrayList<String> files = new ArrayList<String>();
				try {
					ArrayList<String> lines = utility.FileUtils.readLines(fileListPath);
					for (String line : lines) {
						int p = line.indexOf('\t');
						if (p >= 0) {
							String fn = line.substring(p + 1);
							files.add(fn);
						} else {
							files.add(line);
						}
					}
				} catch (IOException e) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_IO_ERROR_IN_READING_FILELIST"));  //$NON-NLS-1$
					return;
				}
				preprocessFileDirectories = lookupPreprocessFileDirectories(files.toArray(new String[0]));
			}

			// set other options
			if (lastMinCloneLengthByDoDetectClones == -1) {
				lastMinCloneLengthByDoDetectClones = main.settingMinimumCloneLength;
			}
			int minCloneLength = lastMinCloneLengthByDoDetectClones;
			if (lastMinTKSByDoDetectClones == -1) {
				lastMinTKSByDoDetectClones = main.settingMinimumTKS;
			}
			int minTKS = lastMinTKSByDoDetectClones;
			if (lastShaperLevel == -1) {
				lastShaperLevel = main.settingShaperLevel;
			}
			int shaperLevel = lastShaperLevel;
			if (lastUsePMatch == null) {
				lastUsePMatch = new Boolean(main.settingUsePMatch);
			}
			boolean usePMatch = lastUsePMatch.booleanValue();
			if (lastPreprocessorByDoDetectClones == null || lastPreprocessorByDoDetectClones.length() == 0) {
				lastPreprocessorByDoDetectClones = main.settingPreprocessor;
			}
			String preprocessor = lastPreprocessorByDoDetectClones;
			if (lastUsePrescreening == null) {
				lastUsePrescreening = new Boolean(main.settingUsePrescreening);
			}
			boolean usePrescreening = lastUsePrescreening.booleanValue();
			{
				CloneDetectionFromFileListOptionDialog dialog = new CloneDetectionFromFileListOptionDialog(shell);
				String[] preprocessors = ccfxhelper.getPreprocessScriptList();
				if (preprocessors == null) {
					return;
				}
				for (int i = 0; i < preprocessors.length; ++i) {
					dialog.addForcedPreprocessorItem(preprocessors[i]);
				}
				dialog.setText(Messages.getString("gemx.MainWindow.S_CLONEDETECTION_FROMFILELIST_STEP2"));   //$NON-NLS-1$
				dialog.setFileListPath(fileListPath);
				dialog.setForcedPreprocessor(preprocessor);
				dialog.setMinimumCloneLength(minCloneLength);
				dialog.setMinimumTKS(minTKS);
				dialog.setShaperLevel(shaperLevel);
				dialog.setUsePMatch(usePMatch);
				dialog.setUsePrescreening(usePrescreening);
				int result = dialog.open();
				if (result == SWT.CANCEL) {
					return;
				}
				int ml = dialog.getMinimumCloneLength();
				if (ml != minCloneLength) {
					minCloneLength = ml;
				}
				lastMinCloneLengthByDoDetectClones = ml;
				int mt = dialog.getMinimumTKS();
				if (mt != minTKS) {
					minTKS = mt;
				}
				int sl = dialog.getShaperLevel();
				if (sl != shaperLevel) {
					shaperLevel = sl;
				}
				boolean up = dialog.isUsePMatch();
				if (up != usePMatch) {
					usePMatch = up;
				}
				boolean ur = dialog.isUsePrescreening();
				if (ur != usePrescreening) {
					usePrescreening = ur;
				}
				preprocessor = dialog.getForcedPreprocessor();
				if (preprocessor == null || preprocessor.length() == 0) {
					showErrorMessage("Preprocess script is not specified"); //$NON-NLS-1$
					return;
				}
				lastPreprocessorByDoDetectClones = preprocessor;
			}

			// detect clones
			String encodingName = main.settingEncoding;
			String fileListName = null;
			if (usePrescreening) {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_PRESCREENING"));  //$NON-NLS-1$
				TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
				try {
					ccfxhelper.doPrescreening(tempFileName1, fileListPath, preprocessor, encodingName,
							preprocessFileDirectories,
							main.settingMaxWorkerThreads);
					usePrescreening = true;
					fileListName = tempFileName1;
				} catch (IOException e) {
					showErrorMessage("error in invoking prescreening filter"); //$NON-NLS-1$
					return;
				} finally {
					tmcc.dispose();
				}
			} else {
				fileListName = fileListPath;
			}

			statusBar.setText(Messages.getString("gemx.MainWindow.SB_DETECTING_CLONES")); //$NON-NLS-1$
			if (! moveToBackupFile(theCloneDataFileName)) {  //$NON-NLS-1$
				return;
			}
			{
				ArrayList<String> options = ccfxhelper.buildCcfxCommandLineForDetection(preprocessor, 
						encodingName, fileListName, minCloneLength, 
						minTKS, shaperLevel, usePMatch, 
						main.settingChunkSize, main.settingMaxWorkerThreads, preprocessFileDirectories);
				options.addAll(Arrays.asList(new String[] { "-o", theCloneDataFileName }));

				if (usePrescreening) {
					options.addAll(Arrays.asList(new String[] { "-mr", "masked" }));   //$NON-NLS-1$ //$NON-NLS-2$
				}

				TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
				try {
					if (CCFinderX.theInstance.invokeCCFinderX(options.toArray(new String[0])) != 0) {
						showErrorMessage("Some error occurred in clone detection"); //$NON-NLS-1$
						return;
					}
				} finally {
					tmcc.dispose();
				}
			}
		}
		finally {
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}

		// update clone data
		do_open_clone_data_file_i(theCloneDataFileName); //$NON-NLS-1$

		if (this.cloneSetTable.isCloneSetSizeTooLarge()) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_DETECTED_CLONES_TOO_MANY")); //$NON-NLS-1$
		}

		if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
			this.add_both_file_and_clone_set_metrics();
		} else {
			if (main.settingCalcFileMetricAlways) {
				add_file_metrics();
			}
			if (main.settingCalcCloneMetricAlways) {
				add_clone_set_metrics();
			}
		}
	}

	public void do_open_clone_data_file_i(String path) {
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READING_CLONE_DATA_FILE")); //$NON-NLS-1$
			CCFinderX.theInstance.clearPrepFileCacheState();
			
			rootModel.readCloneDataFile(path);

			scopeHistory = new ArrayList<ScopeHistoryItem>();
			ScopeHistoryItem shi = new ScopeHistoryItem();
			shi.cloneDataFile = path;
			scopeHistory.add(shi);
			scopeHistoryList.removeAll();

			currentSelectedCloneSetIDs = null;
			currentSelectedFileIndices = null;

			update_model(rootModel);
			theScrapbook.clearData();

			this.addScopeHistoryItem("File: " + path, currentScope.getFileCount(), currentScope.getCloneSetCount(), true); //$NON-NLS-1$

			metricModels.dispose();
			metricModels = new MetricModelsHolder(this);
		} catch (java.io.IOException e) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_FILE_IO_ERROR_IN_READING_A_CLONE_DATA_FILE") //$NON-NLS-1$
					+ path);
		} catch (DataFileReadError e) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_CAN_NOT_READ_A_CLONE_DATA_FILE") + path); //$NON-NLS-1$
		} finally {
			tmcc.dispose();
		}
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		update_info_bars();
	}

	private void do_open_clone_data_file() {
		FileDialog dialog = new FileDialog(shell, SWT.OPEN);
		String[] exts = new String[] { "*.ccfxd", "*.icetd" }; //$NON-NLS-1$ //$NON-NLS-2$
		String[] extNames = new String[] { Messages.getString("gemx.MainWindow.S_CLONE_DATA_FILE") }; //$NON-NLS-1$
		dialog.setFilterExtensions(exts);
		dialog.setFilterNames(extNames);

		dialog.open();

		String dir = dialog.getFilterPath();
		String files[] = dialog.getFileNames();
		if (files.length >= 2) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_SPECIFY_ONE_CLONE_DATA_FILE")); //$NON-NLS-1$
		} else if (files.length == 1) {
			String path = dir + java.io.File.separator + files[0];
			do_open_clone_data_file_i(path);

			if (this.cloneSetTable.isCloneSetSizeTooLarge()) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_DETECTED_CLONES_TOO_MANY"));  //$NON-NLS-1$
			}

			if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
				this.add_both_file_and_clone_set_metrics();
			} else {
				if (main.settingCalcFileMetricAlways) {
					add_file_metrics();
				}
				if (main.settingCalcCloneMetricAlways) {
					add_clone_set_metrics();
				}
			}
		} else {
			return;
		}
	}

	private void do_exit() {
		shell.close();
	}

	private void do_show_about_dialog() {
		AboutDialog aboutDialog = widgetsFactory.newAboutDialog(shell);
		aboutDialog.open();
	}

	public void doCcfxSettingsDialog() {
		CcfxSettingsDialog dialog = new CcfxSettingsDialog(shell);
		String[] preprocessors = ccfxhelper.getPreprocessScriptsRelatedToAnyExtensions();
		if (preprocessors == null) {
			return;
		}
		for (int i = 0; i < preprocessors.length; ++i) {
			dialog.addPreprocessorItem(preprocessors[i]);
		}
		int prepIndex = -1;
		for (int i = 0; i < preprocessors.length; ++i) {
			if (preprocessors[i].equals(main.settingPreprocessor)) {
				prepIndex = i;
				break; // for i
			}
		}
		if (prepIndex >= 0) {
			dialog.setPreprocessor(preprocessors[prepIndex]);
		}
		else {
			dialog.setPreprocessor(""); //$NON-NLS-1$
		}
		dialog.setChunkSize(main.settingChunkSize);
		dialog.setEncoding(main.settingEncoding);
		dialog.setMinimumCloneLength(main.settingMinimumCloneLength);
		dialog.setMinimumTKS(main.settingMinimumTKS);
		dialog.setShaperLevel(main.settingShaperLevel);
		dialog.setUsePMatch(main.settingUsePMatch);
		dialog.setUsePreprocessCache(main.settingUsePreprocessCache);
		dialog.setMaxWorkerThreads(main.settingMaxWorkerThreads);
		dialog.setUsePrescreening(main.settingUsePrescreening);

		int result = dialog.open();

		if (result == SWT.OK) {
			clearDetectConesOptions();
			main.settingChunkSize = dialog.getChunkSize();
			String newEncoding = dialog.getEncoding();
			if (! newEncoding.equals(main.settingEncoding)) {
				main.settingEncoding = dialog.getEncoding();
				this.sourcePane.setEncoding(main.settingEncoding);
			}
			main.settingMinimumCloneLength = dialog.getMinimumCloneLength();
			main.settingMinimumTKS = dialog.getMinimumTKS();
			main.settingPreprocessor = dialog.getPreprocessor();
			main.settingShaperLevel = dialog.getShaperLevel();
			main.settingUsePMatch = dialog.isUsePMatch();
			main.settingUsePreprocessCache = dialog.isUsePreprocessCache();
			main.settingMaxWorkerThreads = dialog.getMaxWorkerThreads();
			main.settingUsePrescreening = dialog.isUsePrescreening();
		}
	}

	public void doGemxSettingsDialog() {
		GemXSettingsDialog dialog = new GemXSettingsDialog(shell);

		dialog.setResizeScatterPlot(main.settingResizeScatterPlot);
		dialog.setCalcFileMetricAlways(main.settingCalcFileMetricAlways);
		dialog.setCalcCloneMetricAlways(main.settingCalcCloneMetricAlways);
		dialog.setNtipleSourceTextPane(main.settingNtipleSourceTextPane);
		dialog.setAllFileViewModeEnabled(main.settingAllFileViewModeEnabled);
		dialog.setClonesetTableClickToShowPair(main.settingClonesetTableClickToShowPair);
		dialog.setResetItemInContextMenus(main.settingResetScopeItemInContextMenus);

		int result = dialog.open();

		if (result == SWT.OK) {
			if (! main.settingResizeScatterPlot && dialog.isResizeScatterPlot()) {
				this.resizeScatterPlotDrawingAreaToWindow(1);
			}
			main.settingResizeScatterPlot = dialog.isResizeScatterPlot();

			if (! main.settingCalcFileMetricAlways && dialog.isCalcFileMetricAlways()) {
				add_file_metrics();
			}
			main.settingCalcFileMetricAlways = dialog.isCalcFileMetricAlways();

			if (! main.settingCalcCloneMetricAlways && dialog.isCalcCloneMetricAlways()) {
				add_clone_set_metrics();
			}
			main.settingCalcCloneMetricAlways = dialog.isCalcCloneMetricAlways();

			int ntipleSourceTextPane = dialog.getNtipleSourceTextPane();
			sourcePane.setCapacity(ntipleSourceTextPane);
			theScrapbook.setCapacity(ntipleSourceTextPane);
			main.settingNtipleSourceTextPane = ntipleSourceTextPane;

			main.settingAllFileViewModeEnabled = dialog.isAllFileViewModeEnabled();
			main.settingClonesetTableClickToShowPair = dialog.isClonesetTableClickToShowPair();
			main.settingResetScopeItemInContextMenus = dialog.isResetItemInContextMenus();
		}
	}

	private void update_model(Model newModel) {
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			if (currentScope != this.rootModel) {
				currentScope.dispose();
			}
			currentScope = null;
	
			currentScope = newModel;
	
			currentSelectedFileIndices = null;
			currentSelectedCloneSetIDs = null;
	
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_FILE_TABLE")); //$NON-NLS-1$
			fileTable.updateModel(currentScope);
	
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_SCATTER_PLOT_PANE")); //$NON-NLS-1$
			scatterPlotPane.updateModel(currentScope);
	
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_CLONE_SET_TABLE")); //$NON-NLS-1$
			cloneSetTable.updateModel(currentScope);
	
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_SOURCE_PANE")); //$NON-NLS-1$
			sourcePane.updateModel(currentScope, main.settingAllFileViewModeEnabled);
	
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		} finally {
			tmcc.dispose();
		}
	}

	private void fit_scope_to_current_selected_files() {
		if (currentSelectedFileIndices != null) {
			fitScopeToFiles(currentSelectedFileIndices);
		}
	}

	private void fit_scope_to_current_checked_files() {
		int[] checked = fileTable.getCheckedFileIndices();
		fitScopeToFiles(checked);
	}

	public void fitScopeToFiles(int[] selectedIndices) {
		assert selectedIndices != null;
		if (selectedIndices.length == 0) {
			MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_WARNING);
			mes.setText("Warning - GemX"); //$NON-NLS-1$
			mes.setMessage(Messages.getString("gemx.MainWindow.S_NO_FILES_ARE_SELECTED")); //$NON-NLS-1$
			mes.open();
			return;
		}
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_NEW_SCOPE")); //$NON-NLS-1$
		Model newModel = currentScope.fitScopeToFiles(selectedIndices);

		if (newModel != null && newModel.getFileCount() > 0) {
			Selection sel = save_selection();
			update_model(newModel);
			metricModels.dispose();
			metricModels = new MetricModelsHolder(this);

			ScopeHistoryItem shi = new ScopeHistoryItem();
			shi.cloneDataFile = newModel.getCloneDataFilePath();
			scopeHistory.add(shi);
			this.addScopeHistoryItem("Fit File: " + String.valueOf(selectedIndices.length), newModel.getFileCount(), newModel.getCloneSetCount(), true); //$NON-NLS-1$

			if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
				this.add_both_file_and_clone_set_metrics();
			} else {
				if (main.settingCalcFileMetricAlways) {
					add_file_metrics();
				}
				if (main.settingCalcCloneMetricAlways) {
					add_clone_set_metrics();
				}
			}
			load_selection(sel);
			update_info_bars();
		}
	}

	public void fitScopeToFilesExceptFor(int[] selectedIndex) {
		assert selectedIndex != null;
		if (selectedIndex.length == 0) {
			MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_WARNING);
			mes.setText("Warning - GemX"); //$NON-NLS-1$
			mes.setMessage(Messages.getString("gemx.MainWindow.S_NO_FILES_ARE_SELECTED")); //$NON-NLS-1$
			mes.open();
			return;
		}
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_NEW_SCOPE")); //$NON-NLS-1$
		Model newModel = currentScope.fitScopeToFilesExceptFor(selectedIndex);

		if (newModel != null && newModel.getFileCount() > 0) {
			Selection sel = save_selection();
			update_model(newModel);
			metricModels.dispose();
			metricModels = new MetricModelsHolder(this);

			ScopeHistoryItem shi = new ScopeHistoryItem();
			shi.cloneDataFile = newModel.getCloneDataFilePath();
			scopeHistory.add(shi);
			this.addScopeHistoryItem("Fit File Except: " + String.valueOf(selectedIndex.length), newModel.getFileCount(), newModel.getCloneSetCount(), true); //$NON-NLS-1$

			if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
				this.add_both_file_and_clone_set_metrics();
			} else {
				if (main.settingCalcFileMetricAlways) {
					add_file_metrics();
				}
				if (main.settingCalcCloneMetricAlways) {
					add_clone_set_metrics();
				}
			}
			load_selection(sel);
			update_info_bars();
		}
	}

	public void resetScope() {
		if (scopeHistory != null && scopeHistory.size() > 1) {
			Selection sel = save_selection();
			ScopeHistoryItem shi = scopeHistory.get(0);
			while (scopeHistory.size() > 1) {
				scopeHistory.remove(scopeHistory.size() - 1);
			}

			try {
				String recoveredModelFilePath = shi.cloneDataFile;
				Model recoveredModel = new Model();
				try {
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_READING_CLONE_DATA_FILE")); //$NON-NLS-1$
					recoveredModel.readCloneDataFile(recoveredModelFilePath);
				}
				catch (IOException e) {
					MessageBox box1 = new MessageBox(shell, SWT.OK | SWT.ICON_ERROR);
					box1.setText("Error - GemX"); //$NON-NLS-1$
					box1.setMessage(String.format(Messages.getString("gemx.MainWindow.S_CANNOT_READ_FILE"), recoveredModelFilePath)); //$NON-NLS-1$
					box1.open();
				}
				update_model(recoveredModel);
				metricModels.dispose();
				metricModels = new MetricModelsHolder(this);

				if (shi.fileMetricFile != null) {
					metricModels.readFileMetricFile(shi.fileMetricFile, this.currentScope.getMaxFileID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_FILE_TABLE")); //$NON-NLS-1$
					fileTable.addFileMetricModel(metricModels.getFileMetricModel());
				}

				if (shi.cloneSetMetricFile != null) {
					metricModels.readCloneSetMetricFile(shi.cloneSetMetricFile, this.currentScope.getMaxCloneSetID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_CLONE_SET_TABLE")); //$NON-NLS-1$
					cloneSetTable.addCloneSetMetricModel(metricModels.getCloneSetMetricModel());
					scatterPlotPane.updateModel(currentScope, metricModels.getCloneSetMetricModel());
				}

				statusBar.setText(Messages.getString("gemx.MainWindow.SB_RECOVERING_SELECTIONS")); //$NON-NLS-1$
				load_selection(sel);
				update_info_bars();
			}
			finally {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$

				scopeHistoryList.remove(1, scopeHistoryList.getItemCount() - 1);
				setScopeHistoryMark(0);
			}
		}
	}

	private void update_info_bars() {
		preprocessorBar.setText(String.format("PS: %s", currentScope.getPreprocessScript())); //$NON-NLS-1$
		fileInfoBar.setText(String.format("#Files: %d (%d)",  //$NON-NLS-1$
				currentScope.getFileCount(), 
				(currentSelectedFileIndices != null ? currentSelectedFileIndices.length : 0)));
		cloneSetInfoBar.setText(String.format("#Clone Sets: %d (%d)", //$NON-NLS-1$
				currentScope.getCloneSetCount(),
				(currentSelectedCloneSetIDs != null ? currentSelectedCloneSetIDs.length : 0)));
	}

	private static class Selection {
		public int[] fileIDs;
		public long[] cloneSetIDs;
	}
	private Selection save_selection() {
		Selection sel = new Selection();
		if (currentScope != null && currentSelectedFileIndices != null) {
			sel.fileIDs = currentScope.getFileIDFromFileIndex(currentSelectedFileIndices);
		} else {
			sel.fileIDs = null;
		}
		sel.cloneSetIDs = currentSelectedCloneSetIDs;
		return sel;
	}
	private void load_selection(Selection sel) {
		if (sel.fileIDs != null) {
			int[] fileIndices = currentScope.getFileIndexFromFileID(sel.fileIDs);
			this.setFileSelection(fileIndices, null);
		}
		if (sel.cloneSetIDs != null) {
			this.setCloneSelection_i(sel.cloneSetIDs, null);
		}
	}

	public void popScope() {
		if (scopeHistory != null && scopeHistory.size() > 1) {
			Selection sel = save_selection();
			scopeHistory.remove(scopeHistory.size() - 1);

			try {
				ScopeHistoryItem shi =  scopeHistory.get(scopeHistory.size() - 1);

				String recoveredModelFilePath = shi.cloneDataFile;
				Model recoveredModel = new Model();
				try {
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_READING_CLONE_DATA_FILE")); //$NON-NLS-1$
					recoveredModel.readCloneDataFile(recoveredModelFilePath);
				}
				catch (IOException e) {
					MessageBox box1 = new MessageBox(shell, SWT.OK | SWT.ICON_ERROR);
					box1.setText("Error - GemX"); //$NON-NLS-1$
					box1.setMessage(String.format(Messages.getString("gemx.MainWindow.S_CANNOT_READ_FILE"), recoveredModelFilePath)); //$NON-NLS-1$
					box1.open();
				}
				update_model(recoveredModel);
				metricModels.dispose();
				metricModels = new MetricModelsHolder(this);

				if (shi.fileMetricFile != null) {
					metricModels.readFileMetricFile(shi.fileMetricFile, this.currentScope.getMaxFileID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_FILE_TABLE")); //$NON-NLS-1$
					fileTable.addFileMetricModel(metricModels.getFileMetricModel());
				}

				if (shi.cloneSetMetricFile != null) {
					metricModels.readCloneSetMetricFile(shi.cloneSetMetricFile, this.currentScope.getMaxCloneSetID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_CLONE_SET_TABLE")); //$NON-NLS-1$
					cloneSetTable.addCloneSetMetricModel(metricModels.getCloneSetMetricModel());
					scatterPlotPane.updateModel(currentScope, metricModels.getCloneSetMetricModel());
				}

				statusBar.setText(Messages.getString("gemx.MainWindow.SB_RECOVERING_SELECTIONS")); //$NON-NLS-1$
				load_selection(sel);
				update_info_bars();
			}
			finally {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$

				int index = scopeHistoryList.getItemCount() - 1;
				scopeHistoryList.remove(index);
				setScopeHistoryMark(index - 1);
			}
		}
	}

	public void selectFilesByCloneClassID(long[] cloneClassIDs) {
		assert cloneClassIDs != null;

		int[] files = currentScope.getRelatedFilesOfCloneSets(cloneClassIDs);
		Arrays.sort(files);
		setFileSelection(files, null);
		sourcePane.clearInitalTopPosition();
		this.setCloneSelection(cloneClassIDs, null);
	}

	public void selectCommonFilesByCloneClassID(long[] cloneClassIDs) {
		assert cloneClassIDs != null;

		int[] files = currentScope.getCommonRelatedFilesOfCloneSets(cloneClassIDs);
		Arrays.sort(files);
		setFileSelection(files, null);
		sourcePane.clearInitalTopPosition();
		this.setCloneSelection(cloneClassIDs, null);
	}

	public void selectCloneClassIDByFileID(int[] selectedIndices) {
		assert selectedIndices != null;

		long[] cloneIDs = currentScope.getCloneSetIDsIncludedFiles(selectedIndices);
		Arrays.sort(cloneIDs);
		this.setCloneSelection(cloneIDs, null);
	}

	public void selectCommonCloneClassIDByFileID(int[] selectedIndices) {
		assert selectedIndices != null;

		long[] cloneIDs = currentScope.getCloneSetIDsCommonlyIncludedFiles(selectedIndices);
		Arrays.sort(cloneIDs);
		this.setCloneSelection(cloneIDs, null);
	}

	private long lastShowingCloneSetIDByShowCloneCode = -1;
	private int lastShowingClonePairIndexByShowCloneCode = -1;

	public void showACodeFragmentOfClone(long cloneSetID, CloneSelectionListener src) {
		if (lastShowingCloneSetIDByShowCloneCode == cloneSetID && lastShowingClonePairIndexByShowCloneCode == -1) {
			return;
		}

		lastShowingCloneSetIDByShowCloneCode = cloneSetID;
		lastShowingClonePairIndexByShowCloneCode = -1;

		statusBar.setText(Messages.getString("gemx.MainWindow.SB_READING_TOKEN_SEQUENCE")); //$NON-NLS-1$
		try {
			long[] cloneSetIDs = new long[] { cloneSetID };
			ClonePair[] pairs = currentScope.getClonePairsOfCloneSets(cloneSetIDs);
			if (pairs.length > 0) {
				CodeFragment f = pairs[0].getLeftCodeFragment();
				int[] files = new int[] { f.file };
				setFileSelection_i(files, null);
				sourcePane.clearInitalTopPosition();
				setCodeFragmentSelection_i(f, cloneSetID, src);
			}

		}
		finally {
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}
	}

	public void showNextPairedCodeOfClone(long cloneSetID, int direction, CloneSelectionListener src) {
		assert direction == -1 || direction == 1;

		if (cloneSetID != lastShowingCloneSetIDByShowCloneCode) {
			lastShowingCloneSetIDByShowCloneCode = cloneSetID;
			lastShowingClonePairIndexByShowCloneCode = 0;
		}

		statusBar.setText(Messages.getString("gemx.MainWindow.SB_READING_TOKEN_SEQUENCE")); //$NON-NLS-1$
		try {
			long[] cloneSetIDs = new long[] { cloneSetID };
			ClonePair[] pairs = currentScope.getClonePairsOfCloneSets(cloneSetIDs);
			if (pairs.length > 0) {
				lastShowingClonePairIndexByShowCloneCode = (lastShowingClonePairIndexByShowCloneCode + direction + pairs.length) % pairs.length;

				int fileIndex1 = pairs[lastShowingClonePairIndexByShowCloneCode].leftFile;
				int fileIndex2 = pairs[lastShowingClonePairIndexByShowCloneCode].rightFile;
				int[] files = new int[] { fileIndex1, fileIndex2 };
				setFileSelection_i(files, null);
				sourcePane.clearInitalTopPosition();
				setClonePairSelection_i(pairs[lastShowingClonePairIndexByShowCloneCode], src);
			}

		}
		finally {
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}
	}

	public void selectFilesThatHaveClonesBetweenFiles(int[] fileIndices, FileSelectionListener src) {
		int[] files = currentScope.getNeighborFiles(fileIndices, true);
		//long[] cloneClassIDs = viewedModel.getCloneSetIDsIncludedFile(fileIndex);
		//int[] files = viewedModel.getRelatedFilesOfCloneSets(cloneClassIDs);
		setFileSelection(files, src);
	}

	public void selectFilesUnderDirectory(int fileIndex, int rad) {
		if (rad < 0) {
			return; // no such directory
		}
		SourceFile file = currentScope.getFile(fileIndex);
		String dir = file.path;
		for (int i = 0; i < rad; ++i) {
			int pos = dir.lastIndexOf('/');
			{
				int p2 = dir.lastIndexOf('\\');
				if (pos < 0 || p2 >= 0 && p2 < pos) {
					pos = p2;
				}
			}
			if (pos < 0) {
				return; // no such directory
			}
			dir = dir.substring(0, pos);
		}
		int pos = dir.length();
		dir = dir + file.path.substring(pos, pos + 1);
		int[] fileIndices = currentScope.findFilesUnderDirectory(dir);
		this.setFileSelection_i(fileIndices, null);
		update_info_bars();
		//fileTable.setSelection(fileIndices);
		//scatterPlotPane.setSelection(fileIndices);
		//sourcePane.setSelection(fileIndices);
	}

	public void doFileSelectionFromOneOfParentPaths(int fileIndex) {
		SourceFile file = currentScope.getFile(fileIndex);
		ParentDirectorySelectDialog dialog = new ParentDirectorySelectDialog(shell, file.path, currentScope.getCommonFilePath());
		int result = dialog.open();
		if (result == SWT.OK) {
			String path = dialog.getChoosedPath();
			int[] fileIndices = currentScope.findFilesUnderDirectory(path);
			this.setFileSelection_i(fileIndices, null);
			update_info_bars();
			//fileTable.setSelection(fileIndices);
			//scatterPlotPane.setSelection(fileIndices);
			//sourcePane.setSelection(fileIndices);
		}
	}

	public void doFilteringFilesByMetrics() {
		if (currentScope.getCloneDataFilePath() == null || currentScope.getCloneDataFilePath().length() == 0) {
			return;
		}
		if (metricModels.getFileMetricModel() == null) {
			if (showConfirmationMessage(SWT.OK | SWT.CANCEL | SWT.ICON_QUESTION,
					Messages.getString("gemx.MainWindow.S_CALCULATE_FILE_METRICS_P")) != SWT.OK) { //$NON-NLS-1$
				return;
			}
			add_file_metrics();
			if (metricModels.getFileMetricModel() == null) {
				return;
			}
		}

		FileMetricModel fileMetricModel = metricModels.getFileMetricModel();
		final MetricsSummaryData summaryData = fileMetricModel.getSummaryData();
		double[] minValues = summaryData.getMinValues();
		double[] maxValues = summaryData.getMaxValues();
		MinMaxSettingDialog settingDialog = new MinMaxSettingDialog(shell, fileMetricModel.getFieldCount());
		settingDialog.setText(Messages.getString("gemx.MainWindow.S_SPECIFY_FILE_METRIC_RANGES_GEMX")); //$NON-NLS-1$

		for (int i = 0; i < minValues.length; ++i) {
			settingDialog.setName(i, fileMetricModel.getMetricName(i));
			if (fileMetricModel.isFlotingPoint(i)) {
				settingDialog.setMinMaxDigits(i, (int)Math.floor(minValues[i] * 1000), (int)Math.ceil(maxValues[i] * 1000), 3);
			} else {
				settingDialog.setMinMaxDigits(i, (int)minValues[i], (int)maxValues[i], 0);
			}
		}

		if (settingDialog.open() == SWT.CANCEL) {
			return;
		}

		StringBuffer usedMetricsStr = new StringBuffer();
		ArrayList<String> expressions = new ArrayList<String>();
		for (int i = 0; i < minValues.length; ++i) {
			if (settingDialog.getValueEdited(i)) {
				String name = fileMetricModel.getMetricName(i);
				usedMetricsStr.append(" "); //$NON-NLS-1$
				usedMetricsStr.append(name);
				if (fileMetricModel.isFlotingPoint(i)) {
					makeExpr(expressions, name, settingDialog.getMinimum(i) / 1000.0, settingDialog.getMaximum(i) / 1000.0);
				} else {
					makeExpr(expressions, name, settingDialog.getMinimum(i), settingDialog.getMaximum(i));
				}
				expressions.add("and"); //$NON-NLS-1$
			}
		}
		if (expressions.size() == 0) { // no filtering to be performed
			return;
		}
		expressions.remove(expressions.size() - 1); // remove the last "and"

		ScopeHistoryItem sht = scopeHistory.get(scopeHistory.size() - 1);
		String metricFile = sht.fileMetricFile;

		Picosel picosel = new Picosel();
		String commandLine = picosel.setCommandLine(metricFile, tempFileName2, "FID", expressions.toArray(new String[0])); //$NON-NLS-1$
		System.err.println("> " + commandLine); //$NON-NLS-1$
		int result = picosel.invokePicosel();
		if (result != 0) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_PERFORM_FILTERING_FILES")); //$NON-NLS-1$
			return;
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFileName2);

		int action = settingDialog.getAction();
		switch (action) {
		case MinMaxSettingDialog.ActionMakeScope:
		{
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_NEW_SCOPE")); //$NON-NLS-1$
			Model newModel = currentScope.fitScopeToFileIDs(tempFileName2);

			if (newModel != null && newModel.getFileCount() > 0) {
				Selection sel = save_selection();
				update_model(newModel);
				metricModels.dispose();
				metricModels = new MetricModelsHolder(this);

				ScopeHistoryItem shi = new ScopeHistoryItem();
				shi.cloneDataFile = newModel.getCloneDataFilePath();
				scopeHistory.add(shi);
				this.addScopeHistoryItem("Filtering File: " + usedMetricsStr.toString(), newModel.getFileCount(), newModel.getCloneSetCount(), true); //$NON-NLS-1$

				if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
					this.add_both_file_and_clone_set_metrics();
				} else {
					if (main.settingCalcFileMetricAlways) {
						add_file_metrics();
					}
					if (main.settingCalcCloneMetricAlways) {
						add_clone_set_metrics();
					}
				}
				load_selection(sel);
				update_info_bars();
			}
		}
		break;
		case MinMaxSettingDialog.ActionAddCheckmark:
		{
			gnu.trove.TIntArrayList ary;
			try {
				ary = ArrayUtil.readIntList(tempFileName2);
			} catch (FileNotFoundException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (IOException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (utility.ArrayUtil.NumberFormatErrorAtLineOfFile e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			}
			fileTable.addCheckmarks(fileTable.getIndicesFromFileIDs(ary.toNativeArray()));
		}
		break;
		case MinMaxSettingDialog.ActionSelect:
		{
			gnu.trove.TIntArrayList ary;
			try {
				ary = ArrayUtil.readIntList(tempFileName2);
			} catch (FileNotFoundException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (IOException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (utility.ArrayUtil.NumberFormatErrorAtLineOfFile e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			}
			this.setFileSelection(fileTable.getIndicesFromFileIDs(ary.toNativeArray()), null);
		}
		break;
		default:
			assert false;
		}
	}

	public void doFilteringCloneSetByMetrics() {
		if (currentScope.getCloneDataFilePath() == null || currentScope.getCloneDataFilePath().length() == 0) {
			return;
		}
		if (metricModels.getCloneSetMetricModel() == null) {
			if (showConfirmationMessage(SWT.OK | SWT.CANCEL | SWT.ICON_QUESTION,
					Messages.getString("gemx.MainWindow.S_CALCULATE_CLONE_SET_METRICS_P")) != SWT.OK) { //$NON-NLS-1$
				return;
			}
			add_clone_set_metrics();
			if (metricModels.getCloneSetMetricModel() == null) {
				return;
			}
		}

		ClonesetMetricModel cloneSetMetricModel = metricModels.getCloneSetMetricModel();
		MetricsSummaryData summaryData = cloneSetMetricModel.getSummaryData();
		double[] minValues = summaryData.getMinValues();
		double[] maxValues = summaryData.getMaxValues();
		MinMaxSettingDialog settingDialog = new MinMaxSettingDialog(shell, cloneSetMetricModel.getFieldCount());
		settingDialog.setText(Messages.getString("gemx.MainWindow.S_SPECIFY_CLONE_SET_METRIC_RANGES")); //$NON-NLS-1$
		for (int i = 0; i < minValues.length; ++i) {
			settingDialog.setName(i, cloneSetMetricModel.getMetricName(i));
			if (cloneSetMetricModel.isFlotingPoint(i)) {
				settingDialog.setMinMaxDigits(i, (int)Math.floor(minValues[i] * 1000), (int)Math.ceil(maxValues[i] * 1000), 3);
			} else {
				settingDialog.setMinMaxDigits(i, (int)minValues[i], (int)maxValues[i], 0);
			}
		}

		if (settingDialog.open() == SWT.CANCEL) {
			return;
		}

		StringBuffer usedMetricsStr = new StringBuffer();
		ArrayList<String> expressions = new ArrayList<String>();
		for (int i = 0; i < minValues.length; ++i) {
			if (settingDialog.getValueEdited(i)) {
				String name = cloneSetMetricModel.getMetricName(i);
				usedMetricsStr.append(" "); //$NON-NLS-1$
				usedMetricsStr.append(name);
				if (cloneSetMetricModel.isFlotingPoint(i)) {
					makeExpr(expressions, name, settingDialog.getMinimum(i) / 1000.0, settingDialog.getMaximum(i) / 1000.0);
				} else {
					makeExpr(expressions, name, settingDialog.getMinimum(i), settingDialog.getMaximum(i));
				}
				expressions.add("and"); //$NON-NLS-1$
			}
		}
		if (expressions.size() == 0) { // no filtering to be performed
			return;
		}
		expressions.remove(expressions.size() - 1); // remove the last "and"

		ScopeHistoryItem sht = scopeHistory.get(scopeHistory.size() - 1);
		String metricFile = sht.cloneSetMetricFile;

		Picosel picosel = new Picosel();
		String commandLine = picosel.setCommandLine(metricFile, tempFileName2, "CID", expressions.toArray(new String[0])); //$NON-NLS-1$
		System.err.println("> " + commandLine); //$NON-NLS-1$
		int result = picosel.invokePicosel();
		if (result != 0) {
			showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_PERFORM_FILTERING_CLONE_SETS")); //$NON-NLS-1$
			return;
		}
		TemporaryFileManager.registerFileToBeRemoved(tempFileName2);

		final int action = settingDialog.getAction();
		switch (action) {
		case MinMaxSettingDialog.ActionMakeScope:
		{
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_NEW_SCOPE")); //$NON-NLS-1$
			Model newModel = currentScope.fitScopeToClones(tempFileName2);

			if (newModel != null && newModel.getFileCount() > 0) {
				Selection sel = save_selection();
				update_model(newModel);
				metricModels.dispose();
				metricModels = new MetricModelsHolder(this);

				ScopeHistoryItem shi = new ScopeHistoryItem();
				shi.cloneDataFile = newModel.getCloneDataFilePath();
				scopeHistory.add(shi);
				this.addScopeHistoryItem("Filtering Clone Set: " + usedMetricsStr.toString(), newModel.getFileCount(), newModel.getCloneSetCount(), true); //$NON-NLS-1$

				if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
					this.add_both_file_and_clone_set_metrics();
				} else {
					if (main.settingCalcFileMetricAlways) {
						add_file_metrics();
					}
					if (main.settingCalcCloneMetricAlways) {
						add_clone_set_metrics();
					}
				}
				load_selection(sel);
				update_info_bars();
			}
		}
		break;
		case MinMaxSettingDialog.ActionAddCheckmark:
		{
			gnu.trove.TLongArrayList ary;
			try {
				ary = ArrayUtil.readLongList(tempFileName2);
			} catch (FileNotFoundException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (IOException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (utility.ArrayUtil.NumberFormatErrorAtLineOfFile e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			}
			cloneSetTable.addCheckmarks(ary.toNativeArray());
		}
		break;
		case MinMaxSettingDialog.ActionSelect:
		{
			gnu.trove.TLongArrayList ary;
			try {
				ary = ArrayUtil.readLongList(tempFileName2);
			} catch (FileNotFoundException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (IOException e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			} catch (utility.ArrayUtil.NumberFormatErrorAtLineOfFile e) {
				showErrorMessage(Messages.getString("gemx.MainWindow.S_FAIL_TO_OPEN_TEMPORARY_FILE"));  //$NON-NLS-1$
				return;
			}
			this.setCloneSelection(ary.toNativeArray(), null);
		}
		break;
		default:
			assert false;
		}
	}

	private <T> void makeExpr(ArrayList<String> expressions, String columnName, T min, T max) {
		expressions.addAll(Arrays.asList(new String[] { 
				columnName, ".ge.", String.valueOf(min),  //$NON-NLS-1$
				"and",  //$NON-NLS-1$
				columnName, ".le.", String.valueOf(max)  //$NON-NLS-1$
		}));
	}

	private void fit_scope_to_current_selected_cloneset_ids() {
		if (this.currentSelectedCloneSetIDs != null) {
			fitScopeToCloneSetIDs(currentSelectedCloneSetIDs);
		}
	}

	private void fit_scope_to_current_checked_cloneset_ids() {
		long[] checked = cloneSetTable.getCheckedCloneSetIDs();
		fitScopeToCloneSetIDs(checked);
	}

	public void fitScopeToCloneSetIDs(long[] cloneClassIDs) {
		assert cloneClassIDs != null;
		if (cloneClassIDs.length == 0) {
			MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_WARNING);
			mes.setText("Warning - GemX"); //$NON-NLS-1$
			mes.setMessage(Messages.getString("gemx.MainWindow.S_NO_CLONE_IDS_ARE_SELECTED")); //$NON-NLS-1$
			mes.open();
			return;
		}
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_NEW_SCOPE")); //$NON-NLS-1$
		Model newModel = currentScope.fitScopeToClones(cloneClassIDs);

		if (newModel != null && newModel.getFileCount() > 0) {
			Selection sel = save_selection();
			update_model(newModel);
			metricModels.dispose();
			metricModels = new MetricModelsHolder(this);

			ScopeHistoryItem shi = new ScopeHistoryItem();
			shi.cloneDataFile = newModel.getCloneDataFilePath();
			scopeHistory.add(shi);
			this.addScopeHistoryItem("Fit Clone: " + String.valueOf(cloneClassIDs.length), newModel.getFileCount(), newModel.getCloneSetCount(), true); //$NON-NLS-1$

			if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
				this.add_both_file_and_clone_set_metrics();
			} else {
				if (main.settingCalcFileMetricAlways) {
					add_file_metrics();
				}
				if (main.settingCalcCloneMetricAlways) {
					add_clone_set_metrics();
				}
			}
			load_selection(sel);
			update_info_bars();
		}
	}

	public void fitScopeToCloneSetIDsExceptFor(long[] cloneClassIDs) {
		assert cloneClassIDs != null;
		if (cloneClassIDs.length == 0) {
			MessageBox mes = new MessageBox(shell, SWT.OK | SWT.ICON_WARNING);
			mes.setText("Warning - GemX"); //$NON-NLS-1$
			mes.setMessage(Messages.getString("gemx.MainWindow.S_NO_CLONE_DS_ARE_SELECTED")); //$NON-NLS-1$
			mes.open();
			return;
		}
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_NEW_SCOPE")); //$NON-NLS-1$
		Model newModel = currentScope.fitScopeToClonesExceptFor(cloneClassIDs);

		if (newModel != null && newModel.getFileCount() > 0) {
			Selection sel = save_selection();
			update_model(newModel);
			metricModels.dispose();
			metricModels = new MetricModelsHolder(this);

			ScopeHistoryItem shi = new ScopeHistoryItem();
			shi.cloneDataFile = newModel.getCloneDataFilePath();
			scopeHistory.add(shi);
			this.addScopeHistoryItem("Fit Clone Excpet: " + String.valueOf(cloneClassIDs.length), newModel.getFileCount(), newModel.getCloneSetCount(), true); //$NON-NLS-1$

			if (main.settingCalcFileMetricAlways && main.settingCalcCloneMetricAlways) {
				this.add_both_file_and_clone_set_metrics();
			} else {
				if (main.settingCalcFileMetricAlways) {
					add_file_metrics();
				}
				if (main.settingCalcCloneMetricAlways) {
					add_clone_set_metrics();
				}
			}
			load_selection(sel);
			update_info_bars();
		}
	}

	public void setFileSelection_i(int[] selectedIndices, FileSelectionListener src) {
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			currentSelectedFileIndices = selectedIndices.clone();
	
			assert selectedIndices != null;
	
			if (fileTable != src) {
				fileTable.setSelection(selectedIndices);
			}
			if (scatterPlotPane != src) {
				scatterPlotPane.setSelection(selectedIndices);
			}
			if (sourcePane != src) {
				sourcePane.setSelection(selectedIndices);
			}
		} finally {
			tmcc.dispose();
		}
	}

	public void setFileSelection(int[] selectedIndices, FileSelectionListener src) {
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_RETRIVING_FILE_DATA")); //$NON-NLS-1$

		lastShowingCloneSetIDByShowCloneCode = -1;
		setFileSelection_i(selectedIndices, src);

		statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		update_info_bars();
	}

	private void setClonePairSelection_i(ClonePair selectedClonePair, CloneSelectionListener src) {
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			assert selectedClonePair != null;
	
			long[] selectedIDs = new long[] { selectedClonePair.classID };
			currentSelectedCloneSetIDs = selectedIDs.clone();
	
			cloneSetTable.setCloneSelection(selectedIDs, src);
			scatterPlotPane.setCloneSelection(selectedIDs, src);
			sourcePane.setClonePairSelection(selectedClonePair);
		} finally {
			tmcc.dispose();
		}

		update_info_bars();
	}

	private void setCodeFragmentSelection_i(CodeFragment selectedCodeFragment, long cloneSetID, 
			CloneSelectionListener src) {
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			assert selectedCodeFragment != null;
	
			long[] selectedIDs = new long[] { cloneSetID };
			currentSelectedCloneSetIDs = selectedIDs.clone();
	
			cloneSetTable.setCloneSelection(selectedIDs, src);
			scatterPlotPane.setCloneSelection(selectedIDs, src);
			sourcePane.setCodeFragmentSelection(selectedCodeFragment, cloneSetID);
		} finally {
			tmcc.dispose();
		}

		update_info_bars();
	}

//	public void setClonePairSelection(ClonePair selectedClonePair, CloneSelectionListener src) {
//	lastShowingClonePairIndexByShowCloneCode = -1;
//	setClonePairSelection_i(selectedClonePair, src);
//	}

	private void setCloneSelection_i(long[] selectedIDs, CloneSelectionListener src) {
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			assert selectedIDs != null;
	
			long[] availableIDs = currentScope.getAvailableCloneSetID(selectedIDs);
			currentSelectedCloneSetIDs = availableIDs;
	
			cloneSetTable.setCloneSelection(availableIDs, src);
			scatterPlotPane.setCloneSelection(availableIDs, src);
			sourcePane.setCloneSelection(availableIDs, src);
			theScrapbook.setCloneSelection(availableIDs, src);
		} finally {
			tmcc.dispose();
		}

		update_info_bars();
	}

	public void setCloneSelection(long[] selectedIDs, CloneSelectionListener src) {
		lastShowingClonePairIndexByShowCloneCode = -1;
		setCloneSelection_i(selectedIDs, src);
	}

	public void setCloneSelectionByFlieAndOffset(int xFile, int xBeginPos,
			int xEndPos, int yFile, int yBeginPos, int yEndPos) {
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_CLONE_SETS")); //$NON-NLS-1$

		gnu.trove.TLongArrayList list = new gnu.trove.TLongArrayList();

		final Rectangle posRectX = new Rectangle(xBeginPos, yBeginPos, xEndPos - xBeginPos, yEndPos - yBeginPos);
		final Rectangle posRectY = new Rectangle(yBeginPos, xBeginPos, yEndPos - yBeginPos, xEndPos - xBeginPos);
		ClonePair[] clonePairs = currentScope.getClonePairsOfFile(xFile, yFile, yFile + 1);
		for (int i = 0; i < clonePairs.length; ++i) {
			ClonePair pair = clonePairs[i];
			Rectangle cloneRect = new Rectangle(pair.leftBegin, pair.rightBegin, 
					pair.leftEnd - pair.leftBegin, pair.rightEnd - pair.rightBegin);

			if (pair.rightFile == yFile && pair.leftFile == xFile && cloneRect.intersects(posRectX)) {
				list.add(pair.classID);
			}
			if (pair.leftFile == yFile && pair.rightFile == xFile && cloneRect.intersects(posRectY)) {
				list.add(pair.classID);
			}
		}

		long[] ary = list.toNativeArray();

		setCloneSelection_i(ary, null);

		statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
	}

	private void add_both_file_and_clone_set_metrics() {
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_BOTH_FILE_AND_CLONESET_METRICS"));  //$NON-NLS-1$
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			String cloneDataFile = currentScope.getCloneDataFilePath();
			if (cloneDataFile != null && cloneDataFile.length() > 0) {
				String fileMetricFile = cloneDataFile + ".fm.tmp"; //$NON-NLS-1$
				String cloneSetMetricFile = cloneDataFile + ".cm.tmp"; //$NON-NLS-1$
						
				final CCFinderX ccfx = CCFinderX.theInstance;
				final ArrayList<String> args = new ArrayList<String>();
				args.add("M"); //$NON-NLS-1$
				if (main.settingMaxWorkerThreads > 0) {
					args.add("--threads=" + String.valueOf(main.settingMaxWorkerThreads));  //$NON-NLS-1$
				}
				args.add(cloneDataFile);
				args.addAll(Arrays.asList(new String[] { "-p", "is" }));   //$NON-NLS-1$ //$NON-NLS-2$
				args.addAll(Arrays.asList(new String[] { "-f", "-o", fileMetricFile }));   //$NON-NLS-1$ //$NON-NLS-2$
				args.addAll(Arrays.asList(new String[] { "-c", "-o", cloneSetMetricFile }));   //$NON-NLS-1$ //$NON-NLS-2$
				int r = ccfx.invokeCCFinderX(args.toArray(new String[0]));
				if (r != 0) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_FAILURE_IN_CALCULATING_METRICS"));  //$NON-NLS-1$
					return;
				}
				TemporaryFileManager.registerFileToBeRemoved(fileMetricFile);

				metricModels.readFileMetricFileAndCloneSetMetricFile(fileMetricFile, this.currentScope.getMaxFileID(),
						cloneSetMetricFile, MainWindow.this.currentScope.getMaxCloneSetID());
				
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_FILE_TABLE")); //$NON-NLS-1$

				fileTable.addFileMetricModel(metricModels.getFileMetricModel());

				ScopeHistoryItem shi = scopeHistory.get(scopeHistory.size() - 1);
				shi.fileMetricFile = fileMetricFile;
				
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_CLONE_SET_TABLE")); //$NON-NLS-1$

				ClonesetMetricModel cloneSetMetricModel = metricModels.getCloneSetMetricModel();
				cloneSetTable.addCloneSetMetricModel(cloneSetMetricModel);

				shi.cloneSetMetricFile = cloneSetMetricFile;

				MainWindow.this.scatterPlotPane.updateModel(currentScope, cloneSetMetricModel);
			}
		} finally {
			tmcc.dispose();
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}
	}

	private void add_file_metrics() {
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_FILE_METRICS")); //$NON-NLS-1$
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			String cloneDataFile = currentScope.getCloneDataFilePath();
			if (cloneDataFile != null && cloneDataFile.length() > 0) {
				String metricFile = cloneDataFile + ".fm.tmp"; //$NON-NLS-1$
				final CCFinderX ccfx = CCFinderX.theInstance;
				final ArrayList<String> args = new ArrayList<String>();
				args.add("M");  //$NON-NLS-1$
				if (main.settingMaxWorkerThreads > 0) {
					args.add("--threads=" + String.valueOf(main.settingMaxWorkerThreads));  //$NON-NLS-1$
				}
				args.add(cloneDataFile);
				args.addAll(Arrays.asList(new String[] { "-f", "-o", metricFile }));   //$NON-NLS-1$ //$NON-NLS-2$
				args.addAll(Arrays.asList(new String[] { "-p", "is" }));   //$NON-NLS-1$ //$NON-NLS-2$
				int r = ccfx.invokeCCFinderX(args.toArray(new String[0]));
				if (r != 0) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_FAILURE_IN_CALCULATING_FILE_METRICS")); //$NON-NLS-1$
					return;
				}
				TemporaryFileManager.registerFileToBeRemoved(metricFile);

				metricModels.readFileMetricFile(metricFile, this.currentScope.getMaxFileID());

				statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_FILE_TABLE")); //$NON-NLS-1$

				fileTable.addFileMetricModel(metricModels.getFileMetricModel());

				ScopeHistoryItem shi = scopeHistory.get(scopeHistory.size() - 1);
				shi.fileMetricFile = metricFile;
			}
		} finally {
			tmcc.dispose();
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}
	}

	public void addBothFileAndCloneSetMetrics() {
		if (metricModels.getFileMetricModel() == null) {
			if (metricModels.getCloneSetMetricModel() == null) {
				add_both_file_and_clone_set_metrics();
			}
			else {
				add_file_metrics();
			}
		} else {
			if (metricModels.getCloneSetMetricModel() == null) {
				add_clone_set_metrics();
			}
		}
	}

	public void showLinebasedMetrics() {
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_LINE_BASED_METRICS"));  //$NON-NLS-1$
		try {
			String cloneDataFile = currentScope.getCloneDataFilePath();
			if (cloneDataFile != null && cloneDataFile.length() > 0) {
				String metricFile = cloneDataFile + ".lm.tmp"; //$NON-NLS-1$
				final CCFinderX ccfx = CCFinderX.theInstance;
				final String[] args = {
						"M", //$NON-NLS-1$
						cloneDataFile,
						"-w", //$NON-NLS-1$
						"-o", metricFile, //$NON-NLS-1$
						"-p", "is", //$NON-NLS-1$ //$NON-NLS-2$
				};
				int r = ccfx.invokeCCFinderX(args);
				if (r != 0) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_FAILURE_IN_CALCULATING_LINE_BASD_METRICS"));  //$NON-NLS-1$
					return;
				}
				TemporaryFileManager.registerFileToBeRemoved(metricFile);
				
				final model.LinebasedMetricModel model = new model.LinebasedMetricModel();
				try {
					model.readLinebasedMetricFile(metricFile, this.currentScope.getMaxFileID());
				} catch (IOException e) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_FAILURE_IN_READING_LINE_BASD_METRICS"));  //$NON-NLS-1$
					return;
				}
				
				String[] metricNames = model.getMetricNames();
				boolean[] floatingPointMap = model.getFlotingPointValueMap();
				MetricsSummaryData summaryData = model.getSummaryData();
				
				show_metrics_summary_values_with_total(Messages.getString("gemx.MainWindow.S_LINE_BASED_METRICS"), metricNames, floatingPointMap, summaryData);  //$NON-NLS-1$
			}
		} finally {
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}
	}

	private void show_metrics_summary_values_with_total(String title, String[] metricNames, boolean[] floatingPointMap, MetricsSummaryData summaryData) {
		final int len = metricNames.length;
		assert floatingPointMap.length == len;
		final Double[] totalValues = summaryData.getTotalValues();
		final double[] minValues = summaryData.getMinValues();
		final double[] maxValues = summaryData.getMaxValues();
		final double[] aveValues = summaryData.getAverageValues();
		
		TotalMinMaxAveShowingDialog settingDialog = new TotalMinMaxAveShowingDialog(shell, clipboard, metricNames.length);
		settingDialog.setText(title);

		for (int i = 0; i < metricNames.length; ++i) {
			settingDialog.setName(i, metricNames[i]);
			if (floatingPointMap[i]) {
				if (totalValues[i] != null) {
					settingDialog.setTotalMinMaxAve(i, (long)Math.floor(totalValues[i] * 1000), (int)Math.floor(minValues[i] * 1000), (int)Math.ceil(maxValues[i] * 1000), 3, aveValues[i]);
				} else {
					settingDialog.setTotalMinMaxAve(i, null, (int)Math.floor(minValues[i] * 1000), (int)Math.ceil(maxValues[i] * 1000), 3, aveValues[i]);
				}
			} else {
				if (totalValues[i] != null) {
					settingDialog.setTotalMinMaxAve(i, (long)(double)totalValues[i], (int)(double)minValues[i], (int)(double)maxValues[i], 0, aveValues[i]);
				} else {
					settingDialog.setTotalMinMaxAve(i, null, (int)(double)minValues[i], (int)(double)maxValues[i], 0, aveValues[i]);
				}
			}
		}

		settingDialog.open();
	}
	
	private void show_metrics_summary_values(String title, String[] metricNames, boolean[] floatingPointMap, MetricsSummaryData summaryData) {
		final int len = metricNames.length;
		assert floatingPointMap.length == len;
		final double[] minValues = summaryData.getMinValues();
		final double[] maxValues = summaryData.getMaxValues();
		final double[] aveValues = summaryData.getAverageValues();
		
		MinMaxAveShowingDialog settingDialog = new MinMaxAveShowingDialog(shell, clipboard, metricNames.length);
		settingDialog.setText(title);

		for (int i = 0; i < metricNames.length; ++i) {
			settingDialog.setName(i, metricNames[i]);
			if (floatingPointMap[i]) {
				settingDialog.setMinMaxAve(i, (int)Math.floor(minValues[i] * 1000), (int)Math.ceil(maxValues[i] * 1000), 3, aveValues[i]);
			} else {
				settingDialog.setMinMaxAve(i, (int)(double)minValues[i], (int)(double)maxValues[i], 0, aveValues[i]);
			}
		}

		settingDialog.open();
	}
	
	public void addFileMetrics() {
		if (metricModels.getFileMetricModel() == null) {
			add_file_metrics();
		}
		if (metricModels.getFileMetricModel() == null) {
			return;
		}

		FileMetricModel fileMetricModel = metricModels.getFileMetricModel();
		String[] metricNames = fileMetricModel.getMetricNames();
		boolean[] floatingPointMap = fileMetricModel.getFlotingPointValueMap();
		MetricsSummaryData summaryData = fileMetricModel.getSummaryData();
		
		show_metrics_summary_values(Messages.getString("gemx.MainWindow.S_FILE_METRICS"), metricNames, floatingPointMap, summaryData); //$NON-NLS-1$
	}

	private void add_clone_set_metrics() {
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_CALCULATING_CLONE_SET_METRICS")); //$NON-NLS-1$
		TemporaryMouseCursorChanger tmcc = new TemporaryMouseCursorChanger(shell);
		try {
			String cloneDataFile = currentScope.getCloneDataFilePath();
			if (cloneDataFile != null && cloneDataFile.length() > 0) {
				String metricFile = cloneDataFile + ".cm.tmp"; //$NON-NLS-1$
				final CCFinderX ccfx = CCFinderX.theInstance;
				final ArrayList<String> args = new ArrayList<String>();
				args.add("M"); //$NON-NLS-1$
				if (main.settingMaxWorkerThreads > 0) {
					args.add("--threads=" + String.valueOf(main.settingMaxWorkerThreads));  //$NON-NLS-1$
				}
				args.add(cloneDataFile);
				args.addAll(Arrays.asList(new String[] { "-c", "-o", metricFile }));   //$NON-NLS-1$ //$NON-NLS-2$
				args.addAll(Arrays.asList(new String[] { "-p", "is" }));   //$NON-NLS-1$ //$NON-NLS-2$
				int r = ccfx.invokeCCFinderX(args.toArray(new String[0]));
				if (r != 0) {
					showErrorMessage(Messages.getString("gemx.MainWindow.S_FAILURE_IN_CALCULATING_CLONE_SET_METRICS")); //$NON-NLS-1$
				}
				TemporaryFileManager.registerFileToBeRemoved(metricFile);

				metricModels.readCloneSetMetricFile(metricFile, MainWindow.this.currentScope.getMaxCloneSetID());

				statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_CLONE_SET_TABLE")); //$NON-NLS-1$

				ClonesetMetricModel cloneSetMetricModel = metricModels.getCloneSetMetricModel();
				cloneSetTable.addCloneSetMetricModel(cloneSetMetricModel);

				ScopeHistoryItem shi = scopeHistory.get(scopeHistory.size() - 1);
				shi.cloneSetMetricFile = metricFile;

				MainWindow.this.scatterPlotPane.updateModel(currentScope, cloneSetMetricModel);
			}
		} finally {
			tmcc.dispose();
			statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
		}
	}

	public void addCloneSetMetrics() {
		if (metricModels.getCloneSetMetricModel() == null) {
			//cloneSetMetricModel.dispose();
			add_clone_set_metrics();
		}
		if (metricModels.getCloneSetMetricModel() == null) {
			return;
		}

		ClonesetMetricModel cloneSetMetricModel = metricModels.getCloneSetMetricModel();
		String[] metricNames = cloneSetMetricModel.getMetricNames();
		boolean[] floatingPointMap = cloneSetMetricModel.getFlotingPointValueMap();
		MetricsSummaryData summaryData = cloneSetMetricModel.getSummaryData();
		
		show_metrics_summary_values(Messages.getString("gemx.MainWindow.S_CLONE_SET_METRICS"), metricNames, floatingPointMap, summaryData); //$NON-NLS-1$
	}
	
	private void buildStatusBar() {
		SashForm sash = new SashForm(shell, SWT.HORIZONTAL);
		{
			GridData gridData = new GridData();
			gridData.horizontalAlignment = GridData.FILL;
			gridData.verticalAlignment = GridData.FILL;
			gridData.grabExcessHorizontalSpace = true;
			gridData.grabExcessVerticalSpace = false;
			gridData.horizontalSpan = 0;
			sash.setLayoutData(gridData);
		}

		statusBar = new Label(sash, SWT.LEFT);
		{
			GridData gridData = new GridData();
			gridData.horizontalAlignment = GridData.FILL;
			statusBar.setLayoutData(gridData);
		}
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_INITIALIZED")); //$NON-NLS-1$

		preprocessorBar = new Label(sash, SWT.LEFT);
		{
			GridData gridData = new GridData();
			gridData.horizontalAlignment = GridData.FILL;
			gridData.widthHint = 80;
			preprocessorBar.setLayoutData(gridData);
		}
		preprocessorBar.setText("PS: -"); //$NON-NLS-1$

		fileInfoBar = new Label(sash, SWT.LEFT);
		{
			GridData gridData = new GridData();
			gridData.horizontalAlignment = GridData.FILL;
			gridData.widthHint = 250;
			fileInfoBar.setLayoutData(gridData);
		}
		fileInfoBar.setText("#Files: -"); //$NON-NLS-1$

		cloneSetInfoBar = new Label(sash, SWT.LEFT);
		{
			GridData gridData = new GridData();
			gridData.horizontalAlignment = GridData.FILL;
			gridData.widthHint = 250;
			cloneSetInfoBar.setLayoutData(gridData);
		}
		cloneSetInfoBar.setText("#Clone Sets: -"); //$NON-NLS-1$

		searchBox = new Searchbox(sash, SWT.NONE);
		searchBox.setLayoutData(new GridData(GridData.GRAB_HORIZONTAL));
		searchBox.addSearchboxListener(new SearchboxListener() {
			public void searchCanceled(SearchboxData data) {
				final SearchboxListener target = MainWindow.this.getSearchboxTarget();
				if (target != null) {
					target.searchCanceled(data);
				}
			}
			public void searchForward(SearchboxData data) {
				final SearchboxListener target = MainWindow.this.getSearchboxTarget();
				if (target != null) {
					target.searchForward(data);
				}
			}
			public void searchBackward(SearchboxData data) {
				final SearchboxListener target = MainWindow.this.getSearchboxTarget();
				if (target != null) {
					target.searchBackward(data);
				}
			}
		});

		sash.setWeights(new int[] { 3, 1, 2, 2, 4 });
	}

	private void buildMenu() {
		final Menu menubar = new Menu(shell, SWT.BAR);
		shell.setMenuBar(menubar);

		{
			MenuItem mItemFile = new MenuItem(menubar, SWT.CASCADE);
			mItemFile.setText(Messages.getString("gemx.MainWindow.M_FILE")); //$NON-NLS-1$

			Menu menuFile = new Menu(mItemFile);
			mItemFile.setMenu(menuFile);

			MenuItem mItemDetect = new MenuItem(menuFile, SWT.PUSH);
			mItemDetect.setText(Messages.getString("gemx.MainWindow.M_DETECT_CLONES")); //$NON-NLS-1$
			mItemDetect.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					do_detect_clones();
				}
			});

			MenuItem mItemReDetect = new MenuItem(menuFile, SWT.PUSH);
			mItemReDetect.setText(Messages.getString("gemx.MainWindow.M_RE_DETECT_CLONES")); //$NON-NLS-1$
			mItemReDetect.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					do_re_detect_clones();
				}
			});

			MenuItem mItemDetectForFileList = new MenuItem(menuFile, SWT.PUSH);
			mItemDetectForFileList.setText(Messages.getString("gemx.MainWindow.M_DETECT_CLONES_FROM_FILE_LIST")); //$NON-NLS-1$
			mItemDetectForFileList.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					doDetectClonesFromFileList(false);
				}
			});

			MenuItem mItemDetectForFilesInClipboard = new MenuItem(menuFile, SWT.PUSH);
			mItemDetectForFilesInClipboard.setText(Messages.getString("gemx.MainWindow.M_DETECT_CLONES_FROM_FILES_IN_CLIPBOARD")); //$NON-NLS-1$
			mItemDetectForFilesInClipboard.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					doDetectClonesFromFileList(true);
				}
			});

			new MenuItem(menuFile, SWT.SEPARATOR);

			MenuItem mItemOpen = new MenuItem(menuFile, SWT.PUSH);
			mItemOpen.setText(Messages.getString("gemx.MainWindow.M_OPEN_CLONE_DATA")); //$NON-NLS-1$
			mItemOpen.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					do_open_clone_data_file();
				}
			});

			MenuItem mItemSave = new MenuItem(menuFile, SWT.PUSH);
			mItemSave.setText(Messages.getString("gemx.MainWindow.M_SAVE_CURRENT_SCOPE_AS")); //$NON-NLS-1$
			mItemSave.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					do_save_clone_data_as();
				}
			});

			new MenuItem(menuFile, SWT.SEPARATOR);

			MenuItem mSaveFileList = new MenuItem(menuFile, SWT.PUSH);
			mSaveFileList.setText(Messages.getString("gemx.MainWindow.M_SAVE_FILE_LIST_AS")); //$NON-NLS-1$
			mSaveFileList.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					do_save_file_list_as();
				}
			});

			new MenuItem(menuFile, SWT.SEPARATOR);

			MenuItem mSaveScatterplotImage = new MenuItem(menuFile, SWT.PUSH);
			mSaveScatterplotImage.setText("Save &scatterplot image as...");
			mSaveScatterplotImage.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					do_save_scatterplot_as();
				}
			});

			new MenuItem(menuFile, SWT.SEPARATOR);

			MenuItem mItemExit = new MenuItem(menuFile, SWT.PUSH);
			mItemExit.setText(Messages.getString("gemx.MainWindow.M_EXIT")); //$NON-NLS-1$
			mItemExit.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					MainWindow.this.do_exit();
				}
			});
		}
		{
			MenuItem mItemEdit = new MenuItem(menubar, SWT.CASCADE);
			mItemEdit.setText(Messages.getString("gemx.MainWindow.M_EDIT")); //$NON-NLS-1$

			Menu menuEdit = new Menu(mItemEdit);
			mItemEdit.setMenu(menuEdit);

			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_FIND"));  //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | 'F');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.setFocusToSearchbox();
					}
				});
			}
			new MenuItem(menuEdit, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_COPY_CTRL_C")); //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | 'C');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						new EditMenuHandlerImpl().copy();
					}
				});
			}
			new MenuItem(menuEdit, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_SELECTALL_CTRL_A")); //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | 'A');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						new EditMenuHandlerImpl().selectAll();
					}
				});
			}
			new MenuItem(menuEdit, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_ADD_CHECK_MARKS_TO_SELCTED_ITEMS"));  //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | 'M');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						new EditMenuHandlerImpl().addCheckMarksToSelectedItems();
					}
				});
			}
			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_REMOVE_CHECK_MARKS_FROM_SELECTED_ITEMS"));  //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | SWT.SHIFT | 'M');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						new EditMenuHandlerImpl().removeCheckMarksFromSelectedItems();
					}
				});
			}
			new MenuItem(menuEdit, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_INVERT_CHECK_MARKS"));  //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | 'I');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						new EditMenuHandlerImpl().invertCheckMarks();
					}
				});
			}
			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_CLEAR_CHECK_MARKS"));  //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | SWT.SHIFT | 'I');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						new EditMenuHandlerImpl().clearCheckMarks();
					}
				});
			}
			new MenuItem(menuEdit, SWT.SEPARATOR);
			{
				MenuItem pitem = new MenuItem(menuEdit, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_SELECT_CHECKED_ITEMS"));  //$NON-NLS-1$
				pitem.setAccelerator(SWT.CTRL | 'S');
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						new EditMenuHandlerImpl().selectCheckedItems();
					}
				});
			}
		}
		{
			MenuItem mItemScope = new MenuItem(menubar, SWT.CASCADE);
			mItemScope.setText(Messages.getString("gemx.MainWindow.M_SCOPE")); //$NON-NLS-1$

			Menu menuScope = new Menu(mItemScope);
			mItemScope.setMenu(menuScope);

			{
				MenuItem pitem = new MenuItem(menuScope, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_FIT_SCOPE_TO_SELECTED_FILES")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.fit_scope_to_current_selected_files();
					}
				});
			}
//			{
//				MenuItem pitem = new MenuItem(menuScope, SWT.PUSH);
//				pitem.setText(Messages.getString("gemx.MainWindow.M_FIT_SCOPE_TO_FILES_EXCEPT_FOR_SELECTED_ONES")); //$NON-NLS-1$
//				pitem.setSelection(true);
//				pitem.addSelectionListener(new SelectionAdapter() {
//					public void widgetSelected(SelectionEvent e) {
//						MainWindow.this.fit_scope_to_current_unselected_files();
//					}
//				});
//			}
			{
				MenuItem pitem = new MenuItem(menuScope, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_SET_SCOPE_TO_CHECKED_FILES"));  //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.fit_scope_to_current_checked_files();
					}
				});
			}

			new MenuItem(menuScope, SWT.SEPARATOR);

			{
				MenuItem pitem = new MenuItem(menuScope, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_FIT_SCOPE_TO_SELECTED_CLONE_SETS")); //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.fit_scope_to_current_selected_cloneset_ids();
					}
				});
			}
//			{
//				MenuItem pitem = new MenuItem(menuScope, SWT.PUSH);
//				pitem.setText(Messages.getString("gemx.MainWindow.M_FIT_SCOPE_TO_CLONES_EXCEPT_FOR_SELECTED_ONES")); //$NON-NLS-1$
//				pitem.setSelection(true);
//				pitem.addSelectionListener(new SelectionAdapter() {
//					public void widgetSelected(SelectionEvent e) {
//						MainWindow.this.fit_scope_to_current_unselected_cloneset_ids();
//					}
//				});
//			}
			{
				MenuItem pitem = new MenuItem(menuScope, SWT.PUSH);
				pitem.setText(Messages.getString("gemx.MainWindow.M_FIT_SCOPE_TO_CHECKED_CLONES"));  //$NON-NLS-1$
				pitem.setSelection(true);
				pitem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.fit_scope_to_current_checked_cloneset_ids();
					}
				});
			}

			new MenuItem(menuScope, SWT.SEPARATOR);

			{
				MenuItem mItemPop = new MenuItem(menuScope, SWT.PUSH);
				mItemPop.setText(Messages.getString("gemx.MainWindow.M_POP_SCOPE")); //$NON-NLS-1$
				mItemPop.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.resetScope();
					}
				});
			}

			{
				MenuItem mItemReset = new MenuItem(menuScope, SWT.PUSH);
				mItemReset.setText(Messages.getString("gemx.MainWindow.M_RESET_SCOPE")); //$NON-NLS-1$
				mItemReset.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.resetScope();
					}
				});
			}
		}
		{
			MenuItem mItemMetric = new MenuItem(menubar, SWT.CASCADE);
			mItemMetric.setText(Messages.getString("gemx.MainWindow.M_METRICS")); //$NON-NLS-1$

			Menu menuMetric = new Menu(mItemMetric);
			mItemMetric.setMenu(menuMetric);

			{
				MenuItem mItemBothMetrics = new MenuItem(menuMetric, SWT.PUSH);
				mItemBothMetrics.setText(Messages.getString("gemx.MainWindow.M_ADD_BOTH_FILE_AND_CLONESET_METRICS_TO_TABLES"));  //$NON-NLS-1$
				mItemBothMetrics.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.addBothFileAndCloneSetMetrics();
					}
				});
			}
			
			new MenuItem(menuMetric, SWT.SEPARATOR);
			
			{
				MenuItem mItemFileMetrics = new MenuItem(menuMetric, SWT.PUSH);
				mItemFileMetrics.setText(Messages.getString("gemx.MainWindow.M_ADD_FILE_METRICS")); //$NON-NLS-1$
				mItemFileMetrics.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.addFileMetrics();
					}
				});
			}

			{
				MenuItem mItemFileFilteringByMetric = new MenuItem(menuMetric, SWT.PUSH);
				mItemFileFilteringByMetric.setText(Messages.getString("gemx.MainWindow.M_FILTER_FILE_BY_METRIC")); //$NON-NLS-1$
				mItemFileFilteringByMetric.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.doFilteringFilesByMetrics();
					}
				});
			}

			new MenuItem(menuMetric, SWT.SEPARATOR);

			MenuItem mItemCloneSetMetrics = new MenuItem(menuMetric, SWT.PUSH);
			mItemCloneSetMetrics.setText(Messages.getString("gemx.MainWindow.M_ADD_CLONE_SET_METRICS")); //$NON-NLS-1$
			mItemCloneSetMetrics.addSelectionListener(new SelectionAdapter() {
				public void widgetSelected(SelectionEvent e) {
					MainWindow.this.addCloneSetMetrics();
				}
			});

			{
				MenuItem mItemCloneSetFilteringByMetric = new MenuItem(menuMetric, SWT.PUSH);
				mItemCloneSetFilteringByMetric.setText(Messages.getString("gemx.MainWindow.M_FILTER_CLONE_SET_BY_METRIC")); //$NON-NLS-1$
				mItemCloneSetFilteringByMetric.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.doFilteringCloneSetByMetrics();
					}
				});
			}
			
			new MenuItem(menuMetric, SWT.SEPARATOR);
			
			{
				MenuItem mItemLinebasedMetrics = new MenuItem(menuMetric, SWT.PUSH);
				mItemLinebasedMetrics.setText(Messages.getString("gemx.MainWindow.M_SHOW_LINE_BASD_METRICS"));  //$NON-NLS-1$
				mItemLinebasedMetrics.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.showLinebasedMetrics();
					}
				});
			}
		}
		{
			MenuItem mItemHelp = new MenuItem(menubar, SWT.CASCADE);
			mItemHelp.setText(Messages.getString("gemx.MainWindow.M_SETTINGS")); //$NON-NLS-1$

			Menu menuHelp = new Menu(mItemHelp);
			mItemHelp.setMenu(menuHelp);

			{
				MenuItem mItem = new MenuItem(menuHelp, SWT.PUSH);
				mItem.setText(Messages.getString("gemx.MainWindow.M_CCFX_SETTINGS")); //$NON-NLS-1$
				mItem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.doCcfxSettingsDialog();
					}
				});
			}
			{
				MenuItem mItem = new MenuItem(menuHelp, SWT.PUSH);
				mItem.setText(Messages.getString("gemx.MainWindow.M_GEMX_SETTINGS")); //$NON-NLS-1$
				mItem.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.doGemxSettingsDialog();
					}
				});
			}
		}
		{
			MenuItem mItemView = new MenuItem(menubar, SWT.CASCADE);
			mItemView.setText(Messages.getString("gemx.MainWindow.M_VIEW"));  //$NON-NLS-1$

			Menu menuView = new Menu(mItemView);
			mItemView.setMenu(menuView);

			{
				MenuItem mItemFocusHistory = new MenuItem(menuView, SWT.PUSH);
				mItemFocusHistory.setText(Messages.getString("gemx.MainWindow.M_VIEW_SCOPE"));  //$NON-NLS-1$
				mItemFocusHistory.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.setFocusToControl(SC_SCOPE_HISTORY_LIST);
					}
				});
			}
			{
				MenuItem mItemFocusHistory = new MenuItem(menuView, SWT.PUSH);
				mItemFocusHistory.setText(Messages.getString("gemx.MainWindow.M_VIEW_FILE_TABLE"));  //$NON-NLS-1$
				mItemFocusHistory.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.setFocusToControl(SC_FILE_TABLE);
					}
				});
			}
			{
				MenuItem mItemFocusHistory = new MenuItem(menuView, SWT.PUSH);
				mItemFocusHistory.setText(Messages.getString("gemx.MainWindow.M_VIEW_CLONESET_TABLE"));  //$NON-NLS-1$
				mItemFocusHistory.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.setFocusToControl(SC_CLONE_SET_TABLE);
					}
				});
			}
			{
				MenuItem mItemFocusHistory = new MenuItem(menuView, SWT.PUSH);
				mItemFocusHistory.setText(Messages.getString("gemx.MainWindow.M_VIEW_SCATTER_PLOT"));  //$NON-NLS-1$
				mItemFocusHistory.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.setFocusToControl(SC_SCATTER_PLOT_PANE);
					}
				});
			}
			{
				MenuItem mItemFocusHistory = new MenuItem(menuView, SWT.PUSH);
				mItemFocusHistory.setText(Messages.getString("gemx.MainWindow.M_VIEW_SOURCE_TEXT"));  //$NON-NLS-1$
				mItemFocusHistory.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.setFocusToControl(SC_SOURCE_PANE);
					}
				});
			}
			{
				MenuItem mItemFocusHistory = new MenuItem(menuView, SWT.PUSH);
				mItemFocusHistory.setText(Messages.getString("gemx.MainWindow.M_VIEW_SCRAPBOOK"));  //$NON-NLS-1$
				mItemFocusHistory.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.setFocusToControl(SC_SCRAPBOOK);
					}
				});
			}
		}
		{
			MenuItem mItemHelp = new MenuItem(menubar, SWT.CASCADE);
			mItemHelp.setText(Messages.getString("gemx.MainWindow.M_HELP")); //$NON-NLS-1$

			Menu menuHelp = new Menu(mItemHelp);
			mItemHelp.setMenu(menuHelp);

			{
				MenuItem mItemDocument = new MenuItem(menuHelp, SWT.PUSH);
				mItemDocument.setText(Messages.getString("gemx.MainWindow.M_DOCUMENT_PAGE")); //$NON-NLS-1$
				mItemDocument.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						CCFinderX.theInstance.openOfficialSiteDocumentPage(Messages.getString("gemx.MainWindow.S_DOCUMENT_PAGE_LANG"), ""); //$NON-NLS-1$
					}
				});
			}
			
			{
				MenuItem mItemAbout = new MenuItem(menuHelp, SWT.PUSH);
				mItemAbout.setText(Messages.getString("gemx.MainWindow.M_ABOUT")); //$NON-NLS-1$
				mItemAbout.addSelectionListener(new SelectionAdapter() {
					public void widgetSelected(SelectionEvent e) {
						MainWindow.this.do_show_about_dialog();
					}
				});
			}
		}
	}

	private abstract class EditMenuHandler {
		public void copy() { }
		public void selectAll() { }
		public void addCheckMarksToSelectedItems() { }
		public void removeCheckMarksFromSelectedItems() { }
		public void invertCheckMarks() { }
		public void clearCheckMarks() { }
		public void selectCheckedItems() { }
	}

	private class EditMenuHandlerImpl extends EditMenuHandler {
		private EditMenuHandler targetNone = new EditMenuHandler() {
		};
		private EditMenuHandler targetScopeHistory = new EditMenuHandler() {
		};
		private EditMenuHandler targetFileTable = new EditMenuHandler() {
			@Override
			public void copy() { 
				fileTable.copyItemsToClipboard();
			}
			@Override
			public void selectAll() { 
				fileTable.selectAllItems();
			}
			@Override 
			public void addCheckMarksToSelectedItems() { 
				fileTable.getTableWithCheckHelper().addCheckMarksToSelectedItems();
			}
			@Override 
			public void removeCheckMarksFromSelectedItems() { 
				fileTable.getTableWithCheckHelper().removeCheckMarksFromSelectedItems();
			}
			@Override
			public void invertCheckMarks() { 
				fileTable.getTableWithCheckHelper().invertCheckMarks();
			}
			@Override
			public void clearCheckMarks() { 
				fileTable.getTableWithCheckHelper().clearCheckMarks();
			}
			@Override
			public void selectCheckedItems() { 
				fileTable.getTableWithCheckHelper().selectCheckedItems();
			}
		};
		private EditMenuHandler targetCloneSetTable = new EditMenuHandler() {
			@Override
			public void copy() { 
				cloneSetTable.copyItemsToClipboard();
			}
			@Override
			public void selectAll() { 
				cloneSetTable.selectAllItems();
			}
			@Override 
			public void addCheckMarksToSelectedItems() { 
				cloneSetTable.getTableWithCheckHelper().addCheckMarksToSelectedItems();
			}
			@Override 
			public void removeCheckMarksFromSelectedItems() { 
				cloneSetTable.getTableWithCheckHelper().removeCheckMarksFromSelectedItems();
			}
			@Override
			public void invertCheckMarks() { 
				cloneSetTable.getTableWithCheckHelper().invertCheckMarks();
			}
			@Override
			public void clearCheckMarks() { 
				cloneSetTable.getTableWithCheckHelper().clearCheckMarks();
			}
			@Override
			public void selectCheckedItems() { 
				cloneSetTable.getTableWithCheckHelper().selectCheckedItems();
			}
		};
		private EditMenuHandler targetScatterPlotPane = new EditMenuHandler() {
		};
		private EditMenuHandler targetSourcePane = new EditMenuHandler() {
			@Override
			public void copy() { 
				sourcePane.copyTextToClipboard();
			}
			@Override
			public void selectAll() { 
				sourcePane.selectEntireText();
			}
		};
		private EditMenuHandler targetScrapbook = new EditMenuHandler() {
			@Override
			public void copy() { 
				theScrapbook.copyTextToClipboard();
			}
			@Override
			public void selectAll() { 
				theScrapbook.selectEntireText();
			}
		};

		private EditMenuHandler getTarget() {
			switch (MainWindow.this.getSelectedControlIndex()) {
			case SC_NONE:
				return targetNone;
			case SC_SCOPE_HISTORY_LIST:
				return targetScopeHistory;
			case SC_FILE_TABLE:
				return targetFileTable;
			case SC_CLONE_SET_TABLE:
				return targetCloneSetTable;
			case SC_SCATTER_PLOT_PANE:
				return targetScatterPlotPane;
			case SC_SOURCE_PANE:
				return targetSourcePane;
			case SC_SCRAPBOOK:
				return targetScrapbook;
			}
			return null;
		}

		@Override
		public void copy() {
			getTarget().copy();
		}
		@Override
		public void selectAll() {
			getTarget().selectAll();
		}
		@Override
		public void addCheckMarksToSelectedItems() {
			getTarget().addCheckMarksToSelectedItems();
		}
		@Override
		public void removeCheckMarksFromSelectedItems() {
			getTarget().removeCheckMarksFromSelectedItems();
		}
		@Override
		public void invertCheckMarks() {
			getTarget().invertCheckMarks();
		}
		@Override
		public void clearCheckMarks() { 
			getTarget().clearCheckMarks();
		}
		@Override
		public void selectCheckedItems() {
			getTarget().selectCheckedItems();
		}
	}

	private SearchboxListener getSearchboxTarget() {
		switch (getSelectedControlIndex()) {
		case SC_FILE_TABLE:
			return fileTable.getSearchboxListener();
		case SC_CLONE_SET_TABLE:
			return cloneSetTable.getSearchboxListener();
		case SC_SOURCE_PANE:
			return sourcePane.getSearchboxListener();
		case SC_SCRAPBOOK:
			return theScrapbook.getSearchboxListener();
		default:
		case SC_NONE:
			return null;
		}
	}
	
	public void showScatterPlotPane() {
		mapFolder.setSelection(0);
	}
	public boolean isScatterPlotPaneShown() {
		return mapFolder.getSelection().getControl() == scatterPlotPane.getControl();
	}
	public void showTextPane() {
		mapFolder.setSelection(1);
	}
	public void showScrapbook() {
		mapFolder.setSelection(2);
	}
	public boolean isTextPaneShown() {
		return mapFolder.getSelection().getControl() == sourcePane.getControl();
	}
	public boolean isScrapbookShown() {
		return mapFolder.getSelection().getControl() == theScrapbook.getControl();
	}

	public void showFileTable() {
		listFolder.setSelection(0);
	}
	public boolean isFileTableShown() {
		return listFolder.getSelection().getControl() == fileTable.getControl();
	}
	public void showCloneSetTable() {
		listFolder.setSelection(1);
	}
	public boolean isCloneSetShown() {
		return listFolder.getSelection().getControl() == cloneSetTable.getControl();
	}

	public Control getSelectedControl() {
		if (selectedSashChildIndex < 0 || selectedSashChildIndex >= sashChildren.length) {
			return null;
		}
		final CTabItem selectedTabItem = sashChildren[selectedSashChildIndex].getSelection();
		return selectedTabItem.getControl();
	}
	
	public void setFocusToSearchbox() {
		this.searchBox.setFocus();
	}

	public void setFocusToControl(int controlIndex) {
		switch (controlIndex) {
		case SC_SCOPE_HISTORY_LIST:
			updateSelecetdTabFolder(sessionFolder);
			break;
		case SC_FILE_TABLE:
			updateSelecetdTabFolder(listFolder);
			MainWindow.this.showFileTable();
			break;
		case SC_CLONE_SET_TABLE:
			updateSelecetdTabFolder(listFolder);
			MainWindow.this.showCloneSetTable();
			break;
		case SC_SCATTER_PLOT_PANE:
			updateSelecetdTabFolder(mapFolder);
			MainWindow.this.showScatterPlotPane();
			break;
		case SC_SOURCE_PANE:
			updateSelecetdTabFolder(mapFolder);
			MainWindow.this.showTextPane();
			break;
		default:
			if (controlIndex >= SC_SCRAPBOOK) {
				assert controlIndex == SC_SCRAPBOOK;
				updateSelecetdTabFolder(mapFolder);
				MainWindow.this.showScrapbook();
			} else {
				assert false;
			}
		break;
		}
	}

	public int getSelectedControlIndex() {
		final Control selectedControl = getSelectedControl();
		if (selectedControl == null) {
			return SC_NONE;
		}
		if (selectedControl == this.scopeHistoryList) {
			return SC_SCOPE_HISTORY_LIST;
		}
		if (selectedControl == fileTable.getControl()) {
			return SC_FILE_TABLE;
		}
		if (selectedControl == cloneSetTable.getControl()) {
			return SC_CLONE_SET_TABLE;
		}
		if (selectedControl == scatterPlotPane.getControl()) {
			return SC_SCATTER_PLOT_PANE;
		}
		if (selectedControl == sourcePane.getControl()) {
			return SC_SOURCE_PANE;
		}
		if (selectedControl == theScrapbook.getControl()) {
			return SC_SCRAPBOOK;
		}
		return SC_NONE;
	}

	/*
	 * CustomCTabFolderはCTabFolderをハックしたもの。タブをクリックして選択されたCTabFolderを知るために、
	 * showItem()をオーバーライドしている
	 */
	private interface CustomCTabFolderSelectionListener {
		public void selected(CustomCTabFolder selectedFolder);
	}
	private class CustomCTabFolderSelectionAdapter implements CustomCTabFolderSelectionListener {
		public void selected(CustomCTabFolder selectedFolder) {
		}
	}
	private class CustomCTabFolder extends CTabFolder {
		private CustomCTabFolderSelectionListener listener;

		public void setSelectionListener(CustomCTabFolderSelectionListener listener) {
			this.listener = listener;
		}

		public CustomCTabFolder(Composite parent, int style) {
			super(parent, style);
			addListener(SWT.FocusIn, new Listener() {
				public void handleEvent(Event event) {
					if (listener != null) {
						listener.selected(CustomCTabFolder.this);
					}
				}
			});
		}

		@Override
		public void showItem (CTabItem item) {
			if (listener != null) {
				listener.selected(this);
			}
			super.showItem(item);
		}
	}

	public void open(Shell shell) {
		this.shell = shell;
		Display display = shell.getDisplay();
		this.clipboard = new Clipboard(shell.getDisplay());

		shell.setMaximized(true);
		shell.setText(String.format("GemX %d.%d.%d.%d", constants.ApplicationVersion.verMajor,  //$NON-NLS-1$
				constants.ApplicationVersion.verMinor1, constants.ApplicationVersion.verMinor2,
				constants.ApplicationVersion.verFix).toString());
		shell.setLayout(new GridLayout(1, true));

		shell.addShellListener(new ShellAdapter() {
			public void shellClosed(ShellEvent e) {
				MessageBox mes = new MessageBox(MainWindow.this.shell, SWT.OK | SWT.CANCEL
						| SWT.ICON_QUESTION);
				mes.setText("Exiting - GemX"); //$NON-NLS-1$
				mes.setMessage(Messages.getString("gemx.MainWindow.S_ARE_YOU_SURE_TO_EXIT_GEMX")); //$NON-NLS-1$
				if (mes.open() == SWT.CANCEL) {
					e.doit = false;
				}
			}
		});

		buildMenu();

		{
			SashForm sash = new SashForm(shell, SWT.HORIZONTAL);
			{
				GridData gridData = new GridData();
				gridData.horizontalAlignment = GridData.FILL;
				gridData.verticalAlignment = GridData.FILL;
				gridData.grabExcessHorizontalSpace = true;
				gridData.grabExcessVerticalSpace = true;
				sash.setLayoutData(gridData);
			}

			final CustomCTabFolderSelectionListener tabFolderSelectionListener = new CustomCTabFolderSelectionAdapter() {
				public void selected(CustomCTabFolder selectedFolder) {
					MainWindow.this.updateSelecetdTabFolder(selectedFolder);
				}
			};

			final boolean bAddResetScopeItemToContextMenu = main.settingResetScopeItemInContextMenus;

			sessionFolder = new CustomCTabFolder(sash, SWT.BORDER);
			final int tabHeight = sessionFolder.getTabHeight() + 3;
			{
				sessionFolder.setSelectionListener(tabFolderSelectionListener);
				sessionFolder.setSimple(false);
				sessionFolder.setTabHeight(tabHeight);

				final Listener focusedListener = new Listener() {
					public void handleEvent(Event event) {
						updateSelecetdTabFolder(sessionFolder);
					}
				};

				CTabItem tabItem1 = new CTabItem(sessionFolder, SWT.NONE);
				tabItem1.setText(Messages.getString("gemx.MainWindow.S_SCOPE_HISTORY")); //$NON-NLS-1$

				scopeHistoryList = new Table(sessionFolder, SWT.VIRTUAL | SWT.MULTI | SWT.FULL_SELECTION);
				scopeHistoryList.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
				scopeHistoryList.addListener(SWT.FocusIn, focusedListener);
				scopeHistoryList.addListener(SWT.MouseDoubleClick, focusedListener);

				scopeHistoryList.setHeaderVisible(true);
				String[] cols = { "", "Idx.", "Desc.", "#Files", "#Clones" }; //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$ //$NON-NLS-5$
				int[] colWids = { 10, 20, 90, 60, 60 };
				int[] colAligns = { SWT.LEFT, SWT.RIGHT, SWT.LEFT, SWT.RIGHT, SWT.RIGHT };
				for (int i = 0; i < cols.length; ++i) {
					TableColumn col = new TableColumn(scopeHistoryList, colAligns[i]);
					col.setText(cols[i]);
					col.setWidth(colWids[i]);
				}

				tabItem1.setControl(scopeHistoryList);
				{
					Menu pmenu = new Menu(shell, SWT.POP_UP);
					scopeHistoryList.setMenu(pmenu);

					{
						MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
						pitem.setText(Messages.getString("gemx.MainWindow.M_SHOW_SCOPE")); //$NON-NLS-1$
						pitem.setSelection(true);
						pitem.addSelectionListener(new SelectionAdapter() {
							public void widgetSelected(SelectionEvent e) {
								MainWindow.this.showScope(MainWindow.this.scopeHistoryList.getSelectionIndex());
							}
						});
					}
					new MenuItem(pmenu, SWT.SEPARATOR);
					{
						MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
						pitem.setText(Messages.getString("gemx.FileTable.M_POP_SCOPE")); //$NON-NLS-1$
						pitem.setSelection(true);
						pitem.addSelectionListener(new SelectionAdapter() {
							public void widgetSelected(SelectionEvent e) {
								MainWindow.this.popScope();
							}
						});
					}
					{
						MenuItem pitem = new MenuItem(pmenu, SWT.PUSH);
						pitem.setText(Messages.getString("gemx.MainWindow.M_PUSH_SCOPE")); //$NON-NLS-1$
						pitem.setSelection(true);
						pitem.addSelectionListener(new SelectionAdapter() {
							public void widgetSelected(SelectionEvent e) {
								MainWindow.this.pushScope(MainWindow.this.scopeHistoryList.getSelectionIndex());
							}
						});
					}
				}

				sessionFolder.setSelection(0);
			}

			{
				listFolder = new CustomCTabFolder(sash, SWT.BORDER);
				listFolder.setSelectionListener(tabFolderSelectionListener);
				listFolder.setSimple(false);
				listFolder.setTabHeight(tabHeight);

				final Listener focusedListener = new Listener() {
					public void handleEvent(Event event) {
						updateSelecetdTabFolder(listFolder);
					}
				};

				fileTable = widgetsFactory.newFileTable(listFolder, bAddResetScopeItemToContextMenu, this);
				fileTable.addListener(SWT.FocusIn, focusedListener);
				fileTable.addListener(SWT.MouseDown, focusedListener);
				{
					CTabItem tabItem1 = new CTabItem(listFolder, SWT.NONE);
					tabItem1.setText(Messages.getString("gemx.MainWindow.S_FILE_TABLE")); //$NON-NLS-1$
					tabItem1.setControl(fileTable.getControl());
				}

				cloneSetTable = widgetsFactory.newCloneSetTable(listFolder, bAddResetScopeItemToContextMenu, this);
				cloneSetTable.addListener(SWT.FocusIn, focusedListener);
				cloneSetTable.addListener(SWT.MouseDown, focusedListener);
				{
					CTabItem tabItem2 = new CTabItem(listFolder, SWT.NONE);
					tabItem2.setText(Messages.getString("gemx.MainWindow.S_CLONE_SET_TABLE")); //$NON-NLS-1$
					tabItem2.setControl(cloneSetTable.getControl());
				}

				listFolder.setSelection(0);
			}

			{
				mapFolder = new CustomCTabFolder(sash, SWT.BORDER);
				mapFolder.setSelectionListener(tabFolderSelectionListener);
				mapFolder.setSimple(false);
				mapFolder.setTabHeight(tabHeight);

				final Listener focusedListener = new Listener() {
					public void handleEvent(Event event) {
						updateSelecetdTabFolder(mapFolder);
					}
				};

				Rectangle rect = display.getClientArea();
				int shorter = rect.width;
				if (rect.height < shorter) {
					shorter = rect.height;
				}
				scatterPlotPane = widgetsFactory.newScatterPlotPane(mapFolder, shorter, bAddResetScopeItemToContextMenu, this);
				scatterPlotPane.addListener(SWT.FocusIn, focusedListener);
				scatterPlotPane.addListener(SWT.MouseDown, focusedListener);
				{
					CTabItem tabItem1 = new CTabItem(mapFolder, SWT.NONE);
					tabItem1.setText(Messages.getString("gemx.MainWindow.S_SCATTER_PLOT")); //$NON-NLS-1$
					tabItem1.setControl(scatterPlotPane.getControl());
				}

				sourcePane = widgetsFactory.newTextPane(mapFolder, this);
				sourcePane.addListener(SWT.FocusIn, focusedListener);
				sourcePane.addListener(SWT.MouseDown, focusedListener);
				String encodingName = main.settingEncoding;
				sourcePane.setEncoding(encodingName);
				{
					CTabItem tabItem2 = new CTabItem(mapFolder, SWT.NONE);
					tabItem2.setText(Messages.getString("gemx.MainWindow.S_SOURCE_TEXT")); //$NON-NLS-1$
					tabItem2.setControl(sourcePane.getControl());
				}

				theScrapbook = widgetsFactory.newTextPane(mapFolder, this);
				theScrapbook.addListener(SWT.FocusIn, focusedListener);
				theScrapbook.addListener(SWT.MouseDown, focusedListener);
				theScrapbook.setEncoding(encodingName);
				{
					CTabItem tabItem3 = new CTabItem(mapFolder, SWT.NONE);
					tabItem3.setText(Messages.getString("gemx.MainWindow.S_SCRAPBOOK"));  //$NON-NLS-1$
					tabItem3.setControl(theScrapbook.getControl());
				}
				theScrapbook.changeIndependentMode(true);

				mapFolder.setSelection(0);
			}

			sashChildren = new CustomCTabFolder[] { sessionFolder, listFolder, mapFolder };
			sash.setWeights(new int[] { 1, 8, 16 });
		}

		buildStatusBar();

		shell.pack();
		shell.open();
	}

	public void copyTextToScrapbook() {
		theScrapbook.copyData(sourcePane);
	}

	private void updateSelecetdTabFolder(CustomCTabFolder selectedTabFolder) {
		final Shell shell = MainWindow.this.shell;
		if (shell == null || sashChildren == null) {
			return;
		}
		final CTabFolder[] sashChildren = MainWindow.this.sashChildren;
		final Display display = shell.getDisplay();
		for (int j = 0; j < sashChildren.length; ++j) {
			final CTabFolder childJ = sashChildren[j];
			if (childJ == selectedTabFolder) {
				childJ.setSelectionForeground(display.getSystemColor(SWT.COLOR_TITLE_FOREGROUND));
				childJ.setSelectionBackground(
						new Color[]{
								display.getSystemColor(SWT.COLOR_TITLE_BACKGROUND),
								display.getSystemColor(SWT.COLOR_TITLE_BACKGROUND_GRADIENT)},
								new int[] {90},
								true);
				MainWindow.this.selectedSashChildIndex = j;
			} else {
				childJ.setSelectionForeground(display.getSystemColor(SWT.COLOR_WIDGET_FOREGROUND));
				childJ.setSelectionBackground(
						new Color[]{
								display.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND),
								display.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND)},
								new int[] {90},
								true);
			}
		}
	}
	
	public void resizeScatterPlotDrawingAreaToWindow(int times) {
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_RESIZING_SCATTER_PLOT_PANE")); //$NON-NLS-1$
		ClonesetMetricModel cloneSetMetricModel = metricModels.getCloneSetMetricModel();
		if (cloneSetMetricModel != null) {
			try {
				CloneSetMetricExtractor extractor = CloneSetMetricExtractor.newCloneSetMetricExtractorByID(cloneSetMetricModel, 
						model.ClonesetMetricModel.fieldNameToID("RNR"));  //$NON-NLS-1$
				scatterPlotPane.resizeDrawingAreaToWindow(currentScope, extractor, times);
			}
			catch (IndexOutOfBoundsException e) {
				scatterPlotPane.resizeDrawingAreaToWindow(currentScope, times);
			}
		}
		else {
			scatterPlotPane.resizeDrawingAreaToWindow(currentScope, times);
		}
		statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$
	}

	private void addScopeHistoryItem(String description, int fileCount, long cloneCount, boolean marked) {
		int index = scopeHistoryList.getItemCount();
		TableItem item = new TableItem(scopeHistoryList, SWT.NULL);
		String[] itemData = { 
				marked ? ">" : "", //$NON-NLS-1$ //$NON-NLS-2$
						String.valueOf(index),
						description,
						String.valueOf(fileCount),
						String.valueOf(cloneCount),
		};
		item.setText(itemData);
		if (marked) {
			setScopeHistoryMark(index);
		}
	}

	private void setScopeHistoryMark(int index) {
		for (int i = 0; i < scopeHistoryList.getItemCount(); ++i) {
			TableItem item = scopeHistoryList.getItem(i);
			String s = item.getText(0);
			if (i != index) {
				if (s.length() != 0) {
					item.setText(0, ""); //$NON-NLS-1$
				}
			} else {
				if (s.length() == 0) {
					item.setText(0, ">"); //$NON-NLS-1$
				}
			}
		}
	}

	public void pushScope(int index) {
		if (scopeHistory != null && 0 <= index && index < scopeHistory.size()) {
			Selection sel = save_selection();
			ScopeHistoryItem shi = scopeHistory.get(index);
			scopeHistory.add(shi);

			String recoveredModelFilePath = shi.cloneDataFile;
			Model recoveredModel = new Model();
			try {
				try {
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_READING_CLONE_DATA_FILE")); //$NON-NLS-1$
					recoveredModel.readCloneDataFile(recoveredModelFilePath);
				}
				catch (IOException e) {
					MessageBox box1 = new MessageBox(shell, SWT.OK | SWT.ICON_ERROR);
					box1.setText("Error - GemX"); //$NON-NLS-1$
					box1.setMessage(String.format(Messages.getString("gemx.MainWindow.S_CANNOT_READ_FILE"), recoveredModelFilePath)); //$NON-NLS-1$
					box1.open();
				}
				update_model(recoveredModel);
				metricModels.dispose();
				metricModels = new MetricModelsHolder(this);

				if (shi.fileMetricFile != null) {
					metricModels.readFileMetricFile(shi.fileMetricFile, this.currentScope.getMaxFileID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_FILE_TABLE")); //$NON-NLS-1$
					fileTable.addFileMetricModel(metricModels.getFileMetricModel());
				}

				if (shi.cloneSetMetricFile != null) {
					metricModels.readCloneSetMetricFile(shi.cloneSetMetricFile, this.currentScope.getMaxCloneSetID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_CLONE_SET_TABLE")); //$NON-NLS-1$
					cloneSetTable.addCloneSetMetricModel(metricModels.getCloneSetMetricModel());
					scatterPlotPane.updateModel(currentScope, metricModels.getCloneSetMetricModel());
				}

				statusBar.setText(Messages.getString("gemx.MainWindow.SB_RECOVERING_SELECTIONS")); //$NON-NLS-1$
				load_selection(sel);
				update_info_bars();
			}
			finally {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$

				this.addScopeHistoryItem("Scope :" + index, recoveredModel.getFileCount(), recoveredModel.getCloneSetCount(), true); //$NON-NLS-1$
			}
		}
	}

	public void showScope(int index) {
		if (scopeHistory != null && 0 <= index && index < scopeHistory.size()) {
			Selection sel = save_selection();
			ScopeHistoryItem shi = scopeHistory.get(index);

			String recoveredModelFilePath = shi.cloneDataFile;
			Model recoveredModel = new Model();
			try {
				try {
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_READING_CLONE_DATA_FILE")); //$NON-NLS-1$
					recoveredModel.readCloneDataFile(recoveredModelFilePath);
				}
				catch (IOException e) {
					MessageBox box1 = new MessageBox(shell, SWT.OK | SWT.ICON_ERROR);
					box1.setText("Error - GemX"); //$NON-NLS-1$
					box1.setMessage(String.format(Messages.getString("gemx.MainWindow.S_CANNOT_READ_FILE"), recoveredModelFilePath));   //$NON-NLS-1$
					box1.open();
				}
				update_model(recoveredModel);
				metricModels.dispose();
				metricModels = new MetricModelsHolder(this);

				if (shi.fileMetricFile != null) {
					metricModels.readFileMetricFile(shi.fileMetricFile, this.currentScope.getMaxFileID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_FILE_TABLE")); //$NON-NLS-1$
					fileTable.addFileMetricModel(metricModels.getFileMetricModel());
				}

				if (shi.cloneSetMetricFile != null) {
					metricModels.readCloneSetMetricFile(shi.cloneSetMetricFile, this.currentScope.getMaxCloneSetID());
					statusBar.setText(Messages.getString("gemx.MainWindow.SB_UPDATING_CLONE_SET_TABLE")); //$NON-NLS-1$
					cloneSetTable.addCloneSetMetricModel(metricModels.getCloneSetMetricModel());
					scatterPlotPane.updateModel(currentScope, metricModels.getCloneSetMetricModel());
				}

				statusBar.setText(Messages.getString("gemx.MainWindow.SB_RECOVERING_SELECTIONS")); //$NON-NLS-1$
				load_selection(sel);
				update_info_bars();
			}
			finally {
				statusBar.setText(Messages.getString("gemx.MainWindow.SB_READY")); //$NON-NLS-1$

				setScopeHistoryMark(index);
			}
		}
	}
}
