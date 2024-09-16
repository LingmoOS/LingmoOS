@mainpage



# dtkio

dtkio 是 deepin/uos 系统为应用软件开发提供的基础 I/O 能力的模块，提供文件系统相关的业务能力，包含文件的读写、文件的搜索、文件解压缩、设备的挂载、光盘刻录等相关功能。

## 项目结构

```bash
.
├── CMakeLists.txt
├── debian
├── docs
├── examples
├── include
├── LICENSE
├── LICENSES
├── misc
├── README.md
├── README.zh_CN.md
├── src
├── test-recoverage.sh
└── tests
```

你可以按照仓库中 README.md 中的步骤进行编译，这里就不再赘述。

## 模块

@subpage dtkburn

@subpage dtkcompressor

@subpage dtkio

@subpage dtkmount

@subpage dtksearch