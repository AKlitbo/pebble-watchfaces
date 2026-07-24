# Host unit tests for the pure lib/c code. Run under WSL or any Linux with gcc.
# Each *.spec.c anywhere under lib/c is a standalone Unity program. `make` builds
# and runs them all and returns non-zero if any test fails, so it doubles as the
# CI gate. The spec files never reach the watch: waf_helpers.py drops them from the
# firmware glob (excl **/*.spec.c and spec/**).
#
# These cover lib/c/core only: date and moon and beats maths, number and text
# formatting, unit conversion. All of it plain arithmetic that needs no watch, which
# is why there is no fake SDK here and why the suite runs in about a second.
# Anything under lib/c/pebble talks to the SDK and is left to the device.

CC      ?= cc
# -Wno-unused-parameter keeps the SDK callback signatures (void *data and friends) quiet
CFLAGS  ?= -std=c11 -Wall -Wextra -Wno-unused-parameter -O2
INCLUDES = -I../core -Iunity

BUILD = build
UNITY = unity/unity.c

# every pure core source, minus the spec files themselves
CORE_SRC = $(shell find ../core -name '*.c' ! -name '*.spec.c')

# every colocated spec anywhere under lib/c (../ from here is lib/c)
SPECS = $(shell find .. -name '*.spec.c')

.PHONY: test clean
.DEFAULT_GOAL := test

# compile each spec fresh, then run it right away
test:
	@mkdir -p $(BUILD)
	@fail=0; \
	for spec in $(SPECS); do \
	  name=$$(basename $$spec .spec.c); \
	  bin=$(BUILD)/$$name; \
	  echo "== $$spec =="; \
	  $(CC) $(CFLAGS) $(INCLUDES) $$spec $(CORE_SRC) $(UNITY) -o $$bin || { fail=1; continue; }; \
	  $$bin || fail=1; \
	done; \
	exit $$fail

clean:
	rm -rf $(BUILD)
