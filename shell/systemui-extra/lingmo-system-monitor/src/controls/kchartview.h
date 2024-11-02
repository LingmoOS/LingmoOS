#ifndef __KCHARTVIEW_H__
#define __KCHARTVIEW_H__

#include <QWidget>
#include <QList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE

#define TEMP_CHARTSDATA_MAX     50

typedef struct KChartAttr_s
{
    QColor colorForeground;
    qreal colorAlpha;
    QList<qreal> listData;
}KChartAttr;

class KChartView : public QWidget
{
    Q_OBJECT
public:
    explicit KChartView(QWidget *parent = nullptr);
    virtual ~KChartView();

    qreal getCurData(int nItemIndex = 0);
    void addItemAttr(KChartAttr chartAttr);
    qreal getRealMaxData(qreal newData, int nItemIndex = 0);
    void setMaxData(qreal qMaxData);
    void setAxisTitle(QString axisRTitle, QColor color = "", QString axisLTitle = "");
    void updateAxisRTitle(QString title);

public slots:
    void onUpdateData(qreal fData, int nItemIndex = 0);

signals:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void drawAxisText(QPainter *painter);

private:
    QChart *m_chart = nullptr;
    QChartView *m_chartView = nullptr;
    QList<QLineSeries*> m_listUpLineSeries;
    QList<QLineSeries*> m_listLowLineSeries;
    QList<QAreaSeries*> m_listAreaSeries;
    QValueAxis *m_valueAxisX = nullptr;
    QValueAxis *m_valueAxisY = nullptr;

    QColor m_colorBgLine;
    QColor m_RadioColor;
    qreal m_dataMax;
    QList<KChartAttr> m_listChartAttr;
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_chartLayout = nullptr;
    QString m_axisRTitle = "";
    QString m_axisLTitle;
};

#endif // __KCHARTVIEW_H__
