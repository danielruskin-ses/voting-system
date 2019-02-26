# Replace this with path to nanopb 
-include ../lib/nanopb/extra/nanopb.mk

# Commands/flags
CXX = g++
CXX_FLAGS += -pthread -I../shared_cpp -I/usr/local/include -I/usr/include -I$(NANOPB_DIR) -std=c++14
LD_FLAGS += -L/usr/local/lib

# Source file (protobuf messages, CPP files, nanopb files)
PROTOS_PATH = ../protos
CPP_PATH = .

# Output file paths
BIN = vote_server
BUILD_DIR = ./build

# Derived source files (CPP, protobuf messages, protobuf messages compiled to C)
CPP = $(shell find $(CPP_PATH) -name "*.cpp")
PROTOS_SOURCE = $(shell find $(PROTOS_PATH) -name "*.proto")
PROTOS_SOURCE_MOD = $(subst $(PROTOS_PATH)/,pb/,$(PROTOS_SOURCE))
PROTOS_CPP = $(PROTOS_SOURCE_MOD:%.proto=%.pb.cpp)
PROTOS_H = $(PROTOS_CPP:%.pb.cpp=%.pb.h)

# Derived compiled source files and dep listings
OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)
OBJ += $(PROTOS_CPP:%.pb.cpp=$(BUILD_DIR)/%.pb.o)
DEP = $(OBJ:%.o=%.d)

# Final output rule - compile the binary 
$(BIN) : $(BUILD_DIR)/$(BIN)
$(BUILD_DIR)/$(BIN) : $(OBJ) $(NANOPB_CORE)
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(NANOPB_CORE)

-include $(DEP)

# Compile C files
$(BUILD_DIR)/%.o : %.c
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -r $(BUILD_DIR) protos_compiled
