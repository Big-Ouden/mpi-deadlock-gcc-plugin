BUILDDIR = build
EXE = $(BUILDDIR)/libplugin.so

DOT_FILES := $(wildcard $(BUILDDIR)/*.dot)
PNG_DIR := $(BUILDDIR)/png
PNG_FILES := $(DOT_FILES:$(BUILDDIR)/%.dot=$(PNG_DIR)/%.png )

CC ?= gcc
CXX ?= g++
MPICC = mpicc

# Added -Iinclude to resolve header files in the new directory
PLUGIN_FLAGS = -I`$(CC) -print-file-name=plugin`/include -Iinclude -g -Wall -fno-rtti -fPIC 
CFLAGS = -g -O3 -Iinclude

# Pointing to the new src/ directory
SRCS := $(wildcard src/*.cpp)
SRC_FILES = $(SRCS:src/%=%)
SRC_DIR = src/
OBJS = $(SRC_FILES:%.cpp=$(BUILDDIR)/%.o)


.PHONY: clean cleanall tests dot2png

TEST_SRCS := $(wildcard tests/test*.c)
TEST_SRCS_FILE := $(TEST_SRCS:tests/%=%)
TESTS := $(TEST_SRCS_FILE:%.c=$(BUILDDIR)/%)

all: $(EXE)
	# @echo -e "$(OBJS)"
	# @echo -e "$(SRCS)"
	# @echo -e "$(SRC_FILES)"
	@echo -e "$(PNG_FILES)"
	@echo -e "$(PNG_DIR)"
	@echo -e "$(DOT_FILES)"
	@echo -e "coucou"
	@echo -e $(DOT_FILES)
	@echo -e $(PNG_DIR)
	@echo -e $(PNG_FILES)
	@echo "%%%%%%%%%%%%%%%%%%%"
	@echo -e $(TEST_SRCS)
	@echo -e $(TEST_SRCS_FILE)
	@echo -e $(TESTS)

help: 
	@echo "Usage : "
	@echo -e "\t - Default target : compile plugin into a shared lib libplugin.so."
	@echo -e "\t - debug : Compile the plugin with DEBUG flag which trigger .dot file generation. .dot files representes Control Flow Graph (CFG), can be converted to png with dot2png target."
	@echo -e "\t - test  : compile test source files with the plugin enabled to test it."
	@echo -e "\t - test-{fail,pass}-{1,2} : compile specific file to test the plugin."
	@echo -e "\t - dot2png : generate png file from .dot file."
	@echo -e "\t - clean : clean the repo."



debug: PLUGIN_FLAGS += -DDEBUG
debug: CFLAGS += -DDEBUG
debug: clean $(EXE)
	@echo "Build debug done"

# Generic rule to build objects from the src folder
$(BUILDDIR)/%.o: src/%.cpp
	$(CXX) $(PLUGIN_FLAGS) -c -o $@ $<

$(EXE): $(OBJS)
	$(CXX) -shared -o $@ $^

#############
## Tests ####
#############



# Build all tests
tests: $(TESTS) 

# Generic build rule for tests inside the tests/ directory
$(BUILDDIR)/test-%: tests/test-%.c $(EXE)
	$(CC) $< $(CFLAGS) -o $@ -fplugin=./$(EXE) -lmpi
	
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
	rm -rvf $(EXE) $(TESTS) src/*.o $(BUILDDIR)/* *.dot $(PNG_DIR)

