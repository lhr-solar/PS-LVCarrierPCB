# crazy colors ------------------
RED=\033[0;31m
GREEN=\033[0;32m
ORANGE=\033[0;33m
BLUE=\033[0;34m
PURPLE=\033[0;35m
CYAN=\033[0;36m
LIGHTGRAY=\033[0;37m
DARKGRAY=\033[1;30m
YELLOW=\033[0;33m
NC=\033[0m # No Color
#---------------------------------


# Project Configuration ----------
TEST ?= main

PROJECT_TARGET ?= stm32g473xx

LSOM ?= U

# source and include directories
PROJECT_C_INCLUDES = $(wildcard */Inc)
PROJECT_C_SOURCES = $(wildcard */Src/*.c)

# build directories
PROJECT_BUILD_DIR = build

BUILD_MAKEFILE_DIR ?= Embedded-Sharepoint

# generate paths
MAKEFILE_DIR = $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

PROJECT_MAIN_DIR ?= core/Src/main

ifneq ($(TEST), main)
PROJECT_C_SOURCES := $(filter-out $(PROJECT_MAIN_DIR).c, $(PROJECT_C_SOURCES))
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES) tests/$(TEST)_test.c)
else
PROJECT_C_SOURCES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_SOURCES))
endif

# debug
ifeq ($(PRINT_DEBUG), true)
$(info SOURCES: $(PROJECT_C_SOURCES))
$(info INCLUDES: $(PROJECT_C_INCLUDES))
endif

# generate paths (cont.d)
PROJECT_C_INCLUDES := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_C_INCLUDES))
PROJECT_BUILD_DIR := $(addprefix $(MAKEFILE_DIR)/, $(PROJECT_BUILD_DIR))

# export variables 
export PROJECT_TARGET
export PROJECT_C_SOURCES
export PROJECT_C_INCLUDES
export PROJECT_BUILD_DIR


# Build --------------------------
ifeq ($(MAKECMDGOALS),)
default: build_code
else ifeq ($(MAKECMDGOALS), all)
all: build_code
else
%:
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) $(MAKECMDGOALS)
endif


build_code:
ifneq ($(TEST), main)
	@echo -e "Making ${PURPLE}$(PROJECT_TARGET)${NC} build for ${BLUE}TEST=${PURPLE}${TEST}${NC}"
else
	@echo -e "Making ${PURPLE}$(PROJECT_TARGET)${NC} build with ${ORANGE}no test.${NC}"
endif
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) all -j
	@echo -e "${GREEN}Compiled! Splendid! Jolly Good!!${NC}"
#---------------------------------


#-------------------------------
# Flash
flash:
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) flash

#-------------------------------
# Flash over UART
flash-uart:
	$(MAKE) -C $(BUILD_MAKEFILE_DIR) flash-uart


#-------------- 
# Documentation
# .PHONY: docs
# docs:
# 	cd $(BUILD_MAKEFILE_DIR) && mkdocs serve