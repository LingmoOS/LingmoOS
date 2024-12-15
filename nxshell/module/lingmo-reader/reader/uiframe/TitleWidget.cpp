                                                                                                                                                                               // Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TitleWidget.h"
#include "ScaleWidget.h"
#include "DocSheet.h"
#include "MsgHeader.h"
#include "MainWindow.h"
#include "Application.h"

#include <QHBoxLayout>

TitleWidget::TitleWidget(DWidget *parent)
    : BaseWidget(parent)
{
    m_pThumbnailBtn = new DIconButton(this);
    m_pThumbnailBtn->setObjectName("Thumbnails");
    m_pThumbnailBtn->setToolTip(tr("Thumbnails"));
    m_pThumbnailBtn->setCheckable(true);
    m_pThumbnailBtn->setDisabled(true);
    m_pThumbnailBtn->setIcon(QIcon::fromTheme(QString("dr_") + "thumbnails"));
    connect(m_pThumbnailBtn, SIGNAL(clicked(bool)), SLOT(onThumbnailBtnClicked(bool)));

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(6, 0, 6, 0);
    hlayout->setSpacing(10);
    setLayout(hlayout);
    hlayout->addWidget(m_pThumbnailBtn);

    m_pSw = new ScaleWidget(this);
    m_pSw->setDisabled(true);
    hlayout->addWidget(m_pSw);
    hlayout->addStretch(1);

    DIconButton *decBtn = m_pSw->findChild<DIconButton *>("SP_DecreaseElement");
    decBtn->setEnabledCircle(true);
    DIconButton *incBtn = m_pSw->findChild<DIconButton *>("SP_IncreaseElement");
    incBtn->setEnabledCircle(true);

    DLineEdit *scaleEdit = m_pSw->findChild<DLineEdit *>("scaleEdit");

    DIconButton *optBtn = parent->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    DIconButton *fullBtn = parent->findChild<DIconButton *>("DTitlebarDWindowQuitFullscreenButton");
    DIconButton *minBtn = parent->findChild<DIconButton *>("DTitlebarDWindowMinButton");
    DIconButton *maxBtn = parent->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
    DIconButton *closeBtn = parent->findChild<DIconButton *>("DTitlebarDWindowCloseButton");

    QList<QWidget *> orderlst;
    orderlst << m_pThumbnailBtn;
    orderlst << decBtn;
    orderlst << scaleEdit;
    orderlst << incBtn;
    orderlst << optBtn;
    if (fullBtn)
        orderlst << fullBtn;
    orderlst << minBtn;
    orderlst << maxBtn;
    orderlst << closeBtn;
    parent->setProperty("orderlist", QVariant::fromValue(orderlst));

    for (QWidget *widget : orderlst) {
        if (widget)
            widget->setFocusPolicy(Qt::TabFocus);
    }
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::NormalMode) {
            m_pThumbnailBtn->setFixedSize(QSize(36, 36));
            m_pThumbnailBtn->setIconSize(QSize(36, 36));
    } else {
            m_pThumbnailBtn->setFixedSize(QSize(24, 24));
            m_pThumbnailBtn->setIconSize(QSize(36, 36));
    }
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
                m_pThumbnailBtn->setFixedSize(QSize(36, 36));
                m_pThumbnailBtn->setIconSize(QSize(36, 36));
        } else {
                m_pThumbnailBtn->setFixedSize(QSize(24, 24));
                m_pThumbnailBtn->setIconSize(QSize(36, 36));
        }
    });
#else
        m_pThumbnailBtn->setFixedSize(QSize(36, 36));
        m_pThumbnailBtn->setIconSize(QSize(36, 36));
#endif
}

TitleWidget::~TitleWidget()
{

}

void TitleWidget::keyPressEvent(QKeyEvent *event)
{
    if (nullptr == m_pSw)
        return BaseWidget::keyPressEvent(event);

    if (event->key() == Qt::Key_Up && !event->isAutoRepeat()) {
        m_pSw->onPrevScale();
    } else if (event->key() == Qt::Key_Down && !event->isAutoRepeat()) {
        m_pSw->onNextScale();
    }

    return BaseWidget::keyPressEvent(event);
}

void TitleWidget::setBtnDisable(const bool &bAble)
{
    m_pThumbnailBtn->setDisabled(bAble);
    m_pSw->setDisabled(bAble);
}

void TitleWidget::onCurSheetChanged(DocSheet *sheet)
{
    m_curSheet = sheet;

    emit dApp->emitSheetChanged();

    if (nullptr == m_curSheet || m_curSheet->fileType() == Dr::Unknown) {
        setBtnDisable(true);
        return;

    } else if (Dr::PDF == m_curSheet->fileType() || Dr::DOCX == m_curSheet->fileType()) {
        if (m_curSheet->opened()) {
            setBtnDisable(false);
            m_pThumbnailBtn->setChecked(m_curSheet->operation().sidebarVisible);
        } else {
            setBtnDisable(true);
            m_pThumbnailBtn->setChecked(false);
        }
        m_pSw->setSheet(m_curSheet);

    } else if (Dr::DJVU == m_curSheet->fileType()) {
        m_pThumbnailBtn->setDisabled(false);
        m_pSw->setDisabled(false);

        m_pThumbnailBtn->setChecked(m_curSheet->operation().sidebarVisible);
        m_pSw->setSheet(m_curSheet);
    }
}

void TitleWidget::onThumbnailBtnClicked(bool checked)
{
    if (m_curSheet.isNull())
        return;

    m_pThumbnailBtn->setChecked(checked);
    bool rl = m_pThumbnailBtn->isChecked();
    m_curSheet->setSidebarVisible(rl);
}

void TitleWidget::onFindOperation(const int &sAction)
{
    if (sAction == E_FIND_CONTENT) {
        m_pThumbnailBtn->setChecked(true);
    } else if (sAction == E_FIND_EXIT) {
        if (m_curSheet) {
            bool close = m_curSheet->operation().sidebarVisible;
            m_pThumbnailBtn->setChecked(close);
        }
    }
}

void TitleWidget::setControlEnabled(const bool &enable)
{
    m_pThumbnailBtn->setChecked(false);
    m_pThumbnailBtn->setEnabled(enable);
    m_pSw->clear();
}
