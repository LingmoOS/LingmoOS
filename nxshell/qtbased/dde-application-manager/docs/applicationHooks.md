# Application Hooks

本文档描述了 dde-application-manager 的hook机制。

## 功能描述

hook 允许系统组件在应用启动前对应用的运行时环境做出配置(如cgroups)。

## 配置文件

hook 的配置文件需要放在'/usr/share/deepin/dde-application-manager/hooks.d/'下，文件名必须符合以下规范:
    - 以数字开头，作为hook的顺序标识。
    - 以`-`分割顺序和hook名。
    - 文件格式需要是`json`，文件扩展名同样以`json`结尾。

例如: `1-proxy.json`就是一个符合要求的hook配置文件。

### 文件格式

文件中需要写明hook二进制的绝对位置和所需要的参数，例如：

```json
{
    "Exec": "/usr/bin/proxy",
    "Args": ["--protocol=https","--port=12345"]
}
```

需要注意的是，配置文件的键是大小写敏感的。
