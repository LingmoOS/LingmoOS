\mainpage ocean-control-center
@brief ocean-control-center

# ocean-control-center
## 接口变更记录
| 时间 | 版本 | 说明 | 控制中心版本号 |
|---|---|---|---|
| 2024.11.8 | 1.0 | 创建 | 6.0.71 |
| 2024.12.2 | 1.0 | 修改main.qml为{name}Main.qml(防止翻译冲突，兼容以前命名) | 6.0.77 |

## V25控制中心新特性
1.  V25控制中心只负责框架设计，具体功能全部由插件实现
2.  V25控制中心支持多级插件系统，支持插件插入到任意位置中
3.  更方便、更精确的搜索功能,更好的搜索交互
4.  高度可定制，可定制任意插件是否显示，若插件支持，可定制任意插件内容是否显示
5.  界面采用qml实现，更灵活，更易维护
6.  插件数据采用C++实现，更高效，与界面完全解偶
7.  插件支持多语言，支持多语言切换
8.  插件显示禁用支持统一配置。配置修改立即生效
## V25控制中心插件安装路径必要说明
1.  V25控制中心插件安装路径为`${CMAKE_INSTALL_LIBDIR}/ocean-control-center/plugins_v1.0`
2.  该路径下插件以单个文件夹形式存在，文件夹名为插件名，文件夹内为插件文件,假设插件名为example，则插件文件夹内容为：
```bash
${CMAKE_INSTALL_LIBDIR}/ocean-control-center/plugins_v1.0/example/
├── example.qml
├── example.so
├── exampleMain.qml
└── xxx.qml
```
1.  example.qml为插件元数据文件，包含一个OceanUIObject对象。通常该对象只是插件入口菜单。为了让主界面快速显示出来
2.  example.so为插件c++导出的动态库
3.  exampleMain.qml为插件入口文件，插件启动时，会自动加载该文件，该文件中根对象为一个OceanUIObject对象，该对象可以包含任意qml对象，并且该文件中可以用到example.so导出的函数，使用方式为：oceanuiData.xxx(),oceanuiData为example.so导出的对象
4.  xxx.qml为插件其他文件，在exampleMain.qml中使用
## V25控制中心插件开发说明
1.  V25控制中心插件开发需要安装ocean-control-center-dev包，该包中包含V25控制中心插件开发所需头文件和库文件
2.  V25控制中心使用的是qt6,qt6与qt5混用会导致程序崩溃。因此插件需要使用qt6进行开发
## V25控制中心插件加载顺序说明
1.  插件加载时，会先根据配置判断该插件是否显示，若不显示，则加载结束。查看配置命令：`ocean-dconfig get org.lingmo.ocean.control-center -r org.lingmo.ocean.control-center hideModule`
2.  加载example.qml，若example.qml中根对象OceanUIObject对象visible属性为false，则加载结束
3.  在线程中加载example.so，最后会将example.so导出的对象移动到主线程
4.  将example.so导出的对象设置为oceanuiData,加载exampleMain.qml。此时，exampleMain.qml中可以使用oceanuiData.xxx()调用example.so导出的函数
5.  加载完成，将OceanUIObject对象插入到模块树中
## V25控制中心插件开发必要说明
1.  控制中心有一个option,可以用来加载一个文件夹下的插件，比如一般插件会放置到`build`文件夹下，这时候可以`ocean-control-center --spec ./lib/plugins_v1.0`来加载单独一个插件进行调试。另外提醒，调试时候不要使用asan，因为没有使用asan的控制中心无法加载使用了asan编译的插件
2.  控制中心插件加载是在线程中，但最终会将插件对象移到主线程。所以example.so构造函数中创建的对象需要在example.so导出类的树结构中(即子对象的父对象或祖先对象是example.so导出类)，否则不会被移动到主线程，导致其中信号槽线程等不到，无法正常使用。
3.  example.so导出类是唯一的，插件中不建议使用单例，可在example.so导出类中创建一个单例对象
## V25控制中心开发接口说明
控制中心导出的qml类有：
### 关键类
#### OceanUIObject
| 属性名称 | 说明 | 备注 |
|---|---|---|
| name | 名称 | 作为唯一id使用，结合父项的name组成url,用于定位跳转、配置隐藏禁用等，由字符、数字组成，不建议有符号空格，不可有‘/’(url分隔符，会影响解析) |
| parentName | 父项名称 | 父项的url,表示该项是哪个项的子项。此处可以是一个url,如：“aa/bb/cc” |
| weight | 权重 | 权重越高，该项所插入的位置越靠后。取值范围：0-65535，建议取值用10、20、30的方式，方便有需求要从中间插入控件 |
| displayName | 显示名称 | 用于搜索、显示，需支持翻译 |
| description | 描述 | 用于显示 |
| icon | 图标 | 图标名 |
| badge | 标识 | 用于显示红色圆点等，如：更新的红点提示，取值范围：0-255 |
| visible | 可见 | 与控件显示关联，默认true |
| enabled | 启用 | 与控件状态关联，默认true |
| visibleToApp | 可见 | 只读，包含配置与visible的结果，与控件显示关联 |
| enabledToApp | 启用 | 只读，包含配置与enabled的结果，与控件状态关联 |
| canSearch | 可搜索 | 默认true |
| backgroundType | 背景样式 | 默认AutoBg |
| pageType | 界面类型 | Menu、Editor、Item等，影响page显示方式，取值范围：0-255 |
| page | 界面控件 |  |
| parentItem | 控件父项 |  |

| 信号 | 说明 | 备注 |
|---|---|---|
| active | 激活 | backgroundType为Clickable时，点击控件出发，参数为空。DBus的ShowPage方法出发，如:ShowPage("aa/bb?param=1")，则aa/bb项会收到active("param=1")信号 |
| deactive | 停用 | 页面退出时触发 |
#### OceanUIApp 全局单例
| 函数 | 说明 | 备注 |
|---|---|---|
| root | 根结点 | 属性 |
| activeObject | 当前菜单项 | 属性 |
| addObject | 添加Object | 将OceanUIObject加到模块树上,方法 |
| removeObject | 移除Object | 将OceanUIObject从模块树上移除，动态创建的Object需要手动destroy,方法 |
| showPage | 跳转页面 | 若url参数为空，会按Object的父项查找，直到找到Menu类型的Object,将其设置为当前页面。url参数不为空，则找到对应项触发active信号,方法 |
| mainWindow | 主窗口 | 方法 |
| activeItemChanged | 搜索或showPage对应的控件，常用于强提醒显示 | 信号 |
### 辅助类
#### OceanUIModel
以根结点的子项组成一个ListModel
| 属性名称 | 说明 | 备注 |
|---|---|---|
| root | 根结点 |  |
#### OceanUIRepeater
使用提供的model实例化多个基于OceanUIObject的对象，并添加到父项中，与Repeater类似
#### OceanUIDBusInterface
与DBus交互的类，支持属性、信号、方法
### 界面类
#### OceanUIGroupView
一个组样式的控件
#### OceanUIRightView
控制中心右侧样式控件，Menu类型的OceanUIObject未指定page时，会自动使用该控件
#### OceanUISettingsView
与OceanUIRightView类似，但可以显示一个下方悬浮区域，需要其对应的OceanUIObject有两个子项
#### OceanUIItemBackground
处理控件背景的控件，在OceanUIRightView中用到
## 代码示例：
### 代码文件夹结构
假设插件名为example，代码文件夹内容为：
```plain
plugin-example
├── CMakeLists.txt          # CMake构建脚本，用于编译和构建插件
├── qml                     # QML文件目录
│   ├── oceanui_example.dci     # 图标文件
│   ├── ExamplePage1.qml    # 第一个示例页面的QML文件，在exampleMain.qml中加载
│   ├── ExamplePage2.qml    # 第二个示例页面的QML文件，在exampleMain.qml中加载
│   ├── example.qml         # 主QML文件，包含简单的插件信息
│   └── exampleMain.qml     # 主QML文件，包含插件所有页面
└── src                     # 源文件目录，存放C++源文件和相关头文件
    ├── pluginexample.cpp   # 插件的C++实现文件，包含功能实现和QML与C++的交互
    ├── pluginexample.h     # 插件的头文件，定义类、函数和QML中可能用到的接口
    └── resources.qrc       # qrc资源文件（可选）
```

（文件说明按模块加载顺序说明）

### CMakeLists.txt
```bash
cmake_minimum_required(VERSION 3.7)

set(PLUGIN_NAME "example")

find_package(Qt6 COMPONENTS Core LinguistTools REQUIRED) # oceanui_handle_plugin_translation中用到LinguistTools的函数
find_package(DdeControlCenter REQUIRED) # 查找ocean-control-center库

file(GLOB_RECURSE PLUGIN_SRCS
        "src/*.cpp"
        "src/*.h"
        # "src/qrc/example.qrc"
)
add_library(${PLUGIN_NAME} MODULE
        ${PLUGIN_SRCS}
)

# target_include_directories(${PLUGIN_NAME} PUBLIC
#     Dde::Control-Center
# )

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Dde::Control-Center # 添加ocean-control-center库
    Qt6::Core
)
# 处理插件安装
oceanui_install_plugin(NAME ${PLUGIN_NAME} TARGET ${PLUGIN_NAME})
# 处理翻译和安装，如果自己处理翻译，可以不调用该函数
oceanui_handle_plugin_translation(NAME ${PLUGIN_NAME} )
```
### example.qml
```javascript
import org.lingmo.oceanui 1.0

// 该文件中不能使用oceanuiData,根对象为OceanUIObject
OceanUIObject {
    id: root
    name: "example"
    parentName: "root"
    displayName: qsTr("Example")
    icon: "oceanui_example"
    weight: 1000

    visible: false // 控制模块显示，如果模块不显示，则不会加载example.so和example.qml
    OceanUIDBusInterface { // 控制中心导致的qml类，可使用DBus。如果是用DConfig,dtk有导出D.Config类，可以直接使用
        property var windowRadius // 关注的dbus属性
        service: "org.lingmo.ocean.Appearance1"
        path: "/org/lingmo/ocean/Appearance1"
        inter: "org.lingmo.ocean.Appearance1"
        connection: OceanUIDBusInterface.SessionBus
        onWindowRadiusChanged: { // dbus属性变化信号
            root.visible = windowRadius > 0
        }
        // on为关键字，关联DBus信号
        function onChanged(type, value) { // dbus信号
            console.log("Changed signal received, type: ", type, ", value: ", value)
            // 调用DBus的List方法，["gtk"]为参数，多个参数示例:["str",1,"str"]，listSlot为处理DBus调用返回，参数个数为DBus返回的参数个数。listErrorSlot为处理DBus错误返回，1个参数，为错误字符串
            callWithCallback("List", ["gtk"], listSlot, listErrorSlot)
        }
        function listSlot(ty) {
            console.log("List slot received, type: " + ty)
        }
        function listErrorSlot(error) {
            console.log("error", error)
        }
    }
}
```
### pluginexample.h
```cpp
class PluginExample : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
public:
    explicit PluginExample(QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &name);
    // 属性、Q_INVOKABLE、信号、槽等可在qml中直接使用
    Q_INVOKABLE int calc(int a, int b);

public Q_SLOTS:
    void setCalcType(int type);

Q_SIGNALS:
    void nameChanged(const QString &name);
    void calcTypeChanged(int calcType);
};
```
### pluginexample.cpp
pluginexample.cpp为PluginExample类实现，与控制中心插件相关内容为：

```cpp
#include "oceanuifactory.h"
DCC_FACTORY_CLASS(PluginExample) // DCC_FACTORY_CLASS在oceanuifactory.h中定义，用于注册插件,该宏会自动生成PluginExampleFactory类，并实现create函数。PluginExampleFactory类为Qt类，所以需要包含pluginexample.moc
#include "pluginexample.moc"
```
### exampleMain.qml
```javascript
import org.lingmo.oceanui 1.0

// 该文件中可以使用oceanuiData,根对象为OceanUIObject
OceanUIObject {
    ExamplePage1 {
        name: "example_1"
        parentName: "example"
        displayName: qsTr("Normal Page")
        icon: "oceanui_example"
        weight: 10
    }
    ExamplePage2 {
        name: "example_2"
        parentName: "example"
        displayName: qsTr("Settings Page")
        icon: "oceanui_example"
        weight: 20
    }
}
```
### ExamplePage1.qml
```javascript
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    property real calcValue
    property real calcType: 0
    OceanUIObject {
        name: "calc"
        parentName: root.name
        displayName: qsTr("calc")
        icon: "oceanui_example"
        weight: 10
        backgroundType: OceanUIObject.Normal // 设置背景样式
        pageType: OceanUIObject.Editor // Editor为page是右边的控件，左边显示displayName、icon等
        page: Button {
            text: oceanuiObj.displayName
            onClicked: {
                calcValue = oceanuiData.calc(calcValue, 2)
            }
        }
    }
    OceanUIObject {
        name: "value"
        parentName: root.name
        displayName: qsTr("value")
        weight: 20
        pageType: OceanUIObject.Editor
        backgroundType: OceanUIObject.ClickStyle // ClickStyl表示有点击效果，点击时会发出active信号
        page: RowLayout {
            Text {
                text: calcValue
            }
            ComboBox {}
        }
        onActive: cmd => console.log(this, "onActive:", cmd)
    }
    OceanUIObject {
        name: "group"
        parentName: root.name
        displayName: qsTr("group")
        weight: 30
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {} // 组效果，其子项会放在一个组里
        OceanUIObject {
            name: "item2" // name要求当前组内唯一
            parentName: root.name + "/group" // parentName要求可定位到对应项，可用多个OceanUIObject的name组合
            displayName: qsTr("value")
            weight: 20
            pageType: OceanUIObject.Item // Item的page将占整个区域
            page: Text {
                text: calcValue
            }
        }
        OceanUIObject {
            name: "item1"
            parentName: root.name + "/group"
            displayName: qsTr("value")
            weight: 10
            pageType: OceanUIObject.Editor
            page: Text {
                text: calcValue
            }
        }
        OceanUIObject {
            id: calcTypeObj
            name: "calcType"
            parentName: root.name
            displayName: qsTr("calc type")
            description: qsTr("description")
            icon: "oceanui_example"
            weight: 30
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Button {
                text: oceanuiObj.displayName
                onClicked: {
                    calcType++
                    if (calcType >= 4) {
                        calcType = 0
                    }
                    oceanuiData.setCalcType(calcType)
                }
            }
            Connections {
                target: oceanuiData
                function onCalcTypeChanged(cType) {
                    calcTypeObj.displayName = cType
                }
            }
        }
        OceanUIObject {
            name: "calcType" // 该OceanUIObject会显示在example_2中，不建议这样写
            parentName: "example_2/body" // OceanUIObject位置只与parentName和weight有关，与其自身位置无关
            weight: 80
            displayName: qsTr("calc type")
            pageType: OceanUIObject.Editor
            backgroundType: OceanUIObject.Normal
            page: Text {
                text: calcType
            }
        }
    }

    OceanUIObject {
        name: "group2"
        parentName: root.name
        displayName: qsTr("group2")
        weight: 40
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}
        OceanUIRepeater { // OceanUIRepeater配合model可实现多个OceanUIObject
            model: 3
            delegate: OceanUIObject {
                name: "item" + (index + 1)
                parentName: root.name + "/group2"
                displayName: qsTr("Item") + (index + 1)
                weight: 30 + index
                pageType: OceanUIObject.Editor
                page: Switch {}
            }
        }
    }
}
```
### ExamplePage2.qml
```javascript
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D

import org.lingmo.oceanui 1.0

OceanUIObject {
    id: root
    property real calcValue
    property real calcType: 0

    weight: 10
    page: OceanUISettingsView {} // 设置界面，此OceanUIObject的pageType为Menu，page为OceanUISettingsView
    OceanUIObject {
        name: "body" // OceanUISettingsView里的限制，其子项必须是两个OceanUIObject，第一个为主界面，第二个为下方按钮区域
        parentName: root.name
        weight: 10
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "calc"
            parentName: root.name + "/body"
            displayName: qsTr("calc")
            icon: "oceanui_example"
            weight: 10
            backgroundType: OceanUIObject.Normal
            pageType: OceanUIObject.Editor
            page: Button {
                text: oceanuiObj.displayName
                onClicked: {
                    calcValue = oceanuiData.calc(calcValue, 2)
                }
            }
        }
        OceanUIObject {
            name: "value"
            parentName: root.name + "/body"
            displayName: qsTr("no Search")
            canSearch: false // 设置界面通常不搜索，可设置canSearch
            weight: 20
            pageType: OceanUIObject.Item
            page: Text {
                text: calcValue
            }
        }
        OceanUIObject {
            name: "menuEditor"
            parentName: root.name + "/body"
            displayName: qsTr("no Search")
            canSearch: false // 设置界面通常不搜索，可设置canSearch
            weight: 30
            pageType: OceanUIObject.MenuEditor // 菜单加编辑控件,子项是一个菜单项
            page: Switch {
            }
            OceanUIObject {
                name: "menu"
                parentName: root.name + "/body/menuEditor"
                weight: 10
                OceanUIRepeater {
                    model: 8
                    delegate: OceanUIObject {
                        name: "item" + (index + 1)
                        parentName: "menuEditor/menu"
                        displayName: qsTr("Item") + (index + 1)
                        weight: 30 + index
                        backgroundType: OceanUIObject.Normal
                        pageType: OceanUIObject.Editor
                        page: Switch {}
                    }
                }
            }
        }

        OceanUIObject {
            name: "group2"
            parentName: root.name + "/body"
            displayName: qsTr("group2")
            weight: 100
            pageType: OceanUIObject.Item
            page: OceanUIGroupView {}

            OceanUIObject {
                name: "item0"
                parentName: root.name + "/body/group2"
                displayName: qsTr("value")
                weight: 20
                pageType: OceanUIObject.Item
                page: Rectangle {
                    implicitHeight: 50
                    Text {
                        anchors.centerIn: parent
                        text: calcValue
                    }
                }
            }
            // OceanUIRepeater类可配合model实现多个OceanUIObject
            OceanUIRepeater {
                model: 23
                delegate: OceanUIObject {
                    name: "item" + (index + 1)
                    parentName: root.name + "/body/group2"
                    displayName: qsTr("Item") + (index + 1)
                    weight: 30 + index
                    pageType: OceanUIObject.Editor
                    page: Switch {}
                }
            }
        }
    }
    OceanUIObject {
        name: "footer"
        parentName: root.name
        weight: 20
        pageType: OceanUIObject.Item
        OceanUIObject {
            name: "delete"
            parentName: root.name + "/footer"
            weight: 10
            pageType: OceanUIObject.Item
            page: Button {
                text: qsTr("Delete")
                onClicked: {
                    deleteDialog.createObject(this).show()
                }
            }
        }

        Component {
            id: deleteDialog
            D.DialogWindow {
                modality: Qt.ApplicationModal
                width: 380
                icon: "preferences-system"
                onClosing: destroy(10)
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.fillWidth: true
                        Layout.leftMargin: 50
                        Layout.rightMargin: 50
                        text: qsTr("Are you sure you want to delete this configuration?")
                        font.bold: true
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }
                    RowLayout {
                        Layout.topMargin: 10
                        Layout.bottomMargin: 10
                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Cancel")
                            onClicked: close()
                        }
                        Rectangle {
                            implicitWidth: 2
                            Layout.fillHeight: true
                            color: this.palette.button
                        }

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Delete")
                            onClicked: {
                                close()
                            }
                        }
                    }
                }
            }
        }
        OceanUIObject {
            // 按钮区域可加个空项处理右对齐问题
            name: "spacer"
            parentName: root.name + "/footer"
            weight: 20
            pageType: OceanUIObject.Item
            page: Item {
                Layout.fillWidth: true
            }
        }
        OceanUIObject {
            name: "cancel"
            parentName: root.name + "/footer"
            weight: 30
            pageType: OceanUIObject.Item
            page: Button {
                text: qsTr("Cancel")
                onClicked: {
                    OceanUIApp.showPage(root.parentName)
                }
            }
        }
        OceanUIObject {
            name: "save"
            parentName: root.name + "/footer"
            weight: 40
            pageType: OceanUIObject.Item
            page: Button {
                text: qsTr("Save")
                onClicked: {
                    calcValue = oceanuiData.calc(calcValue, 3)
                }
            }
        }
    }
}
```
