CFLAGS= -D__LINUX__ -c -Wall -O2
LIBS = -lm -lbb_rtc -lpthread

all: set_alarm

set_alarm: main.o
	g++ main.o $(LIBS) -o set_alarm 

main.o: main.cpp
	g++ $(CFLAGS) main.cpp

clean:
	rm *.o set_alarm
