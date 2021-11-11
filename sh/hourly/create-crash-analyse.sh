#!/bin/sh

if [ ! -n "$1" ]; then
    currdate=$(date +%m-%d)
else
    currdate=$1
fi
rootpath=/data/www/report/crash/$currdate

if [ ! -d "$rootpath" ]; then
    echo path ${rootpath} not exist!
    exit 0
fi

for file in ${rootpath}/*
do
    if [ -d $file ]; then
	analyse ${file}
    fi
done
