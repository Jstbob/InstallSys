# sys驱动命令行加载工具

## 基本指令

以管理者打开cmd，运行以下指令；

![image-20221110133435888](https://gitee.com/ningbocai/pictures/raw/master/image-20221110/133438-43.png)

![image-20221110134447487](https://gitee.com/ningbocai/pictures/raw/master/image-20221110/134450-53.png)

![](https://gitee.com/ningbocai/pictures/raw/master/image-20221110/134348-ed.png)

```c++
安装sys驱动:		InstallSys.exe -i [syspath/service]\n");
卸载sys驱动:		InstallSys.exe -u [syspath/service]\n");
查询非系统驱动：	  InstallSys.exe -q\n");
查询所有模块和公司:   InstallSys.exe -c\n");
```

## 非管理者运行，会提示句柄无效：

![image-20221110133557392](https://gitee.com/ningbocai/pictures/raw/master/image-20221110/134019-06.png)