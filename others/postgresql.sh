
#!/bin/bash

#创建用户和组
groupadd postgres
useradd -g postgres postgres

#进入安装目录
cd /usr/local/

#删除原有安装
rm -rf postgresql

#解压
tar zxvf postgresql-9.3.2.tar.gz
cd postgresql-9.3.2

#配置
./configure --prefix=/usr/local/postgresql --without-readline

#编译安装
make && make install

#安装contrib目录下的一些工具，是第三方组织的一些工具代码，建议安装
cd /usr/local/postgresql-9.3.2/contrib
make && make install

#添加环境变量，并使之生效（初级用户请自行学习设置）
export PATH=/usr/local/postgresql/bin:$PATH

#建postgres数据库的目录
mkdir -p /var/postgresql/data
chown postgres:postgres /var/postgresql/*
chown postgres:postgres /usr/local/postgresql/*
chmod -R 775 /var/postgresql/*

#切换用户
su postgres

#初始化数据库
initdb -D /var/postgresql/data

#启动服务
pg_ctl -D /var/postgresql/data -l /var/postgresql/logfile start

#创建测试数据库
#create database test;
createdb test;

#切换到david 数据库
\c test

#创建测试表
create table test (id integer, name text);

#插入测试数据
insert into test values (1,'david');

#选择数据
select * from test ;

#修改linux 系统用户postgres 的密码
passwd postgres
#postgres

#修改PostgresSQL 数据库配置实现远程访问

#修改postgresql.conf 文件
vi /var/postgresql/data/postgresql.conf

#--------------------允许远程连接---------------------------
#修改客户端认证配置文件pg_hba.conf，将需要远程访问数据库的IP地址或地址段加入该文件
vi /var/postgresql/data/pg_hba.conf

#在文件的最下方加上下面的这句话
host    all         all         0.0.0.0/0             trust

#设置监听整个网络，查找“listen_addresses ”字符串，
vi /var/postgresql/data/postgresql.conf

#修改为如下：
listen_addresses = '*' 

#重启服务
pg_ctl -D /var/postgresql/data -l /var/postgresql/logfile restart

#停止服务
pg_ctl -D /var/postgresql/data -l /var/postgresql/logfile stop

#端口是否启用
netstat -anp | grep 5432
