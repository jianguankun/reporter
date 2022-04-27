FROM centos:7 as v1

COPY *.h *.cpp Makefile nginx.conf /reporter/
COPY bin /reporter/bin
COPY sh /reporter/sh

ADD fastcgi++-2.1.tar.bz2 /reporter/
ADD fcgi-2.4.1-SNAP-0910052249.tar.gz /reporter/
ADD nginx-1.3.8.tar.gz /reporter/

RUN yum install -y gcc gcc-c++ gcc-g77 boost-devel bzip2 make \
    && yum install -y pcre pcre-devel zlib zlib-devel libssl-dev \
    #step2
    && cd /reporter/fcgi-2.4.1-SNAP-0910052249 \
    && ./configure && make && make install \
    && cd /reporter/fastcgi++-2.1 \
    && ./configure && make && make install \
    && touch /etc/ld.so.conf.d/fastcgipp.conf \
    && echo /usr/local/lib/ > /etc/ld.so.conf.d/fastcgipp.conf \
    && ldconfig \
    #step3
    && cd /reporter/nginx-1.3.8/ \
    && groupadd www && useradd -g www www -s /bin/false \
    && ./configure --user=www --group=www \
        --prefix=/usr/local/nginx --with-http_stub_status_module --with-http_realip_module \
    && make && make install \
    #step4
    && cd /reporter/ \
    && make report && make install \
    && \cp -f nginx.conf /usr/local/nginx/conf/ \
    && cp /reporter/sh/start.sh / \
    && chmod +x /start.sh \
    #step5
    && yum remove -y gcc gcc-c++ gcc-g77 boost-devel bzip2 make \
    && yum remove -y pcre-devel zlib-devel libssl-dev \
    && yum clean all \
    && rm -fr /reporter/

VOLUME ["/data/www/report/","/usr/local/nginx/logs/"]

EXPOSE 80 8080

ENTRYPOINT ["./start.sh"]