// SPDX-FileCopyrightText: 2017 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORBUFFER_H
#define EDITORBUFFER_H

#include "../editor/dtextedit.h"
#include "../widgets/bottombar.h"
#include "../controls/warningnotices.h"
#include "../editor/leftareaoftextedit.h"
#include "../common/CSyntaxHighlighter.h"
#include "../common/utils.h"
#include <QVBoxLayout>
#include <QWidget>
#include <DMessageManager>
#include <DFloatingMessage>
#include <QByteArray>
#include <QTextCodec>
#include <DDialog>
#include <DMessageBox>
#include <DFileDialog>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>

class Window;
class EditWrapper : public QWidget
{
    Q_OBJECT

public:
    struct FileStateItem {
        QDateTime modified;
        QFile::Permissions permissions;
    };

    EditWrapper(Window *window = nullptr, QWidget *parent = nullptr);
    ~EditWrapper();

    //清除焦点　梁卫东　２０２０－０９－１４　１１：００：５０
    void clearAllFocus();
    void setQuitFlag();
    bool isQuit();
    bool getFileLoading();

    /**
     * @brief openFile 打开文件
     * @param filepath　打开文件路径
     * @param qstrTruePath　真实文件路径
     * @param bIsTemFile　修改状态
     */
    void openFile(const QString &filepath, QString qstrTruePath, bool bIsTemFile = false);
    // 以编码 encode 重新读取文件
    bool readFile(QByteArray encode = "");
    // 按编码 encode 保存文件
    bool saveFile(QByteArray encode = "");
    /**
     * @brief getPlainTextContent 获取文本框里的文本内容
     * @param plainTextConteng 存放获取到的内容
     */
    void getPlainTextContent(QByteArray &plainTextContent);
    //重新加载文件编码
    bool saveAsFile(const QString &newFilePath, const QByteArray &encodeName);
    //保存草稿文件
    bool saveDraftFile(QString &newFilePath);
    //另存为第一次打开文件编码文件
    bool saveAsFile();
    //重新加载文件编码 1.文件修改 2.文件未修改处理逻辑一样 切换编码重新加载和另存为 梁卫东
    bool reloadFileEncode(QByteArray encode);
    // 重新加载文件高亮类型
    void reloadFileHighlight(QString definitionName);
    //重写加载修改文件
    void reloadModifyFile();
    //获取文件编码
    QString getTextEncode();

    // 保存备份文件
    bool saveTemFile(QString qstrDir);
    //更新路径
    void updatePath(const QString &file, QString qstrTruePath = QString());
    //判断是否修改
    bool isModified();
    //判断是否草稿文件
    bool isDraftFile();
    //判断是否为备份文件
    bool isBackupFile();
    //判断内容是否为空
    bool isPlainTextEmpty();

    bool isTemFile();

    void hideWarningNotices();
    void checkForReload();
    void initToastPosition();
    void showNotify(const QString &message);
    bool getTextChangeFlag();
    void setTextChangeFlag(bool bFlag);
    void setLineNumberShow(bool bIsShow, bool bIsFirstShow = false);
    void setShowBlankCharacter(bool ok);
    void handleCursorModeChanged(TextEdit::CursorMode mode);
    void clearDoubleCharaterEncode();
    //
    BottomBar *bottomBar();
    QString filePath();
    TextEdit *textEditor();
    Window *window();
    void updateHighlighterAll();

    //get and set m_tModifiedDateTime
    QDateTime getLastModifiedTime() const;
    void setLastModifiedTime(const QString &time);

    void updateModifyStatus(bool isModified);
    void updateSaveAsFileName(QString strOldFilePath, QString strNewFilePath);

    // 取得当前编辑器使用的高亮处理(用于打印高亮)
    inline CSyntaxHighlighter *getSyntaxHighlighter() const
    { return m_pSyntaxHighlighter; }

signals:
    void sigClearDoubleCharaterEncode();

protected:
    // 处理文件加载事件
    virtual void customEvent(QEvent *e) override;

private:
    // 类似setPlainText(QString) 接口支持大文本加载 不卡顿 秒退出 梁卫东 2020年11月11日16:56:27
    void loadContent(const QByteArray &);
    void handleHightlightChanged(const QString &name);
    int GetCorrectUnicode1(const QByteArray &ba);
    // 文件加载时重新初始化部分设置
    void reinitOnFileLoad(const QByteArray &encode);

public slots:
    // 处理文档预加载数据
    void handleFilePreProcess(const QByteArray &encode, const QByteArray &content);
    void handleFileLoadFinished(const QByteArray &encode, const QByteArray &content, bool error);
    void OnThemeChangeSlot(QString theme);
    void UpdateBottomBarWordCnt(int cnt);
    void OnUpdateHighlighter();
    //set the value of m_bIsTemFile
    void setTemFile(bool value);

private:
    //第一次打开文件编码
    QString m_sFirstEncode = QString("UTF-8");
    //当前切换文件编码
    QString  m_sCurEncode = QString("UTF-8");

    //左边栏　标记　行号　折叠三合一控件
    LeftAreaTextEdit *m_pLeftAreaTextEdit = nullptr;
    //
    Window *m_pWindow = nullptr;
    //
    TextEdit *m_pTextEdit = nullptr;
    //
    BottomBar *m_pBottomBar = nullptr;
    //
    WarningNotices *m_pWaringNotices = nullptr;

    QDateTime m_tModifiedDateTime;
    //退出
    bool m_bQuit = false;
    //文件是否加载
    bool m_bFileLoading = false;
    bool m_bIsTemFile = false;
    //撤销重做栈操作任务文件修改
    bool m_bUndoRedoOption = false;
    //语法高亮
    KSyntaxHighlighting::Repository m_Repository;
    KSyntaxHighlighting::Definition m_Definition;
    //KSyntaxHighlighting::SyntaxHighlighter *m_pSyntaxHighlighter = nullptr;
    CSyntaxHighlighter *m_pSyntaxHighlighter = nullptr;
    bool m_bHighlighterAll = false;

    bool m_bAsyncReadFileFinished = false;
    bool m_bHasPreProcess = false;               // 预处理标识
};

#endif
