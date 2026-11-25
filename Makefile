BUILDDIR = build
EXE = $(BUILDDIR)/libplugin.so

DOT_FILES := $(wildcard $(BUILDDIR)/*.dot)
PNG_DIR := $(BUILDDIR)/png
PNG_FILES := $(DOT_FILES:$(BUILDDIR)/%.dot=$(PNG_DIR)/%.png )

CC ?= gcc
CXX ?= g++
MPICC = mpicc

# DEBUG=1 enables the CFG dump (see 'make debug')
DEBUG ?= 0

# Added -Iinclude to resolve header files in the new directory
# -MMD -MP generate header dependencies so editing a header rebuilds the objects
PLUGIN_FLAGS = -I`$(CC) -print-file-name=plugin`/include -Iinclude -g -Wall -fno-rtti -fPIC -MMD -MP
CFLAGS = -g -O3 -Iinclude

ifeq ($(DEBUG),1)
PLUGIN_FLAGS += -DDEBUG
CFLAGS += -DDEBUG
endif

# Pointing to the new src/ directory
SRCS := $(wildcard src/*.cpp)
SRC_FILES = $(SRCS:src/%=%)
SRC_DIR = src/
OBJS = $(SRC_FILES:%.cpp=$(BUILDDIR)/%.o)
DEPS = $(OBJS:.o=.d)


.PHONY: all clean tests dot2png check debug help

all: $(EXE)


help:
	@echo "Usage : "
	@echo "  - Default target : compile plugin into a shared lib libplugin.so."
	@echo "  - debug : Compile the plugin with DEBUG flag which trigger .dot file generation. .dot files representes Control Flow Graph (CFG), can be converted to png with dot2png target."
	@echo "  - tests : compile test source files with the plugin enabled to test it."
	@echo "  - test-{fail,pass}-{1,2} : compile specific file to test the plugin."
	@echo "  - check : compile every test and verify its warnings (fail tests warn, pass tests do not)."
	@echo "  - dot2png : generate png file from .dot file."
	@echo "  - clean : clean the repo."


$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Clean first so every object is rebuilt with DEBUG defined
debug:
	$(MAKE) clean
	$(MAKE) DEBUG=1 $(EXE)
	@echo "Build debug done"

# Generic rule to build objects from the src folder
$(BUILDDIR)/%.o: src/%.cpp | $(BUILDDIR)
	$(CXX) $(PLUGIN_FLAGS) -c -o $@ $<

$(EXE): $(OBJS)
	$(CXX) -shared -o $@ $^

-include $(DEPS)

#############
## Tests ####
#############

TEST_SRCS := $(wildcard tests/test*.c)
TEST_SRCS_FILE := $(TEST_SRCS:tests/%=%)
TESTS := $(TEST_SRCS_FILE:%.c=$(BUILDDIR)/%)

# Build all tests
tests: $(TESTS)

# Generic build rule for tests inside the tests/ directory
$(BUILDDIR)/test-%: tests/test-%.c $(EXE) | $(BUILDDIR)
	$(MPICC) $< $(CFLAGS) -o $@ -fplugin=./$(EXE)

# Warning emitted by the plugin on an invalid fork
WARN_MSG := can cause an invalid fork

# Rebuild each test from scratch and check its warnings:
# test-fail-* must warn, test-pass-* must not. Exits non-zero on any failure.
check: $(EXE) | $(BUILDDIR)
	@status=0; \
	for src in $(TEST_SRCS); do \
	  t=$$(basename $$src .c); \
	  rm -f $(BUILDDIR)/$$t; \
	  if ! $(MAKE) --no-print-directory $(BUILDDIR)/$$t MPICC=$(MPICC) > $(BUILDDIR)/$$t.log 2>&1; then \
	    echo "FAIL: $$t did not compile"; cat $(BUILDDIR)/$$t.log; status=1; continue; \
	  fi; \
	  n=$$(grep -c "$(WARN_MSG)" $(BUILDDIR)/$$t.log); \
	  case $$t in \
	    test-fail-*) [ $$n -gt 0 ] || { echo "FAIL: $$t: expected a warning"; status=1; } ;; \
	    test-pass-*) [ $$n -eq 0 ] || { echo "FAIL: $$t: unexpected warning"; status=1; } ;; \
	  esac; \
	  echo "$$t: $$n warning(s)"; \
	done; \
	exit $$status

##########
## DOTS ##
##########

dot2png: $(PNG_FILES)

# Ensure png/ directory exists
$(PNG_DIR):
	mkdir -p $(PNG_DIR)

# Rule to generate png/filename.png from filename.dot
$(PNG_DIR)/%.png: $(BUILDDIR)/%.dot | $(PNG_DIR)
	dot -T png $< -o $@

###########
## CLEAN ##
###########

clean:
	rm -rvf $(EXE) $(TESTS) src/*.o $(BUILDDIR) *.dot $(PNG_DIR)

