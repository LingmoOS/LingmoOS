# 日历代码梳理笔记


## 日历的通用配置

在 service 端由 daccountmanagerservice 提供 dbus 接口，daccountmanagemodule 提供具体功能，
daccountmanagerdatabase 操作数据库。

dbusservice -> calendarDataManager -> dbmanager

现在要实现通用配置中的“每星期开始”和“时间格式”跟随控制中心，最初的想法是给通用配置添加新的值，比如-1来代表需要跟随控制中心，但这样可能会导致其他使用这两个配置项的应用得到意外的值，所以决定给通用配置新增两个配置项，用来标识“每星期开始”和“时间格式”是否跟随控制中心。