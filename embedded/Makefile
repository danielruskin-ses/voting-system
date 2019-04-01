CC:=c++
CPPFLAGS:=-std=c++14

SRC_DIR:=./src
TEST_DIR:=./test
BUILD_DIR:=./build
SRCS:=$(shell find $(SRC_DIR) -name *.cpp)
TEST_SRCS:=$(shell find $(TEST_DIR) -name *.cpp)
OBJS:=$(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS:=$(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/test/%.o)

MAIN:=$(BUILD_DIR)/main
TEST_RUNNER:=$(BUILD_DIR)/test_runner

.PHONY: clean

all: CPPFLAGS+=-O2
all: $(MAIN)

debug: CPPFLAGS+=-g -Wall -DDEBUG
debug: $(MAIN)

test: $(TEST_RUNNER)

$(MAIN): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^
	
$(TEST_RUNNER): $(TEST_OBJS) $(filter-out %main.o, $(OBJS))
	$(CC) $(CPPFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -o $@ -c $<
	
$(BUILD_DIR)/test/%.o: $(TEST_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -o $@ -c $<

clean:
	rm -r $(BUILD_DIR)/*
