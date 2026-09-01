#!/bin/make
#===============================================================================
# File name   : Makefile
# Author      : Alexander Krysl (axkrysl47)
# Date        : 2026-09-28
# Version     : 1.0.0
# Description : Makefile for the GridLock Window Organization Utility.
#
# Copyright (c) 2026
#===============================================================================
# Choose your shell (e.g., make SHELL_TYPE=LINUX or SHELL_TYPE=WINDOWS)
SHELL_TYPE ?= LINUX
ifeq ($(SHELL_TYPE), WINDOWS)
    SHELL := cmd.exe
    MKDIR = if not exist $(subst /,\\,$(1)) mkdir $(subst /,\\,$(1))
    RM = if exist $(subst /,\\,$(1)) rmdir /s /q $(subst /,\\,$(1))
else
    SHELL := /bin/sh
    MKDIR = mkdir -p $(1)
    RM = rm -rf $(1)
endif

#===============================================================================
# Compiler / Linker
SOURCE_COMPILER = gcc
RESOURCE_COMPILER = windres

#-------------------------------------------------------------------------------
# Linker / Compiler / Library Flags
LINKER_FLAGS = -O3 -flto
SOURCE_COMPILER_FLAGS = -Wall -Wextra -fms-extensions -g $(LINKER_FLAGS) \
	-DVERSION_STRING=\"$(VERSION)\" -DWIN32_LEAN_AND_MEAN -DNOMINMAX
RESOURCE_COMPILER_FLAGS = \
	-DRELEASE_VERSION=$(if $(filter command line,$(origin VERSION)),1,0)
LIBRARIES = -Wl,-Bstatic -lyaml -Wl,-Bdynamic -luser32 -lshell32 -ldwmapi \
	-lgdi32 -lcomctl32

#-------------------------------------------------------------------------------
# Directory / Target Definitions
SOURCE_DIRECTORY = Source
RESOURCE_DIRECTORY = Resource
BUILD_DIRECTORY = Build
RELEASE_DIRECTORY = Release
EXECUTABLE_FILENAME = GridLock
VERSION = Unreleased

#-------------------------------------------------------------------------------
# Derived Files
SOURCE_FILES = $(wildcard $(SOURCE_DIRECTORY)/*.c)
OBJECT_FILES = \
	$(patsubst $(SOURCE_DIRECTORY)/%.c, $(BUILD_DIRECTORY)/%.o, $(SOURCE_FILES))
DEPENDENCIES = \
	$(patsubst $(SOURCE_DIRECTORY)/%.c, $(BUILD_DIRECTORY)/%.d, $(SOURCE_FILES))
RESOURCE_FILES = $(wildcard $(RESOURCE_DIRECTORY)/*.rc)
MANIFEST_FILES = $(wildcard $(RESOURCE_DIRECTORY)/*.manifest)
RESOURCE_OBJECT_FILES = \
	$(patsubst $(RESOURCE_DIRECTORY)/%.rc, $(BUILD_DIRECTORY)/%.o, \
	$(RESOURCE_FILES))

#===============================================================================
# Default target to make latest build
all: $(RELEASE_DIRECTORY)/$(EXECUTABLE_FILENAME).exe

#-------------------------------------------------------------------------------
# Link object files to create the final executable
$(RELEASE_DIRECTORY)/$(EXECUTABLE_FILENAME).exe: $(OBJECT_FILES) \
	$(RESOURCE_OBJECT_FILES) $(CONFIGURATION_OBJECT_FILE)
	@$(call MKDIR,$(RELEASE_DIRECTORY))
	$(SOURCE_COMPILER) $(SOURCE_COMPILER_FLAGS) -o \
	    $(RELEASE_DIRECTORY)/$(EXECUTABLE_FILENAME).exe \
	    $(LINKER_FLAGS) \
	    $(OBJECT_FILES) \
	    $(RESOURCE_OBJECT_FILES) \
	    $(CONFIGURATION_OBJECT_FILE) \
	    $(LIBRARIES)

#-------------------------------------------------------------------------------
# Compile object file from source files
$(BUILD_DIRECTORY)/%.o: $(SOURCE_DIRECTORY)/%.c
	@$(call MKDIR,$(BUILD_DIRECTORY))
	$(SOURCE_COMPILER) $(SOURCE_COMPILER_FLAGS) -c $< -o $@

#-------------------------------------------------------------------------------
# Compile the resource script into an object file
$(BUILD_DIRECTORY)/%.o: $(RESOURCE_DIRECTORY)/%.rc $(MANIFEST_FILES)
	@$(call MKDIR,$(BUILD_DIRECTORY))
	cd $(RESOURCE_DIRECTORY) && \
	$(RESOURCE_COMPILER) $(RESOURCE_COMPILER_FLAGS) $(notdir $<) -o ../$@

#-------------------------------------------------------------------------------
# Include dependencies
-include $(DEPENDENCIES)

#-------------------------------------------------------------------------------
# Clean build files
clean:
	@$(call RM,$(BUILD_DIRECTORY))
	@$(call RM,$(RELEASE_DIRECTORY))

#-------------------------------------------------------------------------------
# Release Target
release: LINKER_FLAGS += -DNDEBUG -mwindows -s
release: clean all

#-------------------------------------------------------------------------------
.PHONY: all clean release
