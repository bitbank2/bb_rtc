CFLAGS=-c -Wall -O2 -D__LINUX__ -I../src
LIBS = -lm -lpthread

all: libbb_rtc.a

libbb_rtc.a: bb_rtc.o
	ar -rc libbb_rtc.a bb_rtc.o ;\
	sudo cp libbb_rtc.a /usr/local/lib ;\
	sudo cp ../src/bb_rtc.h /usr/local/include

bb_rtc.o: ../src/bb_rtc.cpp ../src/bb_rtc.h
	$(CXX) $(CFLAGS) ../src/bb_rtc.cpp

clean:
	rm *.o libbb_rtc.a
