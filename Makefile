all:	sender recv

debug:  send_debug recv_debug

sender:	sender.o
	g++ -g sender.o -o sender

recv:	recv.o
	g++ -g recv.o -o recv

sender.o:	 sender.cpp
	g++ -g -c sender.cpp

recv.o:	recv.cpp
	g++ -g -c recv.cpp

send_debug:	sender.o
	g++ -g sender.o -o sender_debug

recv_debug:	recv.o
	g++ -g recv.o -o recv_debug

sender_debug.o:	 sender.cpp
	g++ -g -Wall -c sender.cpp

recv_debug.o:	recv.cpp
	g++ -g -Wall -c recv.cpp 


clean:
	rm -rf *.o sender recv recv_debug sender_debug
