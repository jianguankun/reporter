# reporter

#### 介绍
日志收集服务


#### 安装方式一，编译安装

1.安装必要的工具

```
yum install -y gcc gcc-c++ gcc-g77 boost-devel bzip2
```

2.打开项目目录，编译并安装依赖库fast-gci

```
tar -xf fcgi-2.4.1-SNAP-0910052249.tar.gz
cd fcgi-2.4.1-SNAP-0910052249
./configure
make && make install
```

3.打开项目目录，编译并安装依赖库fastcgipp库

```
tar -xf fastcgi++-2.1.tar.bz2
cd fastcgi++-2.1
./configure
make && make install
touch /etc/ld.so.conf.d/fastcgipp.conf
echo /usr/local/lib/ > /etc/ld.so.conf.d/fastcgipp.conf
ldconfig
```

4.打开项目目录，编译并安装项目

```
make report
make install
```

至此已产生两个服务程序inforeport与crashreport，分别安装于/usr/local/report目录下。
安装程序产生/data/www/report目录，用于存放客户上报的日志数据。
inforeport监听端口6000
crashreport监听端口6001

5.安装nginx,已安装请忽略这步。

```
yum install -y pcre pcre-devel zlib zlib-devel libssl-dev
tar -xf nginx-1.3.8.tar.gz
cd nginx-1.3.8
./configure --user=www --group=www --prefix=/usr/local/nginx --with-http_stub_status_module --with-http_ssl_module --with-http_realip_module
make && make install

#设置环境变量
vim /root/.bash_profile
#加入PATH=$PATH:$HOME/bin:/usr/local/nginx/sbin/
source /root/.bash_profile
```

6.添加nginx配置，路由inforeport、crashreport请求，reload nginx生效

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
以使用端口8000为例，通过http://host:8000即可浏览/data/www/report目录，reload nginx生效

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
至此可以使用inforeport、crashreport请求提交日志。

9.启动cron，cron用于定期（每天、每半小时）产生crashreport的报告
```
cd /usr/local/report/sbin/
./cron -d
```

#### 安装方式二，使用docker镜像

1.安装docker，已安装请忽略这步。

```
curl -fsSL https://get.docker.com -o get-docker.sh
sh get-docker.sh
```

2.制作镜像

```
#注意最后参数“.”，表示当前有Dockerfile的目录
docker build -t reporter:v1 .
```

3.通过镜像启动容器

```
docker run -it -d \
   -p 80:80 -p 8000:8000 \
   -v /data/www/reporter-data:/data/www/report \
   -v /var/reporter/nginx-logs:/usr/local/nginx/logs \
   -v /var/reporter/analyse-logs:/var/reporter/logs \
   -e TZ=Asia/Shanghai \
   --name reporter \
   reporter:v1
```

#### 安装方式三，使用公共docker镜像

1.安装docker，已安装请忽略这步。

```
curl -fsSL https://get.docker.com -o get-docker.sh
sh get-docker.sh
```

2.获取镜像

```
docker pull jianguankun/reporter
```

3.通过镜像启动容器

```
docker run -it -d \
   -p 80:80 -p 8000:8000 \
   -v /data/www/reporter-data:/data/www/report \
   -v /var/reporter/nginx-logs:/usr/local/nginx/logs \
   -v /var/reporter/analyse-logs:/var/reporter/logs \
   -e TZ=Asia/Shanghai \
   --name reporter \
   jianguankun/reporter
```

#### 接口使用说明

1.inforeport接口，普通日志接口
- 接口地址http://host:port/cgi-bin/inforeport
- 请求方式HTTP post
- 请求体（x-www-form-urlencoded）

| KEY     | VALUE          | 说明            |
|---------|----------------|---------------|
| project | com.wsqpg.test | 项目名，建议写APP包名  |
| logfile | test.log       | 要追加日志的文件名     |
| line0   | 111111111      | 这次日志，第1行要写的内容 |
| line1   | 2222222222     | 这次日志，第2行要写的内容 |
| ...   | ...     | ... |
| line[n]   | nnnnnnnnnnn     | 这次日志，第n行要写的内容 |

- 返回结果
成功，返回“info record success!”
失败，返回原因，如“info record fail!missing project name.”

2.crashreport接口，APP错误/崩溃日志专用接口
- 接口地址http://host:port/cgi-bin/crashreport
- 请求方式HTTP post
- 请求体（x-www-form-urlencoded）

| KEY     | VALUE          | 说明            |
|---------|----------------|---------------|
| project | com.wsqpg.test | 项目名，建议写APP包名  |
| userid| 用户id       | APP当前的用户标识(可以不用)     |
| username| 用户名| APP当前的用户名(可以不用) |
| content| stack traceback:[C]: in function 'SetText'     | 错误内容 |

- 返回结果
成功，返回“bug record success!”
失败，返回原因，如“info record fail!mbug record fail!missing project name.”

#### 如何自定义周期性产生crashreport报告

本项目提供的sbin/cron程序，是规定每半小时产生报告，每天零时产生日报
如果希望灵活定义，可不使用cron程序，改用系统的计划工具，以Centos的crond为例
注意容器的生产环境下不建议使用系统的计划工具，容器已默认启动项目本身的sbin/cron程序

1.添加环境变量
```
vim ~/.bash_profile
把/usr/local/report/sbin添加环境变量PATH中去
source ~/.bash_profile
```
2.修改/etc/crontab文件，第2行PATH添加/usr/local/report/sbin，参考下面配置

```
SHELL=/bin/bash
PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/report/sbin
MAILTO=root
```

3.修改/etc/crontab文件尾总添加2个设置，保存文件

```
  */30  *  *  *  * root run-parts /usr/local/report/hourly > /usr/local/report/log
  59 23  *  *  * root run-parts /usr/local/report/daliy > /usr/local/report/log
```
意思是每小时执行一次/usr/local/report/hourly目录下的脚本，第天执行一次/usr/local/report/daliy目录下的脚本。

4.重新载入crontab配置生效，注意要先停止项目提供的sbin/cron程序。

```
service crond reload
```

