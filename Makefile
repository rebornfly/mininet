
ARRU = ar -ru

CXXFLAG = -Wall -O2 $(PGOPT) -D_REENTRANT 

SRCS = acceptor.cpp globals.cpp net_epoll.cpp net_conn.cpp net_handler.cpp tcpserver.cpp request_dispatch.cpp async_request_dispatch.cpp logger.cpp timer.cpp mainloop.cpp

OBJS = $(SRCS:.cpp=.o)

.SUFFIXES: .o .cpp
.cpp.o:
	$(CXX) $(CXXFLAG) ${INCLUDE} -std=c++11  -c -o $@ $<

all: libnet.a 

libnet.a: $(OBJS) 
	$(ARRU) ../lib/libtinynet.a  *.o

../../lib:
	mkdir ../../lib

clean:
	rm -f *.o
	rm -f ../lib/libtinynet.a 

install:


