#ifndef POPUPTIPS_H
#define POPUPTIPS_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>

#include "basepopuptitle.h"
#include "include/core.h"

/* 字体提示弹窗 */
class PopupTips: public QWidget
{
    Q_OBJECT
public:
    PopupTips(QWidget *parent = nullptr);
    ~PopupTips();
    void setWidgetUi();
    void changeFontSize();

private:
    BasePopupTitle *m_titleBar = nullptr;   /* 标题栏 */
    QLabel *m_tipText = nullptr;            /* 提示信息 */
    QPushButton *m_sureBtn = nullptr;       /* 确认按钮 */
    QCheckBox *m_checkBox = nullptr;        /* 复选框 */
    QVBoxLayout *m_vMainLayout = nullptr;
    QHBoxLayout *m_hMainLayout = nullptr;
    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

    QString m_tips;

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void signalWidgetClose();

public slots:
    void slotSureClick();
};

#endif 
