// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BtnLabel.h"
#include "commontools.h"
#include "DDLog.h"

#include <QBoxLayout>

#include <DDialog>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DApplication>
#include <QLoggingCategory>
using namespace DDLog;
BtnLabel::BtnLabel(DWidget *parent)
    : DLabel(parent)
    , m_Desc("")
{
    // bug132075 安装成功状态此button无法点击
    // 仅安装失败可点击
    connect(this, &QLabel::linkActivated, this, [this]() {
        if (m_Desc.isEmpty())
            return;

        if (m_Desc == QString("Backup Failed")) {
            DDialog dialog;

            DWidget *contentFrame = new DWidget(this);

            DLabel *retryLabel = new DLabel(this);
            QVBoxLayout *vLayout = new QVBoxLayout(this);
            retryLabel->setElideMode(Qt::ElideMiddle);
            retryLabel->setText(QObject::tr("Please try again or give us feedback"));
            vLayout->addWidget(retryLabel, 0, Qt::AlignHCenter);

            contentFrame->setLayout(vLayout);

            dialog.setIcon(QIcon::fromTheme("cautious"));
            dialog.setTitle(QObject::tr("Driver backup failed!"));
            dialog.addContent(contentFrame);
            dialog.addButton(tr("OK"), false, DDialog::ButtonNormal);
            dialog.addButton(tr("Feedback"), false, DDialog::ButtonNormal);

            int clickedButtonIndex = dialog.exec();
            if (1 == clickedButtonIndex) {
                //反馈
                qCDebug(appLog) << __func__ << "fedback....";
                CommonTools::feedback();
            }
        } else {
            DDialog dialog;
            dialog.setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
            dialog.setTitle(m_Desc);
            dialog.addButton(tr("OK", "button"));
            dialog.exec();
        }
    });
}

void BtnLabel::setDesc(const QString &txt)
{
    m_Desc = txt;
}

void BtnLabel::paintEvent(QPaintEvent* e)
{
    DPalette::ColorGroup cg;
    if (Qt::ApplicationActive == DApplication::applicationState()) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    DPalette pa = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    pa.setColor(DPalette::Text, pa.color(cg, DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(this, pa);

    return DLabel::paintEvent(e);
}


TipsLabel::TipsLabel(DWidget *parent)
    : DLabel(parent)
{

}

void TipsLabel::paintEvent(QPaintEvent* e)
{
    DPalette::ColorGroup cg;
    if (Qt::ApplicationActive == DApplication::applicationState()) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    DPalette pa = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    pa.setColor(DPalette::Text, pa.color(cg, DPalette::TextTips));
    DApplicationHelper::instance()->setPalette(this, pa);

    return DLabel::paintEvent(e);
}

TitleLabel::TitleLabel(DWidget *parent)
    : DLabel(parent)
{
    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
}

void TitleLabel::paintEvent(QPaintEvent* e)
{
    DPalette::ColorGroup cg;
    if (Qt::ApplicationActive == DApplication::applicationState()) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    DPalette pa = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    pa.setColor(DPalette::Text, pa.color(cg, DPalette::TextTitle));
    DApplicationHelper::instance()->setPalette(this, pa);

    return DLabel::paintEvent(e);
}


