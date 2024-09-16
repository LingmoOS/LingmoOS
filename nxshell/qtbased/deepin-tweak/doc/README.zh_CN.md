## 目录结构

deepin-tweak 接受的插件目录必须具备以下结构:

- assets 目录

  存储图片等素材

- translations 目录

  存放 Qt 的翻译文件

- main.qml

  插件主界面

## 主界面

deepin-tweak 会加载插件的主界面到窗口中，主界面的 QML 不能是 Window 元素。

### 插件元信息

在 `main.qml` 中需要提供以下属性作为插件的元信息:

- displayName
- description
- version
- author
- icon

```qml
property string displayName: "Test Name"
property string description: "Test description"
property string version: "1.0.0"
property string author: "justforlxz"
property string icon: "icon.png"
```

## 调用接口

deepin-tweak 内置了执行命令、读写文件及 DBus 调用的接口。

tweak 提供了一个 Tweak 模块，在 qml 文件头部使用 `import org.deepin.tweak 1.0` 导入 tweak 模块，就可以使用 `Tweak` 对象。

Tweak 对象提供了以下方法用于获取特定功能支持的对象。

### 执行命令(newLauncher)

`Launcher` 对象提供了 `create` 方法创建一个执行实例，并通过链式调用设置相关参数，最终通过 `call` 方法或者 `async` 方法启动执行，并获取结果。

目前支持的参数有:

- program

  设置启动的命令。

- arguments

  设置传递给启动命令的参数。

- timeout

  设置命令超时的时间，默认情况下将一直等待执行结果。

`call` 的返回值与 `asyncCall` 的回调函数是一个对象，拥有 `allStandardOutput` 属性与 `allStandardError` 属性。

### 读写文件(newFile)

需要传入 File 对象操作的文件路径。

File 对象拥有以下方法:

- open

  设置文件的打开方式，可以在 qml 中使用 FileMode 枚举进行设置:

  - NotOpen

    选择不打开文件。

  - ReadOnly

    选择只读，选择此模式将无法使用 write 方法。

  - WriteOnly

    选择只写，选择此模式将无法使用 read 方法。

  - ReadWrite

    选择可读可写，选择此模式可以随意使用 read 与 write 方法。

  - Append

    使用此模式时，write 方法会将内容在文件末尾添加。

  - Truncate

    如果可能，设备在打开之前将被截断。设备的所有早期内容都将丢失。

  - Text

    读取时，行尾终止符将转换为“\n”。写入时，行尾终止符将转换为本地编码，例如 Win32 的“\r\n”。

  - Unbuffered

    设备中的任何缓冲区都会被绕过。

  - NewOnly

    如果要打开的文件已经存在，则失败。只有当文件不存在时才创建并打开它。操作系统保证您是唯一创建和打开该文件的人。注意，这种模式意味着 WriteOnly，并且允许将其与 ReadWrite 结合使用。该标志目前只影响QFile。其他类将来可能会使用这个标志，但在此之前，对QFile以外的任何类使用这个标志可能会导致未定义的行为。

  - ExistingOnly

    如果要打开的文件不存在，则会失败。此标志必须与 ReadOnly、WriteOnly 或 ReadWrite 一起指定。注意，单独使用 ReadOnly 标志是多余的，因为当文件不存在时，ReadOnly 已经失败了。

- close

  关闭已打开的文件。

- flush

  强制刷新缓冲区，将缓冲区的内容实际的写入文件。

- read

  读取指定大小的内容。

- readAll

  读取文件的全部内容。

- write

  写入数据到文件。

- exists

  判读文件是否存在。

- rename

  对文件进行重新命名

- remove

  删除指定文件

- seek

  设置 read 函数开始读取的位置

- pos

  获取当前文件 read 的位置

- size

  获取当前文件的大小

### settings(newSettings)

需要传入操作的 ini 配置文件的路径。

- value

  获取某个 key 对应的值

- setValue

  设置某个 key 的值

- beginGroup

  设置读取的组，切换不同的组时必须先调用 endGroup 方法。

- endGroup

  结束组的标记

- keys

  获取所有的 key

### GSettings(newGSettings)

需要传递初始化的 gsettings schema id 和 path。

- changed

  传递一个函数签名为 (key, value) 的回调函数，回调函数将会在 schema id 下的值发生变化时调用。

- contains

  判断 schema id 是否包含指定的 key。

- get

  获取指定 key 对应的值。

- set

  设置指定的 key 对应的值。

- keys

  获取 schema id 所有的 key。

- choices

  列出指定 key 可选择的值。

- reset

  将指定 key 重设为默认值。

### DBus 调用

tweak 使用 nemo.dbus 为 qml 提供 DBus 调用的能力。

在 qml 文件中导入 `import Nemo.DBus 2.0`，并使用
tweak 使用 Nemo.DBus 为 qml 提供 DBus 调用的能力。

在 qml 文件中导入 `import Nemo.DBus 2.0`，并创建一个 DBusInterface 对象执行 DBus 调用。

DBusInterface 提供了 `service`、`iface` 和 `path` 属性用来初始化 DBus 连接。

```qml
DBusInterface {
  id: profiled
  service: 'org.kde.KWin'
  iface: 'org.kde.kwin.Effects'
  path: '/Effects'
}
```

在函数中调用  call 方法执行 DBus 调用。

```
profiled.call('loadEffect', ['blur']);
```

### 插件示例

```qml
import QtQuick.Layouts 1.7
import QtQuick 2.4
import org.deepin.dtk 1.0

RowLayout {
  property string displayName: "Test Name"
  property string description: "Test description"
  property string version: "1.0.0"
  property string author: "justforlxz"
  property string icon: "icon.png"
  Text {
      text: "input"
  }
  LineEdit {}
}
```

更多插件示例在 [demos](demos) 目录查看。
