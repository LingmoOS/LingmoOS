#include "basedetailviewwidget.h"

BaseDetailViewWidget::BaseDetailViewWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnect();
}

BaseDetailViewWidget::~BaseDetailViewWidget()
{

}

void BaseDetailViewWidget::initUI()
{
    mMainLayout =  new QVBoxLayout();
    mMainLayout->setContentsMargins(16,0,20,16);
    mMainLayout->setSpacing(0);

    QFont ft;
    ft.setPixelSize(36);

    QFont ft1;
    ft1.setPixelSize(14);

    mTitleLabel = new QLabel(mTitleText);
    mTitleLabel->setFont(ft);
    mDetailLabel = new LightLabel(mDetailText);
    mDetailLabel->setFont(ft1);
    mHideBtn = new KBorderlessButton(tr("Hide Details"));
    mHideBtn->setFont(ft1);

    QHBoxLayout *mTopHLayout = new QHBoxLayout();
    mTopHLayout->setContentsMargins(0,0,0,0);
    mTopHLayout->addWidget(mTitleLabel);
    mTopHLayout->addSpacing(8);
    mTopHLayout->addWidget(mDetailLabel);
    mTopHLayout->addStretch();
    mTopHLayout->addWidget(mHideBtn);
    mMainLayout->addLayout(mTopHLayout);

    this->setLayout(mMainLayout);
}

void BaseDetailViewWidget::initConnect()
{
    connect(mHideBtn, &KBorderlessButton::clicked, this, [=](){
        Q_EMIT hideDetails(0);
    });
}

void BaseDetailViewWidget::setTitle(const QString &text)
{
    mTitleText = text;
    mTitleLabel->setText(mTitleText);
}

void BaseDetailViewWidget::setDetail(const QString &text)
{
    mDetailText = text;
    mDetailLabel->setText(mDetailText);
}
