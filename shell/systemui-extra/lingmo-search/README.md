# lingmo-search介绍

[dWIP] LINGMO Search is a user-wide desktop search feature of LINGMO desktop environment.

## 简介
狭义上的lingmo-search指lingmo桌面环境中的全局搜索应用，目前最新版本为4.0.x.x。全局搜索应用提供了本地文件、文本内容、应用、设置项、便签等聚合搜索功能，基于其文件索引功能，可以为用户提供快速准确的搜索体验。

广义的lingmo-search除了包括全局搜索应用，还包括在lingmo桌面环境中的本地搜索服务以及其开发接口。基于文件索引服务，应用搜索数据服务等基础数据源服务，可以提供基于C++接口的搜索功能，应用开发者可以通过引用动态库的形式直接使用其搜索功能。除此之外，lingmo桌面环境搜索服务还提供了一组基于Qt插件框架的插件接口，用户可以通过继承接口以实现搜索功能的扩展。
以下提到的lingmo-search如无说明均指后者。

lingmo-search 目前被打包成9个包（lingmo）：
+ lingmo-search_xxxxxx.deb
+ lingmo-search-service_xxxx.deb
+ liblingmo-search-dev_xxxxx.deb
+ liblingmo-search2_xxxxx.deb
+ liblingmo-search-common_xxxxx.deb
+ libchinese-segmentation1_xxxx.deb
+ libchinese-segmentation-dev_xxxx.deb
+ libchinese-segmentation-common_xxxx.deb
+ lingmo-search-systemdbus_xxxxx.deb

xxx代表版本号。其中，lingmo-search 为全局搜索应用本体，lingmo-search-service为搜索数据服务相关进程，liblingmo-search包提供了搜索服务基本功能以及扩展接口，liblingmo-search-dev为其开发包。libchinese-segmentation包为搜索服务提供了NLP能力，如中文分词等。lingmo-search-systemdbus包提供了一些systemdbus提权操作。

## 运行
搜索服务相关的进程共有5个，包括lingmo-search(全局搜索GUI界面)，lingmo-search-service(文件搜索服务)，lingmo-search-service-dir-manager(文件搜索目录管理模块), lingmo-search-app-data-service(应用数据服务),lingmosearch-systemdbus(systembus)。

所有进程默认开机自启。

## 快捷键、命令行和dbus接口

呼出搜索GUI界面的系统快捷键为`WIN+s`，快捷键由lingmo-settings-daemon(lingmo用户配置服务)提供。

lingmo-search进程的命令行如下：

```shell
Usage: lingmo-search [options]
Options:
  -h, --help     Displays this help.
  -v, --version  Displays version information.
  -q, --quit     Quit lingmo-search application
  -s, --show     Show main window
  --unregister <pluginName>  unregister a plugin with <pluginName>
  --register <pluginName>    register a plugin with <pluginName>
  -m, --move <pluginName>    move <pluginName> to the target pos
  -i, --index <index>        move plugin to <index>

```

lingmo-search-service的命令行如下：

```shell
Usage: lingmo-search-service [options]
Options:
  -h, --help            Displays this help.
  -v, --version         Displays version information.
  -q, --quit            Stop service
  -i, --index <option>  start or stop file index
  -m, --monitor         Show index monitor window
```

lingmo-search 提供的dbus接口：

```
service: com.lingmo.search.service
path: /
interface: com.lingmo.search.service
    mainWindowSwitch() ↦ () //显示或关闭主窗口
    showWindow () ↦ () //显示搜索主窗口
    searchKeyword (String keyword) ↦ () //显示主窗口并搜索传入的关键字
```

## 交互

搜索的功能有一部分依赖于其他桌面环境组件：

设置项搜索：依赖lingmo-control-center提供的dbus接口：

```
service:org.lingmo.ukcc.session
path:/
interface:org.lingmo.ukcc.session.interface
method:getSearchItems () ↦ (Dict of {String, Variant} arg_0)
signal:searchItemsAdd(Dict of{String, Variant})
       searchItemsDelete(Dict of{String, Variant})
```

跳转到搜索结果对应的控制面板页面使用了lingmo-control-center的命令行：

```shell
Usage: lingmo-control-center [options]
Options:
  -h, --help     Displays this help.
  -v, --version  Displays version information.
  -m <module>    display the specified module page  //全局搜索使用的是这个命令

```

在线应用搜索：依赖lingmo-software-center提供的dbus接口：

```
service: com.lingmo.softwarecenter.getsearchresults
path: /com/lingmo/softwarecenter/getsearchresults
interface: com.lingmo.getsearchresults
method:get_search_result (String keyword) ↦ (Boolean arg_1)
```

跳转到软件商店安装页面的使用了以下dbus接口：

```
service: com.lingmo.softwarecenter
path: /com/lingmo/softwarecenter
interface: com.lingmo.utiliface
        show_search_result (String appname) ↦ (Array of [Dict of {String, String}] arg_1)
```

如果软件商店未打开或接口不可用时，则调用以下命令行：

```shell
lingmo-software-center -find <包名>
```

便签本搜索：依赖lingmo-notebook提供的dbus接口：

```
service：org.lingmo.note
path：/org/lingmo/note
interface：org.lingmo.note.interface
         keywordMatch (Array of [String] keyList) ↦ (Dict of {String, Variant} arg_0)
```

打开文件所在路径功能调用了explor提供的dbus接口：

```
service：org.freedesktop.FileManager1
path：/org/freedesktop/FileManager1
interface: org.freedesktop.FileManager1
         ShowItems (Array of [String] uriList, String startUpId) ↦ ()
```

## 原理与功能特点

全局搜索支持控制面板设置项搜索，应用搜索，文件搜索，便签本搜索。支持名称，拼音，或拼音首字母搜索（文本内容搜索和便签本搜索不支持拼音搜索）。其中，设置项搜索通过控制面板提供dbus接口获取数据，打开对应的控制面板页面也依赖与控制面板提供的命令行；应用搜索分为本地已安装应用（包括安卓兼容应用）和软件商店已上架的在线应用，在线应用的搜索和跳转安装通过软件商店提供的接口实现。所以，当怀疑搜索的设置搜索或应用搜索有问题时，可以直接测试控制面板或软件商店对应的接口。

文件搜索分为文件名（文件夹名）搜索和文本内容搜索。文件搜索有两种模式：`直接搜索`和`建立索引搜索`。

+ 直接搜索：类似文件管理的搜索，通过遍历匹配关键字搜索，不支持文本内容搜索。

+ 索引搜索：搜索通过遍历文件系统建立数据库（需要消耗一定的时间和资源），搜索时直接对数据库进行搜索，可以实现毫秒级的搜索响应，建立索引的过程中，搜索结果可能不全或者搜不出结果。
首次打开索引时，lingmo-search-service进程会新建两个数据库分别存储基础索引信息（用于文件名搜索）和文本内容索引信息（用于文本内容搜索），完成首次索引后，索引服务会依赖inotify机制进行实时监听更新。索引关闭再打开或重启服务时，索引服务会对遍历文件并对数据库进行校验以增量更新。
索引数据库会基于文件系统监听进行实时更新。但是由于解析文本需要时间，所以大文件的索引新可能会有短暂的延迟。由于各种意外原因，比如索引更新过程中掉电关机，可能会导致索引损坏，此时搜索在下次开机时会重新建立索引来保证正常的文件搜索功能。基于机器配置和本地文件的数量，大小以及种类，索引重建的时间可以从几秒到数分钟不等。
搜索目录可以在控制面板中手动配置，目前索引已经支持外接设备。
索引搜索支持文本内容搜索，基本原理可以参考 [倒排索引与优灵墨的文件搜索](https://docs.qq.com/doc/DU0p0S1lRelp2aW1y) 。建立索引时，搜索会对常用的文本文件进行解析，提取关键词存入数据库。搜索时，用户输入的文本也会被提取关键词，和数据库中的关键词进行匹配， 所以文本索引并不能保证你搜索一个文本文件里的任意内容都能搜出这个文件，这也不是普遍的应用场景。搜索输入的文本中必须要包含【关键词】才可以。比如你搜索一个‘的’，由于‘的’并不是任何文件的关键词，所以并不会有搜索到任何文件。事实上，我们有一个停用词词库，专门用来排除‘我’‘的’于是‘等等基本上在每个文档都会出现的一些无用词。目前，搜索支持解析的文件格式有：docx，pptx, xlsx, txt(大部分编码格式),  doc, dot, wps, ppt, pps, dps, et, xls, pdf，uof，uot，uos，uop，ofd以上格式均不支持加密文件的解析，此外，文件索引支持图片ocr提取文字，所以你也可以通过图片中的文字搜索到图片（就像文档一样），支持的图片格式：png，bmp，gif，tif，tiff，webp，jpe，jpg，jpeg。

> 注意：应用的.desktop文件并不是应用本身或者“快捷方式”，对于搜索来说它只是一个文件，所以搜索desktop文件的名字并不能搜出这个应用，除非它恰好和应用重名。另外，在文件搜索中显示的dekstop文件并不会以应用的形式显示，而是显示它本来的样子——一个文件。

## 配置文件与用户数据

lingmo-search应用和lingmo-search-service、lingmo-search-app-data-service的配置文件以及用户数据都保存在如下路径：

> ~/.config/org.lingmo/lingmo-search

文件说明：

+ lingmo-search.conf -------------------------------------全局搜索GUI配置文件。
+ lingmo-search-plugin-order.conf -------------------搜索插件显示顺序
+ lingmo-search-block-dirs.conf ----------------------文件搜索黑名单，在控制面板中设置
+ lingmo-search-index-status.conf -------------------文件索引服务状态记录
+ lingmo-search-current-indexable-dir.conf -------搜索目录配置文件
+ index_data --------------------------------------------文件索引数据库
+ content_index_data --------------------------------文本内容数据库

## 编译

下载源码

根据debian/control文件安装编译依赖

```shell
mkdir build;cd build;qmake ..;make
```

编译会生成的二进制文件：

+ lingmo-search（搜索应用）
+ lingmo-search-service（搜文件索引服务）
+ lingmo-search-app-data-service（应用数据服务）
+ lingmo-search-systemdbus（文件索引服务的system dbus提权接口）

库文件：

+ libchinese-segmentation.so（中文分词）
+ liblingmo-search.so（提供搜索服务和搜索应用的API）
+ libsearch-ukcc-plugin.so(lingmo-contorl-center插件)

## 调试

lingmo-search目前并未采用lingmo-log4qt模块的日志功能。如需调试，可在~/.config/org.lingmo/目录新建`lingmo-search.log`、`lingmo-search-service.log`以及`lingmo-search-app-data-service.log`文件，分别对应全局搜索GUI应用，全局搜索文件索引服务和应用数据服务。新建日志文件后，日志会自动打印到对应文件中，但目前日志没有自动备份或删除机制。

## 开发接口

### 搜索服务接口(此接口目前快速更新，请以代码为准)

#### Use with CMake:

```cmake
find_package(PkgConfig)
pkg_check_modules(lingmo-search REQUIRED lingmo-search)
include_directories(${lingmo-search_INCLUDE_DIRS})
target_link_libraries(yourapp lingmo-search)
```

#### Use with Qmake：

```
CONFIG += link_pkgconfig
PKGCONFIG += lingmo-search
```

使用示例：

```c++
#include <LingmoUISearchTask>
......
//初始化一个搜索实例
LingmoUISearch::LingmoUISearchTask ukst;    
//初始化需要用到的搜索插件
ukst.initSearchPlugin(LingmoUISearch::SearchProperty::SearchType::File);
//初始化队列
LingmoUISearch::DataQueue<LingmoUISearch::ResultItem> *queue = ukst.init();    
//设置最大结果数量（默认为100）
ukst.setMaxResultNum(999999);  
//添加搜索文件夹
QString path = "/home/usr/下载";
ukst.addSearchDir("path"); 
//设置需要的信息，将被储存在 LingmoUISearch::ResultItem中
ukst.setResultProperties(LingmoUISearch::SearchProperty::SearchType::File,
                                 LingmoUISearch::SearchResultProperties{LingmoUISearch::SearchProperty::FilePath,
                                  LingmoUISearch::SearchProperty::FileIconName});
//添加关键词，支持添加多个关键词，用 ‘与’的关系搜索,注意，当需要重新添加关键词时需要调用‘clearKeyWords清空关键词’
ukst.addKeyword(searchText);
//添加搜索条件
ukst.setOnlySearchFile(true);   
//执行搜索，参数表示执行搜索的搜索插件，注意每次搜索之前可以调用‘’
ukst.startSearch(LingmoUISearch::SearchProperty::SearchType::File);
//接收结果（示例）
while(!queue->isEmpty()) {
    auto result = queue->dequeue();
    //通过属性取值
    qDebug() << result.getValue(LingmoUISearch::SearchProperty::FilePath);
    //直接获取所有值
    LingmoUISearch::SearchResultPropertyMap map = result.getAllValue();
    qDebug() << map;
}
```

目前搜索服务内置的可初始化的插件有（目前仅支持文件，文本和应用搜索）：

```c++
enum class SearchType{    
    File             = 0x1 << 0,    
    FileContent      = 0x1 << 1,    
    Application      = 0x1 << 2,    
    Setting          = 0x1 << 3,    
    Note             = 0x1 << 4,    
    Mail             = 0x1 << 5,    
    Custom           = 0x1 << 6
};
```

### 搜索服务插件接口

除了上面的内置插件，用户可以通过集成插件接口实现自定义搜索插件：

```c++
namespace LingmoUISearch {
class SearchTaskPluginIface : public QObject, public PluginInterface{    
    Q_OBJECT
    public:    
    virtual QString getCustomSearchType() = 0;    
    virtual SearchType getSearchType() = 0;    
    //Asynchronous,multithread.    
    virtual void startSearch(std::shared_ptr<SearchController> searchController) = 0;    
    virtual void stop() = 0;
    Q_SIGNALS:    
    void searchFinished(size_t searchId);
};
}
Q_DECLARE_INTERFACE(LingmoUISearch::SearchTaskPluginIface, SearchTaskPluginIface_iid)
```

调用方法和上面的类似，只是需要在初始化插件和启动搜索的时候，指定用户自定的插件名称（用户插件默认启动即加载）：

表示加载用户插件

```c++
ukst.initSearchPlugin(LingmoUISearch::SearchType::Custom, "<用户自定义的名称>");  
```

启动搜索

```c++
ukst.startSearch(LingmoUISearch::SearchType::Custom, "<用户自定义的名称>";
```

### 搜索应用插件接口

搜索应用本身也提供了一个插件接口，可以通过加载用户实现的插件以实现额外搜索以及详情页定制功能：

```c++
class SearchPluginIface : public PluginInterface
{
public:
    enum InvokableAction
    {
        None                     = 1u << 0,
        HideUI                   = 1u << 1
    };
    Q_DECLARE_FLAGS(InvokableActions, InvokableAction)

    struct DescriptionInfo
    {
        QString key;
        QString value;
    };
    struct Actioninfo
    {
        int actionkey;
        QString displayName;
    };
    /**
     * @brief The ResultInfo struct
     */
    struct ResultInfo
    {
        QIcon icon;
        QString name;
        QVector<DescriptionInfo> description;
        QString actionKey;
        int type;
        ResultInfo(const QIcon &iconToSet = QIcon(), const QString &nameToSet = QString(),
                   const QVector<DescriptionInfo> &descriptionToSet = QVector<DescriptionInfo>(),
                   const QString &actionKeyToSet = QString(), const int &typeToSet = 0) {
            icon = iconToSet;
            name = nameToSet;
            description = descriptionToSet;
            actionKey = actionKeyToSet;
            type = typeToSet;
        }
    };

    virtual ~SearchPluginIface() {}
    virtual QString getPluginName() = 0;
    virtual void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult) = 0;
    virtual void stopSearch() = 0;
    virtual QList<Actioninfo> getActioninfo(int type) = 0;
    virtual void openAction(int actionkey, QString key, int type) = 0;
//    virtual bool isPreviewEnable(QString key, int type) = 0;
//    virtual QWidget *previewPage(QString key, int type, QWidget *parent = nullptr) = 0;
    virtual QWidget *detailPage(const ResultInfo &ri) = 0;

    void invokeActions(InvokableActions actions);
};
```

> 接口使用注意事项：
> 
> 接口实现时，需要继承`SearchPluginIface`，并设置以下接口信息：
> 
> Q_PLUGIN_METADATA(IID SearchPluginIface_iid FILE "common.json") Q_INTERFACES(Zeeker::SearchPluginIface)

其中 `common.json`为开发者自己编写的插件元数据文件，比如可以用来指定版本号等。

子类需要上面两个接口类的所有虚函数，其中，

```c++
virtual void KeywordSearch(QString keyword,DataQueue<ResultInfo> *searchResult) = 0;
```

函数会被UI直接调用，如果你的搜索功能十分费时，请在子线程里实现搜索，不要阻塞UI；

`ResultInfo`代表每一个结果项，`DataQueue`是前端取结果的数据队列。

```c++
virtual QList<Actioninfo> getActioninfo(int type) = 0;
```

这个函数用于获取每一项搜索结果可以执行的动作，比如打开等，`Actioninfo`中的`actionkey`用于指定特定的`action`，`type`用于指定搜索结果的类型(如果你的搜索结果有的话);

```c++
virtual QWidget *detailPage(const ResultInfo &ri) = 0;
```

这是用于获取每一项的详情页的函数，详情页同一时间只会显示一个，所以如果你的搜索结果详情页都是一致的风格，最好提前初始化，当这个方法被调用时只更新数据即可。

请一定要注意，搜索可能被快速触发，所以你需要确保当用户进行一次搜索时，队列里不会被错误的插入上一次的搜索结果。

## 联系我们
目前lingmo-search项目的负责人：iaom zhangpengfei@kylinos.cn
由于项目更新很快，所以以上内容请以代码为准。
