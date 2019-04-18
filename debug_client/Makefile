######## PARAMS

# lib paths
NANOPB_PATH=../lib/nanopb

# Source file paths (protos, CPP)
PROTOS_PATH = ../protos
CPP_PATH = ./src
SHARED_CPP_PATH = ../shared_cpp
SHARED_C_PATH = ../shared_c

# Output file paths (final bin, build files)
BIN = debug_client
BUILD_DIR = ./build
GEN_DIR = $(BUILD_DIR)/gen
SHARED_DIR = $(BUILD_DIR)/shared

# Compiler/linker flags
CXX = g++
CXX_FLAGS += -g -DPB_FIELD_16BIT -pthread -I/usr/local/include -I/usr/include -I$(GEN_DIR) -I$(SHARED_DIR) -I$(NANOPB_PATH) -std=c++14
LD_FLAGS += -L/usr/local/lib -lpqxx -lstdc++fs -lwolfssl -lgmp

######### DERIVED

# List of source/header files, proto files
CPP = $(shell find $(CPP_PATH) -name "*.cpp")
SHARED_CPP_SOURCE = $(shell find $(SHARED_CPP_PATH) -name "*.cpp")
SHARED_C_SOURCE = $(shell find $(SHARED_C_PATH) -name "*.c")
SHARED_CPP_HEADER = $(shell find $(SHARED_CPP_PATH) -name "*.h")
SHARED_C_HEADER = $(shell find $(SHARED_C_PATH) -name "*.h")
PROTOS_SOURCE = $(shell find $(PROTOS_PATH) -name "*.proto")

# List of shared/generated files: shared files (CPP, C, H), proto files (C, H), nanopb files
SHARED_CPP = $(subst $(SHARED_CPP_PATH)/,$(SHARED_DIR)/shared_cpp/,$(SHARED_CPP_SOURCE))
SHARED_C = $(subst $(SHARED_C_PATH)/,$(SHARED_DIR)/shared_c/,$(SHARED_C_SOURCE))
SHARED_H = $(subst $(SHARED_CPP_PATH)/,$(SHARED_DIR)/shared_cpp/,$(SHARED_CPP_HEADER)) $(subst $(SHARED_C_PATH)/,$(SHARED_DIR)/shared_c/,$(SHARED_C_HEADER))
PROTOS_SOURCE_MOD = $(subst $(PROTOS_PATH)/,$(GEN_DIR)/gen_c/pb/,$(PROTOS_SOURCE))
PROTOS_C = $(PROTOS_SOURCE_MOD:%.proto=%.pb.c)
PROTOS_H = $(PROTOS_SOURCE_MOD:%.proto=%.pb.h)
NANOPB_C = $(GEN_DIR)/gen_c/nanopb/pb_encode.c $(GEN_DIR)/gen_c/nanopb/pb_decode.c $(GEN_DIR)/gen_c/nanopb/pb_common.c
GEN_FILES = $(SHARED_CPP) $(SHARED_C) $(SHARED_H) $(PROTOS_C) $(PROTOS_H) $(NANOPB_C)

# List of object files that should be included in final binary, plus their dependencies
OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)
OBJ += $(SHARED_CPP:%.cpp=%.o)
OBJ += $(SHARED_C:%.c=%.o)
OBJ += $(PROTOS_C:%.pb.c=%.pb.o)
OBJ += $(NANOPB_C:%.c=%.o)
DEP = $(OBJ:%.o=%.d)

all: pre-build main-build

# Create all generated c/h files before starting compilation
pre-build: $(GEN_FILES)

main-build: $(BIN)

# Final output rule - compile the binary 
$(BIN) : $(BUILD_DIR)/$(BIN)
$(BUILD_DIR)/$(BIN) : $(OBJ)
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(LD_FLAGS)

-include $(DEP)

# Copy over shared files (take from shared dir, but into build/shared)
$(SHARED_DIR)/shared_cpp/%.cpp: $(SHARED_CPP_PATH)/%.cpp
	mkdir -p $(@D)
	cp $^ $@
$(SHARED_DIR)/shared_c/%.c: $(SHARED_C_PATH)/%.c
	mkdir -p $(@D)
	cp $^ $@
$(SHARED_DIR)/shared_cpp/%.h: $(SHARED_CPP_PATH)/%.h
	mkdir -p $(@D)
	cp $^ $@
$(SHARED_DIR)/shared_c/%.h: $(SHARED_C_PATH)/%.h
	mkdir -p $(@D)
	cp $^ $@

# Compile protobuf files (take from proto path, put into build/gen/pb)
$(GEN_DIR)/gen_c/pb/%.pb.c $(GEN_DIR)/gen_c/pb/%.pb.h: $(PROTOS_PATH)/%.proto $(PROTOS_PATH)/%.options
	mkdir -p $(@D)
	protoc --plugin=protoc-gen-nanopb=$(NANOPB_PATH)/generator/protoc-gen-nanopb --proto_path $(<D) --nanopb_out="-f$(subst .proto,.options, $<):$(@D)" $<

# Copy over nanopb files (take from nanopb, but into build/gen/nanopb)
$(GEN_DIR)/gen_c/nanopb/%.c: $(NANOPB_PATH)/%.c
	mkdir -p $(@D)
	cp $^ $@

# Compile gen files (C)
$(GEN_DIR)/gen_c/%.o : $(GEN_DIR)/gen_c/%.c
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

# Compile gen files (CPP)
$(GEN_DIR)/gen_cpp/%.o : $(GEN_DIR)/gen_cpp/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

# Compile shared files (CPP)
$(SHARED_DIR)/shared_cpp/%.o: $(SHARED_DIR)/shared_cpp/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

# Compile shared files (C)
$(SHARED_DIR)/shared_c/%.o: $(SHARED_DIR)/shared_c/%.c
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

# Compile all other files (CPP)
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@


.PHONY : clean
clean :
	rm -fr $(BUILD_DIR) protos_compiled pb nanopb
