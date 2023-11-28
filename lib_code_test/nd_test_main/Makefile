OBJS=main.o
TARGET=test

$(TARGET):$(OBJS)
	g++ main.o -o test -L./ -lods -lfo -lnd

main.o:main.cpp
	g++ -c main.cpp 

clean:
	rm -rf $(OBJS) $(TARGET)