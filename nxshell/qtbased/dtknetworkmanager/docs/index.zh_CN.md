# dtknetworkmanager

## 项目介绍

dtknetworkmanager是对于deepin/UOS系统上的org.freedesktop.NetworkManager和com.deepin.daemon.Network的DBus接口的封装，同时也提供了对于NetworkManager的配置文件的封装，方便应用开发者轻松且快速的调用接口进行开发。

## 项目结构

对外暴露的类均在include文件夹下，其中include/settings下代表对于配置文件的封装。

## 如何使用项目

如果要使用此项目，可以阅读相关文档再参照examples文件夹下的例子。

## 注意事项

settings下提供的配置文件类中的每个属性的具体作用与含义请参考[这个文档](https://developer-old.gnome.org/NetworkManager/stable/ref-settings.html), 本文档只会对其中的一些帮助方法进行说明。
