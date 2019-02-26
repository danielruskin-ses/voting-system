# Nanopb path
NANOPB_PATH=../lib/nanopb

# Commands/flags
CXX = g++
CXX_FLAGS += -pthread -I../shared_cpp -I/usr/local/include -I/usr/include -I$(NANOPB_PATH) -std=c++14
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
OBJ += $(BUILD_DIR)/nanopb/pb_encode.o
OBJ += $(BUILD_DIR)/nanopb/pb_decode.o 
OBJ += $(BUILD_DIR)/nanopb/pb_common.o
DEP = $(OBJ:%.o=%.d)

# Final output rule - compile the binary 
$(BIN) : $(BUILD_DIR)/$(BIN)
$(BUILD_DIR)/$(BIN) : $(OBJ)
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $^ -o $@

-include $(DEP)

# Compile protobuf files
pb/%.pb.c: $(PROTOS_PATH)/%.proto
	mkdir -p $(@D)
	protoc --plugin=protoc-gen-nanopb=$(NANOPB_PATH)/generator/protoc-gen-nanopb --proto_path $(^D) --nanopb_out=$(@D) $^

# Copy over nanopb files
nanopb/%.c: $(NANOPB_PATH)/%.c
	mkdir -p $(@D)
	cp $^ $@

# Compile C files
$(BUILD_DIR)/%.o : %.c
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -fr $(BUILD_DIR) protos_compiled pb nanopb
