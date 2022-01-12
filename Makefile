CC = g++
TESTLIBS = -lgtest  -lgtest_main -lpthread
SRC_RECEIVER = src/receiver/receiver.cpp src/protocol.cpp
SRC_SENDER = src/sender/sender.cpp src/protocol.cpp
TEST_DIR = test/test_protocol.cpp src/protocol.cpp

CCFLAGS = -std=c++11

PROGRAM_RECEIVER = receiver
PROGRAM_SENDER = sender
PROGRAM_TEST = ptest

%.o : %.cc
	$(CC) $(CCFLAGS) -c $< -o $@

$(PROGRAM_TEST): $(TEST_DIR)
	$(CC) $(CCFLAGS) -lz -o $@ $(TEST_DIR) $(TESTLIBS)

$(PROGRAM_RECEIVER): $(SRC_RECEIVER)
	$(CC) $(CCFLAGS) -lz -o $@ $(SRC_RECEIVER) $(TESTLIBS)

$(PROGRAM_SENDER): $(SRC_SENDER)
	$(CC) $(CCFLAGS) -lz -o $@ $(SRC_SENDER) $(TESTLIBS)

.PHONY: all
all: $(PROGRAM_RECEIVER) $(PROGRAM_SENDER) $(PROGRAM_TEST)

.PHONY : clean
clean :
	rm -f *.o *~ *.d \
	src/*.o \
	src/receiver/*.o \
	src/sender/*.o \
	test/test_protocol.o \
	receiver sender ptest \
