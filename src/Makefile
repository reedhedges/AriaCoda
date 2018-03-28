# A simple $(MAKE)file to cause make to go look in the top directory. A simple
# convience.

all:
	$(MAKE) -C ..

dep:
	$(MAKE) -C .. dep

clean:
	$(MAKE) -C .. clean

examples:
	$(MAKE) -C .. examples

%: FORCE
	$(MAKE) -C .. $@

FORCE:
