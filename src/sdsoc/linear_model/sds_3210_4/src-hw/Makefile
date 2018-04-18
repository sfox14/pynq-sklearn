CC = arm-linux-gnueabihf-gcc
CPP = arm-linux-gnueabihf-g++

OBJSH = cf_stub.o linear.o portinfo.o
LIBS = -l pthread /usr/lib/libsds_lib.so
INC = -I include

all:
	$(CC) -fPIC $(INC) -c -g3 -gstabs cf_stub.c
	$(CC) -fPIC $(INC) -c -g3 -gstabs linear.cpp
	$(CC) -fPIC $(INC) -c -g3 -gstabs portinfo.c
	$(CPP) -g3 -gstabs -shared -fPIC -rdynamic $(OBJSH) -Wl,--start-group $(LIBS) -Wl,--end-group -o liblreg32104.so
	rm *.o

test:	
	$(CC) -fPIC $(INC) -c -g3 -gstabs cf_stub.c
	$(CC) -fPIC $(INC) -c -g3 -gstabs linear.cpp
	$(CC) -fPIC $(INC) -c -g3 -gstabs portinfo.c	
	$(CPP) -fPIC $(INC) linear.o portinfo.o cf_stub.o $(LIBS) -o test main.cpp
	rm *.o
	
clean:
	rm -f *.so *.o test *.dat
