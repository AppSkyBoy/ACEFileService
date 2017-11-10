acefileserver:main.o LocalFile.o SOCK_Server.o EventHandler.o
	gcc -o acefileserver main.o LocalFile.o SOCK_Server.o EventHandler.o -lACE

main.o: main.cpp SOCK_Server.h
	gcc -c main.cpp

LocalFile.o:LocalFile.cpp LocalFile.h
	gcc -c LocalFile.cpp

SOCK_Server.o:SOCK_Server.cpp SOCK_Server.h
	gcc -c SOCK_Server.cpp

EventHandler.o:EventHandler.cpp EventHandler.h
	gcc -c EventHandler.cpp

clean :
	-rm -rf *.o acefileserver

