del /s *.class

set PATH=%PATH%;"C:\Program Files (x86)\Java\jdk1.6.0_19\bin"
set JARS=H:\kamiya\prog\smith2008os\win32\swt.jar;H:\kamiya\prog\smith2008os\win32\trove-2.0.4.jar;H:\kamiya\prog\smith2008os\win32\pathjson.jar;H:\kamiya\prog\smith2008os\GemX\icu4j-localespi-4_0_1.jar;H:\kamiya\prog\smith2008os\GemX\icu4j-charsets-4_0_1.jar;H:\kamiya\prog\smith2008os\GemX\icu4j-4_0_1.jar
set OPTS=-g:none -target 1.5

javac %OPTS% -classpath %JARS%;. GemXMain.java
javac %OPTS% -classpath %JARS%;. ccfinderx\*.java
javac %OPTS% -classpath %JARS%;. gemx\*.java
javac %OPTS% -classpath %JARS%;. gemx\dialogs\*.java
javac %OPTS% -classpath %JARS%;. gemx\scatterplothelper\*.java
javac %OPTS% -classpath %JARS%;. model\*.java
javac %OPTS% -classpath %JARS%;. utility\*.java
javac %OPTS% -classpath %JARS%;. res\*.java
javac %OPTS% -classpath %JARS%;. constants\*.java
javac %OPTS% -classpath %JARS%;. customwidgets\*.java
javac %OPTS% -classpath %JARS%;. resources\*.java

del gemxlib.jar
jar cvf gemxlib.jar ccfinderx\*.class gemx\*.class gemx\*.png gemx\dialogs\*.class gemx\dialogs\*.png gemx\scatterplothelper\*.class model\*.class utility\*.class res\*.class res\messages.properties constants\*.class customwidgets\*.class resources\*.class

ren GemXMain.class GemXMain.xclass
del /s *.class
ren GemXMain.xclass GemXMain.class
