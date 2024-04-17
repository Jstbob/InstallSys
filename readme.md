# sys驱动命令行加载工具

## 说明

该工具可以直接用来加载和卸载以 “.sys” 后缀的驱动程序，方便驱动程序的开发和调试，“.sys” 文件需要测试签名，或者操作系统处于测试模式。

## 基本指令

需要以**管理员权限**打开命令行窗口，运行以下指令：
```cmd
安装sys驱动:    InstallSys.exe -i [syspath/service_name]"
卸载sys驱动:    InstallSys.exe -u [syspath/service_name]"
```

## 错误提示

非管理者运行，会提示句柄无效

![image-20221110133557392](https://gitee.com/ningbocai/pictures/raw/master/image-20221110/134019-06.png)

## 实例

![image-20221110134447487](https://gitee.com/ningbocai/pictures/raw/master/image-20221110/134450-53.png)

## 执行文件

编译的可执行文件位于 "x64/" 目录下。