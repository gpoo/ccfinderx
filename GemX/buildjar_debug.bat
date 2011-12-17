del /s *.class

set JARS=H:\kamiya\prog\smith2008\win32\swt.jar;H:\kamiya\prog\smith2008\win32\trove-2.0.2.jar;H:\kamiya\prog\smith2008\win32\pathjson.jar
set OPTS=

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
jar cvf gemxlib.jar ccfinderx\*.class gemx\*.class gemx\*.png gemx\dialogs\*.class gemx\dialogs\*.png gemx\scatterplothelper\*.class model\*.class utility\*.class res\*.class res\messages.properties constants\*.class customwidgets\*.class resources\*.class resources\*.zip

rem del /s *.class
