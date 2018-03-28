# A simple makefile to cause make to go look in the top directory. A simple
# convience.

all:
	$(MAKE) -C .. utils

clean: 
	$(MAKE) -C .. cleanUtils

%: %.cpp
	$(MAKE) -C .. utils/$@
