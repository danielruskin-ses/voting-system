CXX = g++
CXX_FLAGS += -pthread -I../shared_cpp -I/usr/local/include -I/usr/include -std=c++14
LD_FLAGS += -L/usr/local/lib

BIN = vote_server
BUILD_DIR = ./build
CPP = main.cpp logger/Logger.cpp server/Server.cpp server/Connection.cpp

OBJ = $(CPP:%.cpp=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)
$(BIN) : $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(BIN) : $(OBJ)
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) $^ -o $@

-include $(DEP)

$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(@D)
	$(CXX) $(CXX_FLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)
