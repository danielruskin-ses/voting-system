######## PARAMS

# Compiler/linker flags
CXX = g++
CXX_FLAGS += -pthread -I../shared_cpp -I/usr/local/include -I/usr/include -I/usr/include/postgresql -std=c++14
LD_FLAGS += -L/usr/local/lib -lsqlpp11-connector-postgresql -lpq

# Source file paths (protos, CPP)
CPP_PATH = .
SHARED_CPP_PATH = ../shared_cpp

# Output file paths (final bin, build files)
BIN = database_migrator 
BUILD_DIR = ./build

######### DERIVED
# Place to put generated, shared files
GEN_DIR = $(BUILD_DIR)/gen
SHARED_DIR = $(BUILD_DIR)/shared

# List of CPP files, proto files
CPP = $(shell find $(CPP_PATH) -name "*.cpp")
SHARED_CPP_SOURCE = $(shell find $(SHARED_CPP_PATH) -name "*.cpp")

SHARED_CPP = $(subst $(SHARED_CPP_PATH)/,$(SHARED_DIR)/,$(SHARED_CPP_SOURCE))

# List of object files that should be included in final binary, plus their dependencies
OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)
OBJ += $(SHARED_CPP:%.cpp=%.o)
DEP = $(OBJ:%.o=%.d)

all: pre-build main-build

pre-build:

main-build: $(BIN)

# Final output rule - compile the binary 
$(BIN) : $(BUILD_DIR)/$(BIN)
$(BUILD_DIR)/$(BIN) : $(OBJ)
	$(CXX) $(CXX_FLAGS) $^ -o $@ $(LD_FLAGS)

-include $(DEP)

# Compile shared files (take from shared path, put into build/shared)
$(SHARED_DIR)/%.o: $(SHARED_CPP_PATH)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

# Compile all other files (CPP)
$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@


.PHONY : clean
clean :
	rm -fr $(BUILD_DIR) 
