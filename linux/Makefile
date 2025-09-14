CFLAGS= -D__LINUX__ -c -Wall -O2
LIBS = -lm -lpthread

all: getset_time

getset_time: main.o bb_rtc.o
	g++ main.o bb_rtc.o $(LIBS) -o getset_time 

main.o: main.cc
	g++ $(CFLAGS) main.cc

bb_rtc.o: ../src/bb_rtc.cpp
	g++ $(CFLAGS) ../src/bb_rtc.cpp

clean:
	rm *.o getset_time
