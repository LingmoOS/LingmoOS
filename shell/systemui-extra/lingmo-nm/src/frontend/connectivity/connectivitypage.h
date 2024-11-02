#ifndef CONNECTIVITYPAGE_H
#define CONNECTIVITYPAGE_H

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QRadioButton>
#include <QDBusInterface>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>

#include "divider.h"
#include "lingmoconnectresource.h"

class ConnectivityPage : public QDialog
{
    Q_OBJECT
public:
    explicit ConnectivityPage(QString uri = "", QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event);

private:
    QScrollArea *m_scrollArea;
    QWidget* m_warningWidget;
    QWidget* m_centerWidget;
    QWidget* m_editWidget;
    QWidget* m_bottomWidget;

    QString m_uri;
    QPushButton *m_confirmBtn;
    QLabel *m_icon;
    QLabel *m_statusText;
    QLabel *m_text;
    QRadioButton *m_publicNetworkButton;
    QRadioButton *m_intranetButton;
    QLineEdit *m_uriEdit;
    QLabel* m_warningLabel;

    KyConnectResourse* m_connectResource;

    void initUi();
    void initConnect();

private Q_SLOTS:
    void setWarning(NetworkManager::Connectivity connectivity);
    void checkUri();

Q_SIGNALS:
    void pageClose();
};

#endif // CONNECTIVITYPAGE_H
