TARGET = boots
OBJS = boots.o bootsector.o read.o show.o \
	mbr.o pbrfat.o
CXXFLAGS += -std=c++11
LDFLAGS += -lboost_program_options

.PHONY: all
all:
	make $(TARGET)

$(TARGET): $(OBJS) Makefile
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm *.o
