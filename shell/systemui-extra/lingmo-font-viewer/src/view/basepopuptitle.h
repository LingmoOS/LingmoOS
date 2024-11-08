#ifndef BASEPOPUPTITLE_H
#define BASEPOPUPTITLE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

/* 弹窗标题栏 */
class BasePopupTitle : public QWidget
{
    Q_OBJECT
public:
    /* 是否使用关闭按钮，标题名 */
    BasePopupTitle(bool isClose = true, QString value = tr("Font Viewer"));
    ~BasePopupTitle();

private:
    void init(void);
    void setWidgetStyle();

private:
    QString m_iconAppPath;
    QString m_iconClosePath;
    QString m_value;
    int m_space;
    bool m_isClose;

    QPushButton *m_iconBtn = nullptr;    /* 放置图标 */
    QLabel *m_valueLab = nullptr;        /* 放置值 */
    QPushButton *m_closeBtn = nullptr;

    QHBoxLayout *m_hlayout = nullptr;

private slots:
    void slotCloseWid(void);
    void slotChangePCModeSize();
    void slotChangePadModeSize();
};

#endif // BASEPOPUPTITLE_H
