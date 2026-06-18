EXE=libplugin.so

all: $(EXE)



CC ?= gcc
CXX ?= g++

MPICC=mpicc

PLUGIN_FLAGS=-I`$(CC) -print-file-name=plugin`/include -g -Wall -fno-rtti -fPIC 


CFLAGS=-g -O3

OBJS=plugin.o \
	 mpi_collectives.o \
	 graphviz.o



debug: PLUGIN_FLAGS += -DDEBUG
debug: CFLAGS += -DDEBUG
debug: clean libplugin.so
	@echo "Build debug done"




%.o: %.cpp %.h
	$(CXX) $(PLUGIN_FLAGS) -c -o $@ $<

libplugin.so: $(OBJS)
	$(CXX) -shared -o $@ $^

#############
## Tests ####
#############



TEST_SRCS := $(wildcard test*.c)
TESTS := $(TEST_SRCS:.c=)


.PHONY: clean cleanall 


# Build all tests
tests: $(TESTS) 

# Generic build rule: testN depends on testN.c
test%: test%.c libplugin.so 
	$(CC) $< $(CFLAGS) -o $@ -fplugin=./libplugin.so -lmpi
	


##########
## DOTS ##
##########

DOT_FILES := $(wildcard *.dot)
PNG_DIR := png
PNG_FILES := $(patsubst %.dot,$(PNG_DIR)/%.png,$(DOT_FILES))



dot2png:  $(PNG_FILES)

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
	rm -rf $(EXE)
	rm -rf $(TESTS)
	rm -rf *.o


cleanall: clean
	rm -rf *.dot $(PNG_FILES)
