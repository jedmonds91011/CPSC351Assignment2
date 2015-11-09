Compile sender with:
g++ sender.cpp -g -lpthread -o sender
Compile receiver with:
g++ recv.cpp -g -lpthread -o recv

We can also use the makefile that's been provided.

The '-g' flag enables inclusion of debugging information in the executable, 
for use with the 'gdb' debugger.