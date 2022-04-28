#! /bin/sh

if [ ! -n "$1" ]; then
    echo error:no images specified.
    exit 1
fi

docker run -it -d \
   -p 80:80 -p 8000:8000 \
   -v /data/www/reporter-data:/data/www/report \
   -v /var/reporter/nginx-logs:/usr/local/nginx/logs \
   -v /var/reporter/cron-logs:/var/reporter/logs \
   -e TZ=Asia/Shanghai \
   --name reporter \
   $1