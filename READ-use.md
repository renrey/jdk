环境：ubuntu16

os初始化

安装jdk7
配置关闭编译验证
```
export DISABLE_HOTSPOT_OS_VERSION_CHECK=ok
```

命令
1. 初始化
  ```shell
  bash configure --with-boot-jdk=$JAVA7_HOME --target-with-bits=64 --with-debug-level=slowdebug
  ```
2. 编译
   ```shell
   make all CONF=slowdebug
   ```
