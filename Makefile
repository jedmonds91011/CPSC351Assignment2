all:	sender recv

debug:  send_debug recv_debug

sender:	sender.o
	g++ sender.o -o sender

recv:	recv.o
	g++ recv.o -o recv

sender.o:	 sender.cpp
	g++ -c sender.cpp

recv.o:	recv.cpp
	g++ -c recv.cpp

send_debug:	sender.o
	g++ sender.o -o sender_debug

recv_debug:	recv.o
	g++ recv.o -o recv_debug

sender_debug.o:	 sender.cpp
	g++ -Wall -g -c sender.cpp

recv_debug.o:	recv.cpp
	g++ -Wall -g -c recv.cpp 


clean:
	rm -rf *.o sender recv recv_debug sender_debug
