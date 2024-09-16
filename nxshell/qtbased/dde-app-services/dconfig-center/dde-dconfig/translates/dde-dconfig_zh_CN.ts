<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>main</name>
    <message>
        <location filename="../main.cpp" line="365"/>
        <source>A console tool to get and set configuration items for DTK Config.</source>
        <translation>用于管理DTK提供的配置策略的CLI工具</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="368"/>
        <source>operate configure items of the user uid.</source>
        <translation>用户uid， 用户的唯一ID，对指定用户的配置项进行读写等操作</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="371"/>
        <source>appid for a specific application, 
it is empty string if we need to manage application independent configuration.
second positional argument as appid if not the option</source>
        <translation>应用Id，应用程序的唯一ID，当管理应用无关配置时此值为空，如果没有设置这个选项，则使用第二个位置参数当作此选项值</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="376"/>
        <source>resource id for configure name, 
it&apos;s value is same as `a` option if not the option.</source>
        <translation>配置Id，配置描述文件的唯一标识，是配置描述文件的文件名，如果没有设置这个选项，则值与应用Id的值相同</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="380"/>
        <source>subpath for configure.</source>
        <translation>子目录，配置描述文件安装可包含子路径，支持配置描述文件的优先级，通常情况下不设置此值</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="383"/>
        <source>configure item&apos;s key.
three positional argument as key if not the option</source>
        <translation>配置项Key值，如果没有设置这个选项，则使用第三个位置参数当作此选项值</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="387"/>
        <source>new value to set configure item.</source>
        <translation>需要设置的配置项值，此选项在set模式下工作</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="390"/>
        <source>working prefix directory.</source>
        <translation>配置策略的工作目录前缀，通常情况下不设置此值</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="393"/>
        <source>method for the configure item.</source>
        <translation>指定配置项的字段信息，用于获取配置项的详细信息，值为{name | discription | visibility | permissions | version | isDefaultValue}其中一个，此选项在get模式下工作</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="396"/>
        <source>language for the configuration item.</source>
        <translation>指定语言，此选项在get模式下工作，并且需要指定method选项</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="399"/>
        <source>list configure information with appid, resource or subpath.</source>
        <translation>list模式，列出可配置的应用Id，配置Id，及子目录</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="403"/>
        <source>query content for configure, including the configure item&apos;s all keys, value ...</source>
        <translation>get模式，用于查询指定配置项的信息</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="407"/>
        <source>set configure item &apos;s value.</source>
        <translation>set模式，用于设置配置项的值</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="411"/>
        <source>reset configure item&apos;s value, reset all configure item&apos;s value if not `-k` option.</source>
        <translation>reset模式，用于重置配置项的值，此会清除对应的缓存值，若没有指定`-k`选项，则重置此配置文件的所有缓存值</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="415"/>
        <source>watch value changed for some configure item.</source>
        <translation>watch模式，用于监听配置项的更改</translation>
    </message>
    <message>
        <location filename="../main.cpp" line="419"/>
        <source>start dde-dconfig-editor as a gui configure tool.</source>
        <translation>gui模式，用于启动GUI工具，需要安装对应的GUI工具dde-dconfig-editor</translation>
    </message>
</context>
</TS>
