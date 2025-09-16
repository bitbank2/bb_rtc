CFLAGS= -D__LINUX__ -c -Wall -O2
LIBS = -lm -lbb_rtc -lpthread

all: getset_time

getset_time: main.o
	g++ main.o $(LIBS) -o getset_time 

main.o: main.cpp
	g++ $(CFLAGS) main.cpp

clean:
	rm *.o getset_time
