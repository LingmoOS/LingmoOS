#ifndef NETDETAILSWIDGET_H
#define NETDETAILSWIDGET_H

#include <QWidget>
#include <QHBoxLayout>

#include "../controls/kchartview.h"
#include "basedetailviewwidget.h"
#include "detaillisttable.h"
#include "netdetailsmodel.h"
#include "netviewscrollarea.h"

class NetDetailsWidget : public BaseDetailViewWidget
{
    Q_OBJECT
public:
    explicit NetDetailsWidget(QWidget *parent = nullptr);
    ~NetDetailsWidget();

private:
    void initUI();

private:
    NetViewScrollArea *mNetGraphScrollArea = nullptr;
    QHBoxLayout *mListHLayout = nullptr;

    DetailListTable *mDetailList = nullptr;
    NetDetailsModel *mDetailModel = nullptr;

    QList<ChartViewWidget*> mChartViewList;
    QString mCurrentNetName;

public slots:
    void onUpdateData(qreal data);

signals:

};

#endif // NETDETAILSWIDGET_H
