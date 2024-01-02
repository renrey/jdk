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
