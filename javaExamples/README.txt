
USING THE JAVA WRAPPER FOR ARIA
-------------------------------

A "wrapper" library for Java has been provided in the "java" directory.
This wrapper layer provides a Java API which simply makes calls using JNI 
into the regular Aria or ArNetworking "native" library.

To use the Aria Java wrapper you need to download a Java SDK. 

On Windows, the Sun J2SE JDK 1.7 (Java SE7) was used to build the wrapper.  
You can download it from:

    http://java.sun.com/javase/downloads

Find the Java SE Development Kit 7 and click "Download". (Only the JDK
is needed, ARIA does not need Java EE or any other extra Java technologies.)

The JDK also includes the runtime environment (JRE) that allows you to run 
Java programs.  If you only need the JRE, it can also
be obtained from <http://java.sun.com>.   

On Ubuntu, Ubuntu's "default-jdk" package was installed and used to build
the wrapper (which uses openjdk 6 on Ubuntu 12.04).

In Windows you must then put Aria's 'bin' directory into your PATH environment
variable so that Java can find the dlls needed to run the wrapper.  (Environment
variables are set in the System control panel. If you used the default
installation location, add "C:\Program Files\MobileRobots\Aria\bin" to PATH.)
(The java wrapper tries to load AriaJava.dll, which also requires AriaVC10.dll.)

For an example you can start the simulator, then enter the javaExamples 
directory and compile the simple Java example by using this command:

    javac -classpath ../java/Aria.jar simple.java

Then run it with this command in Windows:
    java -cp ../java/Aria.jar;. simple

or this one in Linux:
    java -cp ../java/Aria.jar:. simple

(The difference is the seperator in the class path: ';' vs ':'.)

The Java wrapper is not as well tested as Aria itself. If you encounter
problems using it, please notify the aria-users mailing list.

You can use the C++ API documentation in 'docs', or you can use javadoc
to generate a Java API reference based on the java source files in the 'java'
directory.



USING ARFUNCTOR IN JAVA
-----------------------

Since you can't have a "pointer" to a method in Java, to use ArFunctors,
you must define a subclass of ArFunctor which overrides invoke().  For 
functors that take arguments, you can only use specially defined 
ArFunctor subclasses, see java/com/mobilerobots/Aria/ArFunctor_*.java. 
(More can be added by adding them to wrapper.i and rebuilding the wrapper
using SWIG.)




REBUILDING THE JAVA WRAPPER
---------------------------

If you want to rebuild the Java wrapper you need to install SWIG:
On Ubuntu or Debian Linux, install with:
  sudo apt-get install swig
On Windows See <http://www.swig.org/download.html>. 
By default, the Makefile will run the command "swig" to invoke swig, but you 
can override this by setting a SWIG environment variable.

You then need to set the environment variable JAVA_INCLUDE to the include 
directory in your Java SDK. If the JDK tools are not in your system path,
you may also set JAVA_BIN to the bin directory in your Java SDK.   

If you installed Sun Java on Linux in "/usr/local/jdk", use 
"/usr/local/jdk/bin" for JAVA_BIN and "/usr/local/jdk/include" for 
JAVA_INCLUDE.  

If you installed "default-jdk" on Ubuntu, then set JAVA_INCLUDE to
"/usr/lib/jvm/default-java/include". (The JDK tools will be in your
system path.)

On Windows, use "C:\Program Files\Java\jdk1.6.0_10\bin" for 
JAVA_BIN and "C:\Program Files\Java\jdk1.6.0_10\include" for JAVA_INCLUDE.

In Linux, run 'make java' in the Aria directory.  

In Windows with Visual Studio, open the most recent AriaJava `sln` file
from the java directory. Select Tools menu -> Options -> Projects -> 
VC++ Directories and add your SWIG directory.  Select Release build 
configuration. Then build or rebuild the AriaJava project.

