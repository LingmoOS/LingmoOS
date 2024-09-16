// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawdialog.h"
#include "utils/baseutils.h"
#include "pagecontext.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>
#include <QApplication>

#ifdef USE_DTK
#define FATHERDILOG DDialog
#else
#define FATHERDILOG MessageDlg
#include <QPushButton>
#endif

DrawDialog::DrawDialog(QWidget *parent)
    : FATHERDILOG(parent)
{
    setModal(true);

    setWgtAccesibleName(this, "Notice save dialog");

#ifdef USE_DTK
    setIcon(QIcon::fromTheme("deepin-draw"));
    addButton(tr("Cancel"), false, DDialog::ButtonNormal);
    addButton(tr("Discard"), false, DDialog::ButtonNormal);
    addButton(tr("Save"), true, DDialog::ButtonRecommend);
    // Input content
    QLabel *title = new QLabel(tr("Save the current contents?"), this);
    title->setWordWrap(true);
    title->setAlignment(Qt::AlignCenter);

    QWidget *w = new QWidget(this);
    w->setFixedHeight(this->height() - 80);

    QVBoxLayout *layout = new QVBoxLayout(w);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addStretch();
    layout->addWidget(title);
    layout->addStretch();
    addContent(w);
#else
//    setWindowIcon(QPixmap(":/theme/common/images/deepin-draw-64.svg"));
//    auto cacelBtn   = new QPushButton(tr("Cancel"),this);
//    auto discardBtn = new QPushButton(tr("Discard"),this);
//    auto saveBtn    = new QPushButton(tr("Save"),this);

//    QHBoxLayout* layout = new QHBoxLayout();
//    layout->addWidget(cacelBtn);
//    layout->addWidget(discardBtn);
//    layout->addWidget(saveBtn);

//    QVBoxLayout *mainLy = new QVBoxLayout(this);
//    mainLy->setContentsMargins(0, 0, 0, 0);
//    mainLy->addLayout(layout);
//    this->setLayout(mainLy);
    setMessage(SMessage(tr("Save the current contents?"), ENormalMsg, QStringList() << tr("Cancel") << tr("Discard") << tr("Save"),
                        QList<EButtonType>() << ENormalMsgBtn << ENormalMsgBtn << ESuggestedMsgBtn));

#endif




}


void  DrawDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        this->close();
    }
}
extern QWidget *defaultParentWindow();
void DrawDialog::showEvent(QShowEvent *event)
{
    QMetaObject::invokeMethod(this, [ = ]() {
        QMetaObject::invokeMethod(this, [ = ]() {

            auto window = this->parentWidget() != nullptr ? this->parentWidget()->window() : defaultParentWindow();
            if (window != nullptr) {
                QPoint centerPos = window->geometry().center() - this->geometry().center();
                QRect parentWindowGem = window->geometry();

                centerPos = parentWindowGem.topLeft() + QPoint((parentWindowGem.width() - this->width()) / 2,
                                                               (parentWindowGem.height() - this->height()) / 2);

                this->move(centerPos);
            }

        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);

    FATHERDILOG::showEvent(event);
}

