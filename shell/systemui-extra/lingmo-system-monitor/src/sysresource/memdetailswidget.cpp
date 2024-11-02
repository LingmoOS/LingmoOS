#include "memdetailswidget.h"

MemDetailsWidget::MemDetailsWidget(QWidget *parent) : BaseDetailViewWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    initUI();
}

MemDetailsWidget::~MemDetailsWidget()
{
}

void MemDetailsWidget::initUI()
{
    setTitle(tr("Memory"));
    setDetail(tr("Capacity size:(%1)").arg(Format_Memory(kdk_rti_get_mem_res_total_KiB(), KDK_KILOBYTE)));

    // 内存曲线图
    mMemChartView = new KChartView();
    mMemChartView->setAxisTitle("100%", QColor("#32D74B"), tr("Memory"));
    KChartAttr attr;
    attr.colorForeground = QColor("#32D74B");
    attr.colorAlpha = 154;
    mMemChartView->addItemAttr(attr);

    mMemVLayout =  new QVBoxLayout();
    mMemVLayout->setContentsMargins(0,0,0,0);
    mMemVLayout->setSpacing(0);
    mMemVLayout->addWidget(mMemChartView);

    // 交换空间曲线图
    mSwapChartView = new KChartView();
    mSwapChartView->setAxisTitle("100%", QColor("#FF9F0A"), tr("Swap"));
    KChartAttr attr1;
    attr1.colorForeground = QColor("#FF9F0A");
    attr1.colorAlpha = 154;
    mSwapChartView->addItemAttr(attr1);

    mSwapVLayout =  new QVBoxLayout();
    mSwapVLayout->setContentsMargins(0,0,0,0);
    mSwapVLayout->setSpacing(0);
    mSwapVLayout->addWidget(mSwapChartView);

    mMemSwapHLayout = new QHBoxLayout();
    mMemSwapHLayout->setContentsMargins(0,0,0,0);
    mMemSwapHLayout->addLayout(mMemVLayout);
    mMemSwapHLayout->addSpacing(18);
    mMemSwapHLayout->addLayout(mSwapVLayout);

    // 详情列表
    mDetailModel = new MemDetailsModel();
    mDetailList =  new DetailListTable(mDetailModel);

    mListHLayout = new QHBoxLayout();
    mListHLayout->addWidget(mDetailList);

    mMainLayout->addLayout(mMemSwapHLayout);
    mMainLayout->addSpacing(8);
    mMainLayout->addLayout(mListHLayout);
}

void MemDetailsWidget::onUpdateMemData(qreal memUsed, qreal memTotal)
{
    qreal memPercent = (memTotal ? memUsed  / memTotal : 0.0f) * 100.0;
    mMemChartView->onUpdateData(memPercent, 0);
    // 刷新数据列表
    mDetailModel->onModelUpdated();
}

void MemDetailsWidget::onUpdateSwapData(qreal swapUsed, qreal swapTotal)
{
    qreal swapPercent = (swapTotal ? swapUsed  / swapTotal : 0.0f) * 100.0;
    mSwapChartView->onUpdateData(swapPercent, 0);
}

