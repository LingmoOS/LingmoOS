// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOW_H
#define WINDOW_H

//#include "../controls/toolbar.h"
#include "../common/utils.h"
#include "../startmanager.h"
#include "../common/performancemonitor.h"
#include "../editor/editwrapper.h"
#include "../controls/findbar.h"
#include "../controls/jumplinebar.h"
#include "../controls/replacebar.h"
#include "../common/settings.h"
#include "../controls/tabbar.h"
#include "../thememodule/themepanel.h"
#include "../common/performancemonitor.h"
#include "../common/dbusinterface.h"
#include "../common/iflytek_ai_assistant.h"
#include "../common/CSyntaxHighlighter.h"
#include <DMainWindow>
#include <DStackedWidget>
#include <qprintpreviewdialog.h>
#include <dprintpreviewdialog.h>

DWIDGET_USE_NAMESPACE

class Window : public DMainWindow
{
    Q_OBJECT

public:
    // 打印文档信息，用于超大文档的打印
    struct PrintInfo {
        QTextDocument       *doc = nullptr;         // 打印的文本
        CSyntaxHighlighter  *highlighter = nullptr; // 高亮处理
    };

    explicit Window(DMainWindow *parent = nullptr);
    ~Window() override;

    /**
     * @brief 加载语音助手配置文件
     */
    void loadIflytekaiassistantConfig();
    /**
     * @brief 获取语音助手配置文件数据
     * @param mode_name　接口模块名称
     */
    bool getIflytekaiassistantConfig(const QString &mode_name);
    //跟新文件修改状态
    void updateModifyStatus(const QString &path, bool isModified);
    //跟新tab文件名称
    void updateSaveAsFileName(QString strOldFilePath, QString strNewFilePath);
    void updateSabeAsFileNameTemp(QString strOldFilePath, QString strNewFilePath);

    void showCenterWindow(bool bIsCenter);
    void initTitlebar();
    bool checkBlockShutdown();

    int getTabIndex(const QString &file);
    void activeTab(int index);

    Tabbar *getTabbar();

    void addTab(const QString &filepath, bool activeTab = false);
    void addTabWithWrapper(EditWrapper *wrapper, const QString &filepath, const QString &qstrTruePath,
                           const QString &tabName, int index = -1);
    bool closeTab();
    bool closeTab(const QString &fileName);
    void restoreTab();
    void clearBlack();

    EditWrapper *createEditor();
    EditWrapper *currentWrapper();
    EditWrapper *wrapper(const QString &filePath);
    TextEdit *getTextEditor(const QString &filepath);
    void focusActiveEditor();
    void removeWrapper(const QString &filePath, bool isDelete = false);

    void openFile();
    bool saveFile();
    bool saveAsFile();
    QString saveAsFileToDisk();
    QString saveBlankFileToDisk();
    bool saveAsOtherTabFile(EditWrapper *wrapper);

    void changeSettingDialogComboxFontNumber(int fontNumber);
    void decrementFontSize();
    void incrementFontSize();
    void resetFontSize();
    void setFontSizeWithConfig(EditWrapper *editor);

    void popupFindBar();
    void popupReplaceBar();
    void popupJumpLineBar();
    void updateJumpLineBar(TextEdit *editor);
    void popupSettingsDialog();
    void popupPrintDialog();
    void popupThemePanel();

    void toggleFullscreen();

    void remberPositionSave();
    void remberPositionRestore();

    void displayShortcuts();
    // 文本打印
    void doPrint(DPrinter *printer, const QVector<int> &pageRange);
    // 大文本打印
    void doPrintWithLargeDoc(DPrinter *printer, const QVector<int> &pageRange);
    // 翻页预览打印，无需重新布局
    void asynPrint(QPainter &p, DPrinter *printer, const QVector<int> &pageRange);

    // 备份文件
    void backupFile();
    // 关闭当前窗口所有文件
    bool closeAllFiles();
    // 恢复备份文件标签页
    void addTemFileTab(const QString &qstrPath, const QString &qstrName, const QString &qstrTruePath, const QString &lastModifiedTime, bool bIsTemFile = false);

    QMap<QString, EditWrapper *> getWrappers();

    //设置显示清除焦点
    void setChildrenFocus(bool ok);

    bool replaceBarIsVisiable();
    bool findBarIsVisiable();
    QString getKeywordForSearchAll();
    QString getKeywordForSearch();
    void setPrintEnabled(bool enabled);
    QStackedWidget *getStackedWgt();

    static void printPage(int index, QPainter *painter, const QTextDocument *doc,
                          const QRectF &body, const QRectF &pageCountBox);

Q_SIGNALS:
    void themeChanged(const QString &themeName);
    void requestDragEnterEvent(QDragEnterEvent *);
    void requestDropEvent(QDropEvent *);
    void newWindow();
    void closeWindow();
    void sigJudgeBlockShutdown();
    void pressEsc();
    // 标签页出现变更信号，文件添加或删除时触发
    void tabChanged();

public Q_SLOTS:
    void addBlankTab();
    void addBlankTab(const QString &blankFile);
    void handleTabCloseRequested(int index);
    void handleTabsClosed(QStringList tabList);
    void handleCurrentChanged(const int &index);

    void handleJumpLineBarExit();
    void handleJumpLineBarJumpToLine(const QString &filepath, int line, bool focusEditor);

    void handleBackToPosition(const QString &file, int row, int column, int scrollOffset);

    void handleFindNextSearchKeyword(const QString &keyword);
    void handleFindPrevSearchKeyword(const QString &keyword);
    /**
     * @brief handleFindKeyword
     * @param keyword
     * @author ut002764 lxp 2021.4.27
     */
    void handleFindKeyword(const QString &keyword, bool state);

    void slotFindbarClose();
    void slotReplacebarClose();

    void handleReplaceAll(const QString &replaceText, const QString &withText);
    void handleReplaceNext(const QString &file, const QString &replaceText, const QString &withText);
    void handleReplaceRest(const QString &replaceText, const QString &withText);
    void handleReplaceSkip(QString file, QString keyword);

    void handleRemoveSearchKeyword();
    void handleUpdateSearchKeyword(QWidget *widget, const QString &file, const QString &keyword);

    void loadTheme(const QString &path);

    void showNewEditor(EditWrapper *wrapper);
    void showNotify(const QString &message);
    int getBlankFileIndex();

    DDialog *createDialog(const QString &title, const QString &content);

    void slotLoadContentTheme(DGuiApplicationHelper::ColorType themeType);
    void slotSettingResetTheme(const QString &path);

    void slot_saveReadingPath();
    void slot_beforeReplace(QString _);
    void slot_setTitleFocus();
    //清除不支持双字节字符集符号
    void slotClearDoubleCharaterEncode();
    void slotSigThemeChanged(const QString &path);
    void slotSigAdjustFont(QString fontName);
    void slotSigAdjustFontSize(qreal fontSize);
    void slotSigAdjustTabSpaceNumber(int number);
    void slotSigAdjustWordWrap(bool enable);
    void slotSigSetLineNumberShow(bool bIsShow);
    void slotSigAdjustBookmark(bool bIsShow);
    void slotSigShowBlankCharacter(bool bIsShow);
    void slotSigHightLightCurrentLine(bool bIsShow);
    void slotSigShowCodeFlodFlag(bool bIsShow);
    void slotSigChangeWindowSize(QString mode);

private:
    void handleFocusWindowChanged(QWindow *w);
    void updateThemePanelGeomerty();
    void checkTabbarForReload();
    void clearPrintTextDocument();
    void setWindowTitleInfo();
    // 取得当前文档打开路径（新建文档为"系统-文档"目录）
    QString getCurrentOpenFilePath();
    // 计算字体大小的缩放比例
    qreal calcFontScale(qreal fontSize);
    // 从字体缩放比例推算字体大小
    qreal calcFontSizeFromScale(qreal fontScale);

    // 克隆文本数据
    bool cloneLargeDocument(EditWrapper *editWrapper);
    // 打印多文本数据
    void printPageWithMultiDoc(int index, QPainter *painter, const QVector<PrintInfo> &printInfo,
                               const QRectF &body, const QRectF &pageCountBox);
    // 重新更新文本
    void rehighlightPrintDoc(QTextDocument *doc, CSyntaxHighlighter *highlighter);

    // 接收布局模式变更信号，更新界面布局
    Q_SLOT void updateSizeMode();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *keyEvent) override;
    void keyReleaseEvent(QKeyEvent *keyEvent) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *event) override;
    // 处理延迟处理等定时器事件
    void timerEvent(QTimerEvent *e) override;

private:
    DBusDaemon::dbus *m_rootSaveDBus {nullptr};

    QWidget *m_centralWidget {nullptr};
    DStackedWidget *m_editorWidget {nullptr};
    QVBoxLayout *m_centralLayout {nullptr};
    Tabbar *m_tabbar {nullptr};

    JumpLineBar *m_jumpLineBar {nullptr};
    ReplaceBar *m_replaceBar {nullptr};
    ThemePanel *m_themePanel {nullptr};
    FindBar *m_findBar {nullptr};
    Settings *m_settings {nullptr};

    QMap<QString, EditWrapper *> m_wrappers;

    DMenu *m_menu {nullptr};

    QStringList m_closeFileHistory;

    QString m_remberPositionFilePath;
    int m_remberPositionRow;
    int m_remberPositionColumn;
    int m_remberPositionScrollOffset;

    QString m_blankFileDir;
    QString m_backupDir;
    QString m_autoBackupDir;
    qreal m_fontSize = 0;

    QString m_titlebarStyleSheet;

    QString m_keywordForSearch;
    QString m_keywordForSearchAll;

    QString m_themePath;
    QString m_tabbarActiveColor;
    QList <TextEdit *> m_reading_list;
    QStringList m_qlistTemFile;///<临时文件列表

    QProcess *m_shortcutViewProcess = nullptr;
    bool m_needMoveToCenter = false;
    DPrintPreviewDialog *m_pPreview {nullptr};

    //打印文本
    QTextDocument *m_printDoc = nullptr;            // 打印文档，用于普通文档打印
    QPageLayout m_lastLayout;                       // 打印布局
    bool m_isNewPrint = false;                      // 判断是否是新的打印文档
    bool m_bLargePrint = false;                     // 是否为大文件打印
    bool m_bPrintProcessing = false;                // 文件打印计算中
    EditWrapper *m_printWrapper = nullptr;          // 当前处理的编辑对象(关闭标签页时需要退出打印)
    QTextDocument *printCopyDoc = nullptr;              // 用于大文本打印时的拷贝文档
    CSyntaxHighlighter *printCopyHighlighter = nullptr; // 用于大文本打印时的高亮文档
    QVector<PrintInfo> m_printDocList;              // 打印文档列表，用于超大文档打印
    int m_multiDocPageCount = 0;                    // 用于超大文档打印时单独记录多文档的打印页数

    QBasicTimer m_delayCloseTabTimer;               // 延迟关闭标签页定时器，防止异常情况多次触发关闭同一标签页的情况
    int m_requestCloseTabIndex = 0;                 // 请求关闭的标签页索引

    //语音助手服务是否被注册
    bool m_bIsRegistIflytekAiassistant {false};
    //记录语音助手接口配置状态
    QMap<QString, bool> m_IflytekAiassistantState;
};

#endif
