# reporter

#### 介绍
日志收集服务


#### 安装步骤

1.  安装必要的工具

```
yum install -y gcc gcc-c++ gcc-g77 boost-devel bzip2
```

2. 打开项目目录，编译并安装依赖库fast-gci

```
tar -xf fcgi-2.4.1-SNAP-0910052249.tar.gz
cd fcgi-2.4.1-SNAP-0910052249
./configure
make && make install
```

3.  打开项目目录，编译并安装依赖库fastcgipp库

```
tar -xf fastcgi++-2.1.tar.bz2
cd fastcgi++-2.1.tar.bz2
./configure
make && make install
touch /etc/ld.so.conf.d/fastcgipp.conf
echo /usr/local/lib/ > /etc/ld.so.conf.d/fastcgipp.conf
ldconfig
```

4.  打开项目目录，编译并安装项目

```
make report
make install
```

至此已产生两个服务程序inforeport与crashreport，分别安装于/usr/local/report目录下。
安装程序产生/data/www/report目录，用于存放客户上报的日志数据。
inforeport监听端口6000
crashreport监听端口6001

5.  安装nginx,已安装请忽略这步。

6.  添加nginx配置，路由inforeport、crashreport请求，reload nginx生效

```
location = /cgi-bin/crashreport {
  add_header 'Access-Control-Allow-Origin' '*';
  fastcgi_pass 127.0.0.1:6000;
  fastcgi_param SCRIPT_FILENAME fcgi$fastcgi_script_name;
  include fastcgi_params;
}

location = /cgi-bin/inforeport {
  add_header 'Access-Control-Allow-Origin' '*';
  fastcgi_pass 127.0.0.1:6001;
  fastcgi_param SCRIPT_FILENAME fcgi$fastcgi_script_name;
  include fastcgi_params;
}
```

7. 添加nginx配置，可浏览/data/www/report目录，以便查看日志，注意需要新增一个server。
以使用端口8000为例，通过http://xxx:8000即可浏览/data/www/report目录，reload nginx生效

```
server {

  charset utf-8;
  listen 8000;
  server_name localhost1;
  autoindex on; #开启目录浏览功能；
  autoindex_exact_size on; #关闭详细文件大小统计，让文件大小显示MB，GB单位，默认为b；
  autoindex_localtime on; #开启以服务器本地时区显示文件修改日期！
  #access_log  logs/host0.access.log  main;

  location / {
    default_type text/plain;
    root /data/www/report;
  }
}
```

8.启动inforeport、crashreport

```
cd /usr/local/report
./inforeport -d
./crashreport -d
```



#### 使用说明

1.  xxxx
2.  xxxx
3.  xxxx

