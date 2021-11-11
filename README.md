# reporter

#### 介绍
日志收集服务


#### 安装教程

1.  安装必要的工具
```
yum install -y gcc gcc-c++ gcc-g77 boost-devel bzip2
```
2. 打开项目目录，编译fast-gci库
```
tar -xf fcgi-2.4.1-SNAP-0910052249.tar.gz
cd fcgi-2.4.1-SNAP-0910052249
./configure
make && make install
```
2.  打开项目目录，编译fastcgipp库
```
tar -xf fastcgi++-2.1.tar.bz2
cd fastcgi++-2.1.tar.bz2
./configure
make && make install
touch /etc/ld.so.conf.d/fastcgipp.conf
echo /usr/local/lib/ > /etc/ld.so.conf.d/fastcgipp.conf
ldconfig
```

3.  xxxx

#### 使用说明

1.  xxxx
2.  xxxx
3.  xxxx

