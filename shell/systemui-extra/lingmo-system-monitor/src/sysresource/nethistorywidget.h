#ifndef NETHISTORYWIDGET_H
#define NETHISTORYWIDGET_H

#include <QWidget>
#include <qgsettings.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>

#include "../controls/kchartview.h"
#include "../controls/kprogressbar.h"
#include "../controls/klabel.h"

class NetHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NetHistoryWidget(QWidget *parent = nullptr);
    virtual ~NetHistoryWidget();

    QString getNetSendSpeed();
    QString getNetLoadSpeed();

public slots:
    void onUpdateData(qreal sendTotalBytes, qreal sendRate, qreal recvTotalBytes, qreal recvRate);
signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnections();
    void initStyleTheme();
    void onThemeFontChange(float fFontSize);
    void onThemeStyleChange(QString strStyleName);
    qreal chkNewMaxData(qreal qNewMaxData);

private:
    KChartView *m_chartView = nullptr;
    KLabel *m_labelTitle = nullptr;
    QLabel *m_iconSend = nullptr;
    QLabel *m_iconRecv = nullptr;
    KLabel *m_labelSendTitle = nullptr;
    KLabel *m_labelSendValue = nullptr;
    KLabel *m_labelRecvTitle = nullptr;
    KLabel *m_labelRecvValue = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_topTitleLayout = nullptr;
    QVBoxLayout *m_midContentLayout = nullptr;
    QHBoxLayout *m_sendLayout = nullptr;
    QHBoxLayout *m_recvLayout = nullptr;
    QHBoxLayout *m_chartLayout = nullptr;

    QColor m_colorBg;
    QGSettings *m_styleSettings = nullptr;
    float m_curFontSize;
};

#endif // NETHISTORYWIDGET_H
