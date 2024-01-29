环境：ubuntu16

os初始化

安装jdk7


配置关闭编译验证
```
export DISABLE_HOTSPOT_OS_VERSION_CHECK=ok
```

编译缓存
```
sudo apt-get install ccache
```
一些依赖

```
sudo apt-get install libxext-dev libxrender-dev libxtst-dev libxt-dev
sudo apt-get install libcups2-dev
sudo apt-get install libfreetype6-dev
sudo apt-get install libasound2-dev
```

具体开始命令
1. 初始化
  ```shell
  bash configure --with-boot-jdk=$JAVA7_HOME --with-target-bits=64 --with-debug-level=slowdebug
  ```
2. 编译
   ```shell
   make all CONF=slowdebug
   ```
## ub18 安装前置
1. 安装gcc-5、g++-5 
安装后把/usr/bin/gcc、/usr/bin/g++ 软链接旧版
```
sudo apt-get install gcc-5 g++-5
sudo rm -rf /usr/bin/gcc
sudo rm -rf /usr/bin/g++
sudo ln -s /usr/bin/gcc-5 /usr/bin/gcc
sudo ln -s /usr/bin/g++-5 /usr/bin/g++
```

2. glibc降级成2.23
从网站https://ftp.gnu.org/gnu/glibc/下载
```
wget https://ftp.gnu.org/gnu/glibc/glibc-2.23.tar.gz
tar -zxvf glibc-2.23.tar.gz
```
编译前一些修改:
```
sudo vi misc/regexp.c
```

misc/regexp.c
```
char *loc1;
char *loc2;
修改成
#include <stdlib.h>	/* Get NULL.  */
/* Define the variables used for the interface.  Avoid .symver on common
   symbol, which just creates a new common symbol, not an alias.  */
char *loc1 = NULL;
char *loc2 = NULL;


char *locs;
修改成
char *locs = NULL;
```

编译
```
sudo mkdir etc
sudo cp /etc/ld.so.conf etc/ld.so.conf

mkdir build && cd build
../configure --prefix=/usr/local/glibc-2.23
make
sudo make install

sudo mv /usr/bin/ldd /usr/ldd-2.27
sudo ln -s /usr/local/glibc-2.23/bin/ldd /usr/bin/ldd-2.23
sudo ln -s /usr/bin/ldd-2.23 /usr/bin/ldd
sudo ln -s /usr/bin/ldd-2.27 /usr/bin/ldd
```

```
https://github.com/settings/keys

ssh-keygen -t ed25519 -C "your_email@example.com"
ssh-add ~/.ssh/id_ed25519

chmod 600 ~/.ssh/id_ed25519

 echo ~/.ssh/id_ed25519.pub
```
