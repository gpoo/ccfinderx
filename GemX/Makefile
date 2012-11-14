CLASSPATH = $(shell echo extlibs/*.jar | tr ' ' ':'):.

.SUFFIXES: .java .class

.java.class:
	echo $<
	javac -classpath $(CLASSPATH) $<

SOURCES = \
	GemXMain.java \
	ccfinderx/CCFinderX.java \
	constants/ApplicationVersion.java \
	constants/CcfxDefaultSettings.java \
	constants/GemXDefaultSettings.java \
	res/Messages.java \
	resources/ImageManager.java \
	resources/TextColors.java \
	resources/MetricColors.java \
	customwidgets/Searchbox.java \
	customwidgets/SearchboxListener.java \
	customwidgets/SearchboxData.java \
	customwidgets/Messages.java \
	customwidgets/Spinner.java \
	model/CodeFragment.java \
	model/Model.java \
	model/CCFinderXInvocationException.java \
	model/SourceFileAndPosition.java \
	model/ClonePair.java \
	model/DataFileReadError.java \
	model/SourceFile.java \
	model/MetricsSummaryData.java \
	model/LinebasedMetricModel.java \
	model/CcfxDetectionOptions.java \
	model/ClonesetMetricModel.java \
	model/FileMetricModel.java \
	model/ClonesetAndFileIndex.java \
	model/CloneSet.java \
	gemx/ITextRuler.java \
	gemx/TextPaneScrollListener.java \
	gemx/Main.java \
	gemx/dialogs/CcfxSettingsDialog.java \
	gemx/dialogs/GemXSettingsDialog.java \
	gemx/dialogs/CloneReDetectionDialog.java \
	gemx/dialogs/TotalMinMaxAveShowingDialog.java \
	gemx/dialogs/CloneDetectionFromFileListOptionDialog.java \
	gemx/dialogs/MinMaxAveShowingDialog.java \
	gemx/dialogs/PreprocessScriptAndDirectoriesDialog.java \
	gemx/dialogs/CloneDetectionOptionDialog.java \
	gemx/dialogs/CcfxOptionsPanel.java \
	gemx/dialogs/DropdownDialog.java \
	gemx/dialogs/MinMaxSettingDialog.java \
	gemx/dialogs/ParentDirectorySelectDialog.java \
	gemx/dialogs/AboutDialog.java \
	gemx/ScopeHistoryItem.java \
	gemx/ScatterPlotPane.java \
	gemx/WidgetsFactory.java \
	gemx/FileTable.java \
	gemx/MainWindow.java \
	gemx/MultipleTextPane.java \
	gemx/TextPane.java \
	gemx/CloneSelectionListener.java \
	gemx/TextRuler.java \
	gemx/TableWithCheckHelper.java \
	gemx/scatterplothelper/PlottingColors.java \
	gemx/scatterplothelper/CloneSetMetricExtractor.java \
	gemx/FileSelectionListener.java \
	gemx/MetricModelsHolder.java \
	gemx/ClonesetTable.java \
	gemx/Messages.java \
	utility/DoublesParser.java \
	utility/Redirector.java \
	utility/SystemColorManager.java \
	utility/Picosel.java \
	utility/PrepReaderError.java \
	utility/Decoder.java \
	utility/StringChecker.java \
	utility/ExCloneable.java \
	utility/AssertUniq.java \
	utility/PrepToken.java \
	utility/TemporaryMouseCursorChanger.java \
	utility/PrepReader.java \
	utility/ArrayUtil.java \
	utility/FileUtils.java \
	utility/BitArray.java \
	utility/LittleEndianReader.java \
	utility/JsonHelper.java \
	utility/Pair.java \
	utility/ExecCommandline.java \
	utility/StringUtil.java \
	utility/ImageSaver.java \
	utility/IntMinMax.java \
	utility/PythonVersionChecker.java \
	utility/ExecutionModuleDirectory.java \
	utility/DoubleMinMax.java \
	utility/SetOperators.java \
	utility/TemporaryFileManager.java \
	utility/Messages.java

classes:= $(SOURCES:.java=.class)

resources:= $(wildcard gemx/*.png gemx/dialogs/*.png res/messages.properties)

clean_files:= $(SOURCES:.java=*.class) gemx/InitializationTask.class gemx/CCFinderXHelper.class

default: gemxlib

gemxlib: $(classes) $(resources)
	# For inner classes javac creates a different file per class:
	#    e.g. file.class, file$foo.class, file$bar.class, and so on.
	jar cvf $@.jar $(SOURCES:.java=*.class) $(resources)

run: gemxlib.jar
	java -classpath $(CLASSPATH):gemxlib.jar GemXMain

ccfinderx_CCFinderX_h: ccfinderx/CCFinderX.java
	javac $?
	javah -classpath . ccfinderx.CCFinderX

all: gemxlib ccfinderx_CCFinderX_h

clean:
	@# For inner classes javac creates a different file per class:
	@#    e.g. file.class, file$foo.class, file$bar.class, and so on.
	rm -f $(clean_files)

clean-all:
	rm -f $(clean_files) gemxlib.jar

.PHONY: clean gemxlib run all clean-all
