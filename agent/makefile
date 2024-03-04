SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) )

TARGET=agent
LIBS=-loldb
INCLUDE_PATH=/usr/include/orion_link

$(TARGET):$(OBJS)
	g++ $+ -o $@ -I $(INCLUDE_PATH) $(LIBS)
	./$@ 127.0.0.1

clean:
	rm -rf *.o $(TARGET)