CC=gcc
LIBSOCKET=-lnsl
CCFLAGS=-Wall -g
SRV=server
CLT=client
SRVT=server-tools.c
CLTT=client-tools.c

all: $(SRV) $(CLT)

$(SRV):$(SRV).c
	$(CC) -g -o $(SRV) $(LIBSOCKET) $(SRVT) $(SRV).c

$(CLT):	$(CLT).c
	$(CC) -g -o $(CLT) $(LIBSOCKET) $(CLTT) $(CLT).c

clean:
	rm -f *.o *~
	rm -f $(SRV) $(CLT)


