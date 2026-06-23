EXE = libplugin.so

all: $(EXE)

CC ?= gcc
CXX ?= g++
MPICC = mpicc

# Added -Iinclude to resolve header files in the new directory
PLUGIN_FLAGS = -I`$(CC) -print-file-name=plugin`/include -Iinclude -g -Wall -fno-rtti -fPIC 
CFLAGS = -g -O3 -Iinclude

# Pointing to the new src/ directory
SRCS = src/plugin.cpp src/mpi_collectives.cpp src/graphviz.cpp
OBJS = $(SRCS:.cpp=.o)


help: 
	@echo "Default target : compile plugin into a shared lib libplugin.so."
	@echo "debug : add DEBUG flag to compilation which trigger .dot file generation. .dot files representes Control Flow Graph (CFG), can be converted to png with dot2png target."
	@echo "test  : compile test source files with the plugin enabled to test it."
	@echo "test-{fail,pass}-{1,2} : compile specific file to test the plugin."
	@echo "dot2png : generate png file from .dot file."
	@echo "clean : clean compiled source file."
	@echo "cleanall : clean target + png / dot files."



debug: PLUGIN_FLAGS += -DDEBUG
debug: CFLAGS += -DDEBUG
debug: clean $(EXE)
	@echo "Build debug done"

# Generic rule to build objects from the src folder
src/%.o: src/%.cpp
	$(CXX) $(PLUGIN_FLAGS) -c -o $@ $<

$(EXE): $(OBJS)
	$(CXX) -shared -o $@ $^

#############
## Tests ####
#############

TEST_SRCS := $(wildcard tests/test*.c)
TESTS := $(patsubst tests/%.c,%,$(TEST_SRCS))

.PHONY: clean cleanall tests dot2png

# Build all tests
tests: $(TESTS) 

# Generic build rule for tests inside the tests/ directory
test-%: tests/test-%.c $(EXE)
	$(CC) $< $(CFLAGS) -o $@ -fplugin=./$(EXE) -lmpi
	
##########
## DOTS ##
##########

DOT_FILES := $(wildcard *.dot)
PNG_DIR := png
PNG_FILES := $(patsubst %.dot,$(PNG_DIR)/%.png,$(DOT_FILES))

dot2png: $(PNG_FILES)

# Ensure png/ directory exists
$(PNG_DIR):
	mkdir -p $(PNG_DIR)

# Rule to generate png/filename.png from filename.dot
$(PNG_DIR)/%.png: %.dot | $(PNG_DIR)
	dot -T png $< -o $@

###########
## CLEAN ##
###########

clean:
	rm -f $(EXE) $(TESTS) src/*.o

cleanall: clean
	rm -rf *.dot $(PNG_DIR)
