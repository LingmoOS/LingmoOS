#ifndef MEMDETAILSWIDGET_H
#define MEMDETAILSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>

#include "../controls/kchartview.h"
#include "basedetailviewwidget.h"
#include "detaillisttable.h"
#include "memdetailsmodel.h"

class MemDetailsWidget : public BaseDetailViewWidget
{
    Q_OBJECT
public:
    explicit MemDetailsWidget(QWidget *parent = nullptr);
    ~MemDetailsWidget();

private:
    void initUI();

private:
    QVBoxLayout *mMemVLayout = nullptr;
    QVBoxLayout *mSwapVLayout = nullptr;
    QHBoxLayout *mMemSwapHLayout = nullptr;
    QHBoxLayout *mListHLayout = nullptr;

    KChartView *mMemChartView = nullptr;
    KChartView *mSwapChartView = nullptr;

    DetailListTable *mDetailList = nullptr;
    MemDetailsModel *mDetailModel = nullptr;

public slots:
    void onUpdateMemData(qreal memUsed, qreal memTotal);
    void onUpdateSwapData(qreal swapUsed, qreal swapTotal);

signals:

};

#endif // MEMDETAILSWIDGET_H
