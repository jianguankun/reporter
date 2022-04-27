#! /bin/sh

docker run -it -d \
   -p 80:80 -p 8000:8000 \
   -v /data/www/report:/data/www/report \
   -v /data/report-logs:/usr/local/nginx/logs \
   reporter:v1

