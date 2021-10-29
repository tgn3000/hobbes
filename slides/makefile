
SUBDIRS = $(shell for dir in `find . -maxdepth 1 -mindepth 1 -type d`; do echo $$dir; done)

all:
	for SUBDIR in $(SUBDIRS); do make -C $$SUBDIR; done
