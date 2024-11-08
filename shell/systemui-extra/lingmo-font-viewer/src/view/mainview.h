#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QWidget>
#include <QHBoxLayout>

#include "functionwid.h"
#include "fontwid.h"
#include "globalsizedata.h"

class MainView : public QWidget
{
    Q_OBJECT
public:
    ~MainView();
    static MainView *getInstance();

    void setWidgetUi();          /* 初始化组件 */
    void initLayout();           /* 初始化布局 */
    void setSlotConnet();        /* 信号和槽 */
    void pullUpWindow();         /* 最小化状态下拉起主界面 */

    void initGsetting(void);     /* 监听主题变化 */
    void changeStyle(void);      /* 切换深浅主题 */

    void getFontFileFromClick(QString fileName);
    void getFontFileFromClickAferOpen(QString fileName);

    quint32 m_windowId = 0;      /* wayland 的 窗口ID */
    FontWidget *m_fontWid = nullptr;       /* 右侧显示界面 */

protected:
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *watch, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;   /* 键盘响应事件 */
    void changeEvent(QEvent *event) override;        /* 窗体改变 */

private:
    MainView(QWidget *parent = nullptr);

private:
    FunctionWid *m_functionWid = nullptr;  /* 左侧功能界面 */
    //FontWidget *m_fontWid = nullptr;       /* 右侧显示界面 */

    QHBoxLayout *m_hLayout = nullptr;

signals:
    void sigWindowState(QString state);

private slots:


};

#endif // MAINVIEW_H
