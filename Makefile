CC = gcc
CFLAGS = -Wall -pedantic
MAIN = httpd
OBJS = main.o checked.o getPort.o handle_request.o limit_fork.o readline.o signal_handler.o simple_net.o getPermBits.o dynamicStrCat.o sendUtil.o
HEADER =  checked.h getPort.h handle_request.h limit_fork.h readline.h signal_handler.h simple_net.h getPermBits.h dynamicStrCat.h sendUtil.h
all : $(MAIN)

$(MAIN) : $(OBJS) $(HEADER)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

main.o : simple_net.c getPort.c checked.c handle_request.c signal_handler.c limit_fork.c
	$(CC) $(CFLAGS) -c main.c simple_net.c getPort.c checked.c handle_request.c signal_handler.c limit_fork.c
simple_net.o : simple_net.c simple_net.h
	$(CC) $(CFLAGS) -c simple_net.c
getPort.o : getPort.c getPort.h
	$(CC) $(CFLAGS) -c getPort.c
checked.o : checked.c checked.h
	$(CC) $(CFLAGS) -c checked.c
handle_request.o : handle_request.c handle_request.h readline.c readline.h getPermBits.h getPermBits.c sendUtil.c sendUtil.h
	$(CC) $(CFLAGS) -c handle_request.c readline.c dynamicStrCat.c sendUtil.c getPermBits.c
signal_handler.o : signal_handler.c signal_handler.h
	$(CC) $(CFLAGS) -c signal_handler.c
limit_fork.o : limit_fork.c limit_fork.h
	$(CC) $(CFLAGS) -c limit_fork.c
readline.o : readline.c readline.h
	$(CC) $(CFLAGS) -c readline.c
getPermBits.o : getPermBits.c getPermBits.h
	$(CC) $(CFLAGS) -c getPermBits.c
dynamicStrCat.o : dynamicStrCat.c dynamicStrCat.h
	$(CC) $(CFLAGS) -c dynamicStrCat.c
sendUtil.o : sendUtil.c sendUtil.h
	$(CC) $(CFLAGS) -c sendUtil.c
clean: 
	rm *.o
