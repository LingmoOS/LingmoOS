#include "cpudetailswidget.h"

#include <libkycpu.h>

CpuDetailsWidget::CpuDetailsWidget(QWidget *parent) : BaseDetailViewWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    initUI();
}

CpuDetailsWidget::~CpuDetailsWidget()
{
}

void CpuDetailsWidget::initUI()
{
    setTitle(tr("Processor"));
    setDetail(kdk_cpu_get_model());

    mCpuChartView = new KChartView();
    mCpuChartView->setAxisTitle("100%", QColor("#3790FA"), tr("CPU"));
    KChartAttr attr;
    attr.colorForeground = QColor("#3790FA");
    attr.colorAlpha = 154;
    mCpuChartView->addItemAttr(attr);

    mCpuGraphVLayout =  new QVBoxLayout();
    mCpuGraphVLayout->setContentsMargins(0,0,0,0);
    mCpuGraphVLayout->setSpacing(0);
    mCpuGraphVLayout->addWidget(mCpuChartView);

    mDetailModel = new CpuDetailsModel();
    mDetailList =  new DetailListTable(mDetailModel);

    mListHLayout = new QHBoxLayout();
    mListHLayout->addWidget(mDetailList);

    mMainLayout->addLayout(mCpuGraphVLayout);
    mMainLayout->addSpacing(8);
    mMainLayout->addLayout(mListHLayout);
}


void CpuDetailsWidget::onUpdateData(qreal cpuPercent)
{
    mCpuChartView->onUpdateData(cpuPercent, 0);
    mDetailModel->onModelUpdated();
}
