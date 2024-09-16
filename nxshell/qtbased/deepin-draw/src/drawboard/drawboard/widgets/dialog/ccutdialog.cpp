// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ccutdialog.h"

#include <QKeyEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QAbstractButton>
#include <QDebug>

CCutDialog::CCutDialog(QWidget *parent)
    : MessageDlg(parent)
    , m_cutStatus(Discard)
{
//    setModal(true);

    setWgtAccesibleName(this, "Notice cut info dialog");
    this->setObjectName("CutDialog");

//    this->setIcon(QPixmap(":/icons/deepin/builtin/Bullet_window_warning.svg"));

//    addButton(tr("Discard"), false, DDialog::ButtonNormal);
//    addButton(tr("Save"), true, DDialog::ButtonRecommend);
//    this->setObjectName("CutDialog");
//    this->getButton(1)->setObjectName("CutDialogSave");

//    // Input content
//    QLabel *title = new QLabel(tr("Do you want to save the cropped image?"), this);
//    title->setWordWrap(true);
//    title->setAlignment(Qt::AlignCenter);

//    QWidget *w = new QWidget(this);
//    w->setFixedHeight(this->height() - 80);

//    QVBoxLayout *layout = new QVBoxLayout(w);
//    layout->setContentsMargins(0, 0, 0, 0);
//    layout->setSpacing(8);
//    layout->addStretch();
//    layout->addWidget(title);
//    layout->addStretch();
//    addContent(w);

//    connect(this, &CCutDialog::buttonClicked, this, [ = ](int index, const QString & text) {
//        Q_UNUSED(text)
//        /*if (0 == index) {
//            m_cutStatus = Discard;
//        } else */if (0 == index) {
//            m_cutStatus = Discard;
//        } else if (1 == index) {
//            m_cutStatus = Save;
//        }
//        this->close();
//    });

    setMessage(SMessage(tr("Do you want to save the cropped image?"),EWarningMsg,
                        QStringList()<<tr("Discard")<<tr("Save"),
                        QList<EButtonType>()<<ENormalMsgBtn<<ESuggestedMsgBtn));
}

CCutDialog::CutStatus CCutDialog::getCutStatus()
{
    return m_cutStatus;
}

int CCutDialog::exec()
{
    int ret = MessageDlg::exec();
    if (0 == ret) {
        m_cutStatus = Discard;
    } else if (1 == ret) {
        m_cutStatus = Save;
    }
    return getCutStatus();
}

void CCutDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        m_cutStatus = Discard;
        this->close();
    }
}
extern QWidget *defaultParentWindow();
void CCutDialog::showEvent(QShowEvent *event)
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

    MessageDlg::showEvent(event);
}
