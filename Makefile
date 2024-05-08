CC ?= gcc
# CC ?= tcc

C_VERSION = -std=c89
# C_VERSION = -std=c99

C_WARNING = \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wshadow \
	-Wbad-function-cast \
	-Wnested-externs \
	-Wdeclaration-after-statement \
	-Werror=vla \
	-Wno-nonnull-compare
#	-pedantic-errors

SRC = src libwally-core/include

INCLUDE_DIR = $(SRC)

# TEST = test

DEPFLAGS = -MD
# DEPFLAGS = -MP -MD

CC_FLAGS = \
	$(C_VERSION) \
	-O0 \
	$(C_WARNING) \
	$(foreach D,$(INCLUDE_DIR),-I$(D))

LIBRARIES_DIR = libwally-core/src/.libs libwally-core/src/secp256k1/.libs
LIB_DIR = $(foreach L,$(LIBRARIES_DIR),-L$(L))

LIBRARIES_FIL = wallycore secp256k1
LIB_FIL = $(foreach L,$(LIBRARIES_FIL),-l$(L))

# TARGET_DIR := libwally-core/src/.libs
# EXTERNAL_LIBS := ${TARGET_DIR}/libwallycore.a
# EXTERNAL_LDLIBS := -L${TARGET_DIR} $(patsubst lib%.a,-l%,$(notdir $(EXTERNAL_LIBS)))

FILES = $(foreach D,$(SRC),$(wildcard $(D)/*.c))
OBJECTS = $(patsubst %.c,%.o,$(FILES))
DEP_FILES = $(patsubst %.c,%.d,$(FILES))

OUT_NAME = bitscriptc

XDG_BIN_HOME ?= $(HOME)/.local/bin
MAKEFILE_DIR = $(shell dirname "$(realpath $(firstword $(MAKEFILE_LIST)))")

.PHONY: all diff clean install uninstall
# .PHONY: all diff clean test install uninstall

$(OUT_NAME): $(OBJECTS)
	$(CC) $(CC_FLAGS) $(LIB_DIR) -o $@ $^ $(LIB_FIL)
# 	$(CC) $(CC_FLAGS) $(LIB) -o $@ $^

%.o: %.c
	$(CC) $(CC_FLAGS) $(DEPFLAGS) -c -o $@ $<

install: $(OUT_NAME)
	cp -f $(MAKEFILE_DIR)/$(OUT_NAME) $(XDG_BIN_HOME)
#	ln -sf $(MAKEFILE_DIR)/$(OUT_NAME) $(XDG_BIN_HOME)

uninstall:
	rm -f $(XDG_BIN_HOME)/$(OUT_NAME)

all: $(OUT_NAME)
# all: $(OUT_NAME) test

diff:
	$(info git status and git diff --compact-summary) $(info )
	@git --no-pager status
	@git --no-pager diff --compact-summary
clean:
	rm -rf $(OBJECTS)
	rm -rf $(DEP_FILES)
	rm -rf $(OUT_NAME)

# include the dependencies
-include $(DEP_FILES)
