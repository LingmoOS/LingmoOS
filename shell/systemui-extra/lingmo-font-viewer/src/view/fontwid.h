#ifndef FONTWIDGET_H
#define FONTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QStackedWidget>

#include "titlebar.h"
#include "basepreviewarea.h"
#include "fontlistview.h"
#include "blankpage.h"

class FontWidget : public QWidget
{
    Q_OBJECT

public:
    FontWidget(QWidget *parent = nullptr);
    ~FontWidget();

    void setWidgetUi();      /* 初始化组件 */
    void initLayout();       /* 设置组件样式 */
    void setSlotConnet();
    bool isFontFile(QString path);       /* 判断文件类型 */
    bool eventFilter(QObject *watched, QEvent *event) override;
    void useFontFileOpen(QString fontFile);
    bool getFontFile(QString fileName);
    void cilckFontFile(QString fontFile);
    TitleBar *m_titleBar = nullptr;


protected:
    void paintEvent(QPaintEvent *event) override;           /* 绘制事件 */
    void dragEnterEvent(QDragEnterEvent *event) override;   /* 鼠标拖拽事件 */
    void dropEvent(QDropEvent *event) override;             /* 鼠标放下事件 */
    void keyPressEvent(QKeyEvent *event) override;             /* 键盘响应事件 */


private:
    //TitleBar *m_titleBar = nullptr;
    FontListView *m_fontView = nullptr;
    BasePreviewArea *m_previewArea = nullptr;
    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

    //堆栈窗体
    QStackedWidget *m_stackedWid;
    BlankPage *m_blankPage = nullptr;
    BlankPage *m_searchBlankPage = nullptr;

    int m_fontSize;

signals:
    void sigWindowState(QString state);

public slots:
    void slotChangeFilterType(FontType type);
    void slotWindowState(QString state);
    void slotShowBlankPage();
    void slotShowFontPage();
    void slotShowSearchBlankPage();
};
#endif // FONTWIDGET_H
