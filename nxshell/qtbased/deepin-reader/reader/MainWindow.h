// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MainWindow_H
#define MainWindow_H

#include <DMainWindow>

class Central;
class DocSheet;
class TitleMenu;
class QPropertyAnimation;
class MainWindow : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)
    Q_PROPERTY(QList<QWidget *> orderWidgets WRITE updateOrderWidgets)

public:
    /**
     * @brief MainWindow
     * 根据文件路径列表会直接打开对应文件
     * @param filePathList 文件路径列表
     * @param parent
     */
    explicit MainWindow(QStringList filePathList, DMainWindow *parent = nullptr);

    /**
     * @brief MainWindow
     * 直接显示传入已经存在的sheet
     * @param sheet
     * @param parent
     */
    explicit MainWindow(DocSheet *sheet, DMainWindow *parent = nullptr);

    ~MainWindow() override;

    /**
     * @brief addSheet
     * 添加新文档窗口
     * @param sheet
     */
    void addSheet(DocSheet *sheet);

    /**
     * @brief hasSheet
     * 是否有这个文档
     * @param sheet
     * @return
     */
    bool hasSheet(DocSheet *sheet) ;

    /**
     * @brief activateSheet
     * 活动文档显示到最前面
     * @param sheet 哪个文档
     */
    void activateSheet(DocSheet *sheet);

    /**
     * @brief setDocTabBarWidget
     * 全屏时设置tabbar
     * @param widget tabbar
     */
    void setDocTabBarWidget(QWidget *widget);

    /**
     * @brief 调整标题控件大小
     */
    void resizeFullTitleWidget();

    /**
     * @brief 更新TAB控件顺序
     * @param orderlst
     */
    void updateOrderWidgets(const QList<QWidget *> &orderlst);

    /**
     * @brief handleClose
     * 进行关闭，成功会关闭并释放自己
     * @param needToBeSaved
     * @return
     */
    bool handleClose(bool needToBeSaved);

    /**
     * @brief 当前窗口打开文件
     * @param filePath 文件路径
     */
    void addFile(const QString &filePath);

    /**
     * @brief 全屏处理
     */
    void handleMainWindowFull();

    /**
     * @brief 退出全屏处理
     */
    void handleMainWindowExitFull();

    /**
     * @brief setTitleBarFocusEnable 统一启用或禁用titlebar和其子控件的焦点逻辑
     */
    void setTitleBarFocusEnable(bool enable);

protected:
    void closeEvent(QCloseEvent *event) override;

    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    /**
     * @brief initBase
     * 初始化基础项
     */
    void initBase();

    /**
     * @brief initUI
     * 初始化UI
     */
    void initUI();

    /**
     * @brief showDefaultSize
     * 显示默认大小
     */
    void showDefaultSize();

    /**
     * @brief initDynamicLibPath 加载动态库
     */
    void initDynamicLibPath();

    /**
     * @brief libPath 动态库路径
     * @param strlib: 动态库名称
     */
    QString libPath(const QString &strlib);

private slots:
    /**
     * @brief onDelayInit
     * 延时初始化
     */
    void onDelayInit();

    /**
     * @brief onUpdateTitleLabelRect
     * 更新提示标签大小
     */
    void onUpdateTitleLabelRect();

    /**
     * @brief 标题悬浮动画结束
     */
    void onTitleAniFinished();

private:
    QWidget *m_FullTitleWidget = nullptr;
    QWidget *m_docTabWidget = nullptr;
    QPropertyAnimation *m_TitleAnimation = nullptr;
    TitleMenu *m_menu = nullptr;
    Central *m_central = nullptr;
    bool needToBeSaved = true;
    int m_lastWindowState = Qt::WindowNoState;
    QStringList m_initFilePathList;

public:
    /**
     * @brief windowContainSheet 返回包含指定sheet的mainwindow
     * @param sheet
     * @return
     */
    static MainWindow *windowContainSheet(DocSheet *sheet);
    /**
     * @brief allowCreateWindow 是否允许生成新窗口
     * @return
     */
    static bool allowCreateWindow();
    /**
     * @brief activateSheetIfExist 激活已打开的文档窗口
     * @param filePath 文档路径
     * @return
     */
    static bool activateSheetIfExist(const QString &filePath);
    static MainWindow *createWindow(QStringList filePathList = QStringList());
    static MainWindow *createWindow(DocSheet *sheet);
    static QList<MainWindow *> m_list;
    QTimer *m_showMenuTimer = nullptr;
};

#endif // MainWindow_H
