#include "netdetailswidget.h"

NetDetailsWidget::NetDetailsWidget(QWidget *parent) : BaseDetailViewWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    initUI();
}

NetDetailsWidget::~NetDetailsWidget()
{

}

void NetDetailsWidget::initUI()
{
    setTitle(tr("Network"));

    mNetGraphScrollArea =  new NetViewScrollArea();

    // 详情列表
    mDetailModel = new NetDetailsModel();
    NetInfoDetailItemDelegate *netItemDelegate = new NetInfoDetailItemDelegate();
    mDetailList =  new DetailListTable(mDetailModel, netItemDelegate);
    mCurrentNetName = mNetGraphScrollArea->getCurrentNCName();
    mDetailModel->refreshNetifInfo(mCurrentNetName);

    mListHLayout = new QHBoxLayout();
    mListHLayout->addWidget(mDetailList);

    mMainLayout->addWidget(mNetGraphScrollArea);
    mMainLayout->addSpacing(8);
    mMainLayout->addLayout(mListHLayout);

    connect(mNetGraphScrollArea, &NetViewScrollArea::netifItemClicked, this, [=](const QString &mac){
        mDetailModel->refreshNetifInfo(mac);
        mCurrentNetName = mac;
    });
}

void NetDetailsWidget::onUpdateData(qreal data)
{
    mNetGraphScrollArea->onModelUpdate();
    mDetailModel->refreshNetifInfo(mCurrentNetName);
}
