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
	$(make) inforeport.cpp -o bin/inforeport -lfcgi++ -lfcgi

crashreport : crashreport.cpp
	$(make) crashreport.cpp -o bin/crashreport -lfcgi++ -lfcgi 

install : analyse report
	test -d '/usr/local/report' || mkdir -p '/usr/local/report'
	test -d '/usr/local/report/sbin/' || mkdir -p '/usr/local/report/sbin/'
	cp bin/inforeport /usr/local/report/inforeport
	cp bin/crashreport /usr/local/report/crashreport
	cp bin/analyse /usr/local/report/sbin 
	cp -r sh/daliy/ /usr/local/report/
	cp -r sh/hourly/ /usr/local/report/
	mkdir -p /data/www/report
