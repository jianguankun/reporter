#! /bin/sh

cd /usr/local/report
./crashreport -d
./inforeport -d
cd /usr/local/nginx/sbin
./nginx -g "daemon off;"