#ifndef POPUPREMOVE_H
#define POPUPREMOVE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "basepopuptitle.h"
#include "include/core.h"

/* 字体卸载弹窗 */
class PopupRemove: public QWidget
{
    Q_OBJECT
public:
    PopupRemove(QString path , QWidget *parent=nullptr);
    ~PopupRemove();
    void setWidgetUi();
    void changeFontSize();

private:
    BasePopupTitle *m_titleBar = nullptr;   /* 标题栏 */
    QLabel *m_tipText = nullptr;            /* 提示信息 */
    QPushButton *m_sureBtn = nullptr;       /* 确认按钮 */
    QPushButton *m_cancelBtn = nullptr;     /* 取消按钮 */

    QVBoxLayout *m_vMainLayout = nullptr;
    QHBoxLayout *m_hMainLayout = nullptr;
    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

    QString m_path;

signals:
    void sigSureClick(QString path);

public slots:
    void slotSureClick();
    void slotCancelClick();
};

#endif // POPUPREMOVE_H
