// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H
#define APPLICATION_H

#define protected public
#include <DApplication>
#undef protected

#include <DGuiApplicationHelper>
#include <QStack>


#include "globaldefine.h"

class MainWindow;
class CColorPickWidget;
class TopTilte;
class DrawToolManager;
class PageScene;
class PageView;
class QMimeData;
class CShapeMimeData;
namespace DrawAttribution {
class CAttributeManagerWgt;
}

#if  defined(dApp)
#undef dApp
#endif
#define dApp (static_cast<DApplication *>(QCoreApplication::instance()))
#define drawApp (Application::drawApplication())
#define CHECK_MOSUEACTIVE_RETURN \
    if(dApp->mouseButtons() == Qt::LeftButton)\
    {\
        auto view1 = CManageViewSigleton::GetInstance()->getCurView();\
        if(view1 != nullptr && view1->viewport()->rect().contains(view1->viewport()->mapFromGlobal(QCursor::pos())))\
        {\
            return;\
        }\
    }
#define CHECK_CURRENTTOOL_RETURN(pView) \
    CHECK_MOSUEACTIVE_RETURN \
    auto view = CManageViewSigleton::GetInstance()->getCurView();\
    if(pView != view){return;}\
    if(pView == nullptr){return;}\
    int currentMode = view->page()->currentTool();\
    if (currentMode != selection) {\
        return;\
    }

DWIDGET_USE_NAMESPACE

class DrawBoard;
class Page;
class Application : public QObject
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    ~Application();

    /**
     * @brief drawApplication 返回顶层topToolbar
     * @return 返回画板程序的指针
     */
    static Application *drawApplication();


    DApplication *dApplication();


    int  execDraw(const QStringList &paths);

    /**
     * @brief topMainWindow 返回顶层mainwindow
     * @return
     */
    MainWindow *topMainWindow() const;
    QWidget *topMainWindowWidget() const;

    /**
     * @brief topToolbar 返回顶层topToolbar
     * @return 返回顶层工具块界面指针
     */
    TopTilte *topToolbar() const;

    /**
     * @brief leftToolBar 返回顶层leftToolBar
     * @return 返回左侧工具块界面指针
     */
    //DrawToolManager *leftToolBar() const;

    DrawBoard *drawBoard() const;

    //CColorPickWidget *colorPickWidget();

    /**
     * @brief attributionsWgt 返回属性界面
     */
    DrawAttribution::CAttributeManagerWgt *attributionsWgt();

    /**
     * @brief currentDrawScence 返回当前显示的画布场景
     * @return 返回当前显示的画布场景指针
     */
    PageScene *currentDrawScence();

    /**
     * @brief setCurrentTool 设置current视窗场景当前的工具
     */
    void setCurrentTool(int tool);

    int  currentTool();

    /**
     * @brief setViewCurrentTool 设置视窗场景当前的工具
     */
    //void  setPageTool(Page *page, EDrawToolMode tool);

    /**
     * @description: openFiles 通过路径打开图片或者ddf文件,新接口，统一使用这个
     * @param files 图片路径列表
     * @param bool  是否以当前图片大小设置scence的大小
     * @param bool  是否加入撤销返回栈
     * @param bool  是否在一个新的scence中进行导入,ddf文件会自动创建新的scence
     * @param appFirstExec　[bool] 程序是否通过打开文件的方式初次运行(这个时候如果打开失败那么在最后需要关闭程序)
    */
    void openFiles(QStringList files, bool asFirstPictureSize = false,
                   bool addUndoRedo = false, bool newScence = false,
                   bool appFirstExec = false);

    /**
     * @brief getRightFiles 根据输入返回所有合法正确的可加载文件(并且会弹窗提示)
     * @return 返回所有合法正确的可加载文件
     */
    //QStringList getRightFiles(const QStringList &files, bool notice = true);

    /**
     * @brief isFileNameLegal 判断文件是否合法(是否可加载)
     * @return 返回文件是否合法
     */
    bool isFileNameLegal(const QString &path, int *outErrorReson = nullptr);

    /**
     * @brief systemThemeColor 获取系统主题颜色
     * @return
     */
    QColor  systemThemeColor();

    /**
     * @brief isFileExist 文件是否存在，如果是资源型路径那么会将其替换成本地路径
     */
    bool isFileExist(QString &filePath);

    /**
     * @brief setWidgetAllPosterityNoFocus 将widget的后代都设置为没有焦点
     */
    static void  setWidgetAllPosterityNoFocus(QWidget *pW);

    /**
     * @brief setWidgetAccesibleName 设置widget对象外界可访问的名字属性
     */
    static void setWidgetAccesibleName(QWidget *w, const QString &name);

    static bool isWaylandPlatform();

    static bool isTabletSystemEnvir();

    int execPicturesLimit(int count);

    /**
     * @brief 当程序退出时会调用
     */
    void onAppQuit();

    /**
     * @brief maxPicSize 获取最大支持的图片尺寸
     * @return 最大支持的图片尺寸
     */
    QSize maxPicSize();

    QStringList readableFormats();
    QStringList writableFormats();
    QStringList readableFormatNameFilters();
    QStringList writableFormatNameFilters();

    /**
         * @brief defaultFileDialogPath 文件对话框的默认路径
         */
    QString defaultFileDialogPath() const;

    /**
     * @brief setDefaultFileDialogPath 设置文件对话框的默认路径
     */
    void  setDefaultFileDialogPath(const QString &defaultPath);

    /**
     * @brief defaultFileDialogNameFilter 文件对话框的默认选中过滤信息
     */
    QString defaultFileDialogNameFilter() const;

    /**
     * @brief setDefaultFileDialogNameFilter 设置文件对话框的默认选中过滤信息
     */
    void setDefaultFileDialogNameFilter(const QString &nameFilter);
    void readSettings();
    void saveSettings();
private:
//    enum EFileClassEnum {ENotFile    = 0,
//                         ENotExist,
//                         EDrawAppNotSup,
//                         EDrawAppSup,
//                         EDrawAppSupAndReadable,
//                         EDrawAppSupButNotReadable
//                        };
    //typedef QMap<EFileClassEnum, QStringList> QFileClassedMap;
    //QStringList doFileClassification(const QStringList &inFilesPath, QFileClassedMap &out);

signals:
    void quitRequest();

public slots:
    void onThemChanged(DGuiApplicationHelper::ColorType themeType);
    void activateWindow();
    void showMainWindow(const QStringList &paths);

    void waitShowThenLoad(const QStringList &paths);
//    void noticeFileRightProblem(const QStringList &problemfile,
//                                Application::EFileClassEnum classTp = EDrawAppNotSup,
//                                bool checkQuit = true);

    void     onAttributionChanged(int attris, const QVariant &var,
                                  int phase, bool autoCmdStack);

    void     onFocusChanged(QWidget *old, QWidget *now);

    void     quitApp();


    QVariant defaultAttriVar(void *sceneKey, int attris);
    QVariant currenDefaultAttriVar(int attris);

public:
    enum   EMessageType {ENormalMsg, EWarningMsg, EQuestionMsg};
//    Q_SLOT int exeMessage(const QString &message, EMessageType msgTp = EWarningMsg,
//                          bool autoFitDialogWidth = true,
//                          const QStringList &moreBtns = QStringList() << tr("OK"),
//                          const QList<int> &btnType = QList<int>() << 0);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void loadTools();
    void loadPluginTools();

private:
    MainWindow *actWin = nullptr;

    DApplication *_dApp = nullptr;

    int  _touchEnchValue = 7;

    static Application *s_drawApp;

    QStack<QCursor> _cursorStack;

    QMap<void *, QMap<int, QVariant>> _defaultAttriVars;

    QStringList supReadFormats;
    QStringList supReadFormatsFilter;
    QStringList supWriteFormatFilters;
    QStringList supWriteFormats;

    QString _defaultFileDialogPath;
    QString _defaultFileDialogNameFilter;
};
#endif // APPLICATION_H
