OBJS=main.o
TARGET=test

$(TARGET):$(OBJS)
	g++ main.o -o test -L./ -lfo

main.o:main.cpp
	g++ -c main.cpp 

clean:
	rm -rf $(OBJS) $(TARGET)