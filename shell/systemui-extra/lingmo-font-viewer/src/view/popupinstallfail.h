#ifndef POPUPINSTALLFAIL_H
#define POPUPINSTALLFAIL_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

#include "basepopuptitle.h"
#include "include/core.h"

/* 字体安装失败弹窗 */
class PopupInstallFail : public QWidget
{
    Q_OBJECT
public:
    PopupInstallFail(QStringList fontList , QWidget *parent = nullptr);
    ~PopupInstallFail();
    void setWidgetUi();
    void setWidLayout();
    void changeFontSize();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    BasePopupTitle *m_titleBar = nullptr;
    QPushButton *m_tipIcon = nullptr;
    QLabel *m_tipText = nullptr;
    QListWidget *m_fontListWid = nullptr;
    QPushButton *m_continueBtn = nullptr;

    QVBoxLayout *m_vLayout = nullptr;
    QVBoxLayout *m_vIconLayout = nullptr;
    QHBoxLayout *m_hTipLayout = nullptr;
    QHBoxLayout *m_hBtnLayout = nullptr;
    QVBoxLayout *m_vMainLayout = nullptr;
    QHBoxLayout *m_hMainLayout = nullptr;

    QStringList m_uninstallFont;
signals:
    void signalWidgetClose();
public slots:
    void slotContinueClick();
};

#endif // POPUPINSTALLFAIL_H
