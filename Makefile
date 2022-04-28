complier = g++
make = $(complier) -g -Wall

analyse_objs = 	analyse.o \
				sha1.o 

cron_objs = cron.o \
			logger.o \
			timer_manager.o 

cron_prepare : 
	pidof cron | xargs kill -9
	rm cron -f
	clear

cron : $(cron_objs) 
	$(make) -o bin/cron $(cron_objs) -l pthread

cron.o : cron.cpp
	$(make) -c cron.cpp -o cron.o

analyse : $(analyse_objs)
	$(make) -o bin/analyse $(analyse_objs)

analyse.o : analyse.cpp
	$(make) -c analyse.cpp -o analyse.o

sha1.o : sha1.cpp sha1.h
	$(make) -c sha1.cpp -o sha1.o

logger.o : logger.cpp logger.h
	$(make) -c logger.cpp -o logger.o

timer_manager.o : timer_manager.cpp timer_manager.h
	$(make) -c timer_manager.cpp -o timer_manager.o

report : inforeport crashreport

inforeport : inforeport.cpp
	$(make) inforeport.cpp -o bin/inforeport -lfcgi++ -lfcgi

crashreport : crashreport.cpp
	$(make) crashreport.cpp -o bin/crashreport -lfcgi++ -lfcgi 

install : analyse report cron
	test -d '/usr/local/report' || mkdir -p '/usr/local/report'
	test -d '/usr/local/report/sbin/' || mkdir -p '/usr/local/report/sbin/'
	cp bin/inforeport /usr/local/report/inforeport
	cp bin/crashreport /usr/local/report/crashreport
	cp bin/analyse /usr/local/report/sbin 
	cp bin/cron /usr/local/report/sbin 
	cp -r sh/daliy/ /usr/local/report/
	cp -r sh/hourly/ /usr/local/report/
	mkdir -p /data/www/report