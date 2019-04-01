CC:=c++
CPPFLAGS:=-std=c++14

SRC_DIR:=./src
TEST_DIR:=./test
BUILD_DIR:=./bin
SRCS:=$(shell find $(SRC_DIR) -name *.cpp)
TEST_SRCS:=$(shell find $(TEST_DIR) -name *.cpp)
OBJS:=$(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TEST_OBJS:=$(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/test/%.o)

.PHONY: clean

all: CPPFLAGS+=-O2
all: main

debug: CPPFLAGS+=-g -Wall -DDEBUG
debug: main

test: test_runner

main: $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^
	
test_runner: $(TEST_OBJS) $(filter-out %main.o, $(OBJS))
	$(CC) $(CPPFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -o $@ -c $<
	
$(BUILD_DIR)/test/%.o: $(TEST_DIR)/%.cpp
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -o $@ -c $<

clean:
	rm main
	rm -r $(BUILD_DIR)/*
