#ifndef NETWORKDISCONNECTIONWIDGET_H
#define NETWORKDISCONNECTIONWIDGET_H

#include <QFrame>
class NetworkDisconnectionWidget : public QFrame
{
    Q_OBJECT
public:
    NetworkDisconnectionWidget(QWidget *parent = nullptr);
    ~NetworkDisconnectionWidget();

public slots:
    void backPage();
    void retryPage();
    void themeChanged(int theme);

private:
    void initUI();
};

#endif   // NETWORKDISCONNECTIONWIDGET_H
