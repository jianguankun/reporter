#! /bin/sh

export PATH=$PATH:/usr/local/report/
export PATH=$PATH:/usr/local/report/sbin/
export PATH=$PATH:/usr/local/nginx/sbin/
crashreport -d
inforeport -d
cron -d
nginx -g "daemon off;"