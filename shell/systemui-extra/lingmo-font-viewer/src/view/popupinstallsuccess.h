#ifndef POPUPINSTALLSUCCESS_H
#define POPUPINSTALLSUCCESS_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "basepopuptitle.h"
#include "include/core.h"

/* 字体安装成功弹窗 */
class PopupInstallSuccess: public QWidget
{
    Q_OBJECT
public:
    PopupInstallSuccess(int num , QWidget *parent = nullptr);
    ~PopupInstallSuccess();
    void setWidgetUi();
    void changeFontSize();

private:
    QLabel *m_tipText = nullptr;         /* 提示框内的信息 */
    QPushButton *m_tipIcon = nullptr;    /* 提示图标 */

    QVBoxLayout *m_vLayout = nullptr;  /* 提示弹窗纵向布局 */
    QHBoxLayout *m_hLayout = nullptr;  /* 提示弹窗横向布局 */

    int m_num = 0;
signals:

public slots:
    void slotTimeDestroy();   /* 销毁安装成功弹窗 */

};

#endif // POPUPINSTALLSUCCESS_H
