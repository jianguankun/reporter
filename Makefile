complier = g++
make = $(complier) -g -Wall

objs = 	analyse.o \
		sha1.o 

analyse : $(objs)
	$(make) -o bin/analyse $(objs)

analyse.o : analyse.cpp
	$(make) -c analyse.cpp -o analyse.o

sha1.o : sha1.cpp sha1.h
	$(make) -c sha1.cpp -o sha1.o

report : inforeport crashreport

inforeport : inforeport.cpp
	$(make) inforeport.cpp -o inforeport -lfcgi++ -lfcgi

crashreport : crashreport.cpp
	$(make) crashreport.cpp -o crashreport -lfcgi++ -lfcgi 

install : analyse
	test -d '/usr/local/report' || mkdir -p '/usr/local/report'
	test -d '/usr/local/report/sbin/' || mkdir -p '/usr/local/report/sbin/'
	cp bin/analyse /usr/local/report/sbin/analyse