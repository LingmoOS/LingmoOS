#ifndef CPUHISTORYWIDGET_H
#define CPUHISTORYWIDGET_H

#include <QWidget>
#include <qgsettings.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColor>
#include <QProgressBar>

#include "../controls/kchartview.h"
#include "../controls/klabel.h"

class CpuHistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CpuHistoryWidget(QWidget *parent = nullptr);
    virtual ~CpuHistoryWidget();

    QString getCpuOccupancy();

public slots:
    void onUpdateData(qreal cpuPercent);
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

private:
    KChartView *m_chartView = nullptr;
    QProgressBar *m_progressBar = nullptr;
    KLabel *m_labelTitle = nullptr;
    KLabel *m_labelValue = nullptr;

    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QHBoxLayout *m_progLayout = nullptr;
    QHBoxLayout *m_chartLayout = nullptr;

    QColor m_colorBg;
    QGSettings *m_styleSettings = nullptr;
    float m_curFontSize;
};

#endif // CPUHISTORYWIDGET_H
