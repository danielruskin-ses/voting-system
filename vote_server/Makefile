CXX = g++
CXX_FLAGS += -pthread -I../shared_cpp -I/usr/local/include -I/usr/include -std=c++14
LD_FLAGS += -L/usr/local/lib
PROTOC = protoc --plugin=protoc-gen-nanopb=../lib/nanopb/generator/protoc-gen-nanopb
PROTOS_PATH = ../protos
CPP_PATH = .

BIN = vote_server
BUILD_DIR = ./build
CPP = $(shell find $(CPP_PATH) -name "*.cpp")
PROTO_COMPILED_C = $(BUILD_DIR)/pb/pb_encode.c $(BUILD_DIR)/pb/pb_decode.c $(BUILD_DIR)/pb/pb_common.c
PROTOS = $(shell find $(PROTOS_PATH) -name "*.proto")

OBJ = $(CPP:%.cpp=$(BUILD_DIR)/vote_server/%.o)
OBJ += $(PROTO_COMPILED_C:%.c=%.o)
DEP = $(OBJ:%.o=%.d)
$(BIN) : $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN) : $(OBJ)
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $^ -o $@

-include $(DEP)

$(PROTO_COMPILED_C) : $(PROTOS)
	mkdir -p $(@D)
	$(PROTOC) --proto_path=$(PROTOS_PATH) --nanopb_out=$(@D) $^

$(BUILD_DIR)/pb/%.o : %.c
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@
        
$(BUILD_DIR)/vote_server/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -r $(BUILD_DIR)
