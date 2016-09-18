export SRC_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# Compile flags
CXX = g++
CPPFLAGS = -I$(SRC_DIR) -I$(SRC_DIR)/include

SUBDIRS = lib tests

.PHONY: all build clean
all: build

build: $(addprefix build-,$(SUBDIRS))
build-%:
	CXX='$(CXX)' CPPFLAGS='$(CPPFLAGS)' $(MAKE) -C $* build

clean: $(addprefix clean-,$(SUBDIRS))
clean-%:
	$(MAKE) -C $* clean
