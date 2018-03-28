
ifdef JAVA_BIN
JAVAC=$(JAVA_BIN)/javac
JAVA=$(JAVA_BIN)/java
else
JAVAC=javac
JAVA=java
endif

#### Different options for Windows or Linux:
host:=$(shell uname | cut -d _ -f 1)
ifeq ($(host),MINGW32)  # host is MINGW32 on Windows:
CLASSPATH_SEP=;
RUNTIME_PATH_VAR=PATH
RUNTIME_LIB_DIR=../bin
LIB_EXT=dll
LIB_PREFIX=
else
CLASSPATH_SEP=:
RUNTIME_PATH_VAR=LD_LIBRARY_PATH
RUNTIME_LIB_DIR=../lib
LIB_EXT=so
LIB_PREFIX=lib
endif



all: $(patsubst %.java,%.class,$(wildcard *.java))

%.class: %.java ../java/Aria.jar $(RUNTIME_LIB_DIR)/$(LIB_PREFIX)Aria.$(LIB_EXT)
	@echo $^
	"$(JAVAC)" -classpath ../java/Aria.jar $<

run%: %.class
	$(RUNTIME_PATH_VAR)=$$$(RUNTIME_PATH_VAR):$(RUNTIME_LIB_DIR) "$(JAVA)" -cp ../java/Aria.jar:. $* $(ARGS)

gdb%: %.class
	$(RUNTIME_PATH_VAR)=$$$(RUNTIME_PATH_VAR):$(RUNTIME_LIB_DIR) gdb --args "$(JAVA)" -cp ../java/Aria.jar:. $* $(ARGS)

info:
	@echo host=$(host)
	@echo "JAVA_BIN=$(JAVA_BIN)"
	@echo "JAVAC=$(JAVAC)"
	@echo "JAVA=$(JAVA)"
	@echo "CLASSPATH_SEP=$(CLASSPATH_SEP)"
	@echo "RUNTIME_PATH_VAR=$(RUNTIME_PATH_VAR)"
	@echo "RUNTIME_LIB_DIR=$(RUNTIME_LIB_DIR)"
	@echo "LIB_EXT=$(LIB_EXT)"
