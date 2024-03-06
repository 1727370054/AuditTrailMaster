SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) )

TARGET=client
LIBS=-loldb
INCLUDE_PATH=/usr/include/orion_link

$(TARGET):$(OBJS)
	g++ $+ -o $@ -I $(INCLUDE_PATH) $(LIBS)
	./$@
clean:
	rm -rf *.o $(TARGET)