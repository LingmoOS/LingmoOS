#ifndef POPUPMESSAGE_H
#define POPUPMESSAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>

#include "basepopuptitle.h"
#include "include/core.h"

/* 字体提示弹窗 */
class PopupMessage: public QWidget
{
    Q_OBJECT
public:
    PopupMessage(QWidget *parent = nullptr);
    ~PopupMessage();
    void setWidgetUi();
    void changeFontSize();

private:
    BasePopupTitle *m_titleBar = nullptr;   /* 标题栏 */
    QLabel *m_tipText = nullptr;            /* 提示信息 */
    QPushButton *m_yesBtn = nullptr;        /* 确认按钮 */
    QPushButton *m_noBtn = nullptr;         /* 确认按钮 */
    QVBoxLayout *m_vMainLayout = nullptr;
    QHBoxLayout *m_hMainLayout = nullptr;
    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

    QString m_tips;

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void signalWidgetClose();
    void signalClickYes();
    void signalClickNo();

public slots:
    void slotSureClick();
    void slotNoClick();
};

#endif 
