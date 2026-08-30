.PHONY: all clean cia 3dsx

all:
	$(MAKE) -C standalone

cia:
	$(MAKE) -C standalone cia

3dsx:
	$(MAKE) -C standalone

clean:
	$(MAKE) -C standalone clean
