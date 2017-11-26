
# Copyright (c) 2017, Franz Hollerer. All rights reserved.
# This code is licensed under the MIT License (MIT).
# See LICENSE file for full details.

# ---------------------------------------- project specific settings ---

TARGETS := boot appl
export BUILD_ROOT_DIR := ./build

# --------------------------------------- derived settings and rules ---

export PROJECT_ROOT_DIR ?= $(CURDIR)

export BUILD_TYPE ?= release

MAKEFILE := $(lastword $(MAKEFILE_LIST))

all:
	for i in $(TARGETS) ; do \
	    $(MAKE) -f $(MAKEFILE) CURRENT_TARGET=$$i $$i || exit 1; \
	done

debug:
	$(MAKE) -f $(MAKEFILE) BUILD_TYPE=debug

release:
	$(MAKE) -f $(MAKEFILE) BUILD_TYPE=release

ifdef CURRENT_TARGET

BUILD_DIR := $(BUILD_ROOT_DIR)/$(CURRENT_TARGET)

$(CURRENT_TARGET): $(BUILD_DIR)/CMakeLists.txt
	make -C $(BUILD_DIR)

$(BUILD_DIR)/CMakeLists.txt: \
    $(PROJECT_ROOT_DIR)/CMakeLists_$(CURRENT_TARGET).txt
	mkdir -p $(BUILD_DIR)
	cp $< $@
	cd $(BUILD_DIR) && \
	cmake -DPROJECT_ROOT_DIR=$(PROJECT_ROOT_DIR) \
	    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

endif

clean:
	rm -rf $(BUILD_ROOT_DIR)
