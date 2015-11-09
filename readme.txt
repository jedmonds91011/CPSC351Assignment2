Compile sender with:
g++ sender.cpp -g -lpthread -o sender
Compile receiver with:
g++ recv.cpp -g -lpthread -o recv

We can also use the makefile that's been provided.

The '-g' flag enables inclusion of debugging information in the executable, 
for use with the 'gdb' debugger.

The shared memory segment, if orphaned, can be shown with 'ipcs -mp'
Our recv program prints the shared memory's shmid, so it can be found
on the list.

It can then be removed with 'ipcrm shm <id>'

The file .gdbinit contains watchpoints and stuff to automatically load into gdb.
To allow this, the global file ~/.gdbinit must have 'add-auto-load-safe-path <path to local .gdbinit>'
in it. This is a security measure to prevent unknown sourcecode from executing commands in gdb.
