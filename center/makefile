SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) )

TARGET=center
LIBS=-loldb
INCLUDE_PATH=/usr/include/orion_link

$(TARGET):$(OBJS)
	g++ $+ -o $@ -I $(INCLUDE_PATH) $(LIBS)
	./$@ install 127.0.0.1
	./$@ add 127.0.0.1 dev1
	./$@ add 192.168.0.1 dev2

clean:
	rm -rf *.o $(TARGET)