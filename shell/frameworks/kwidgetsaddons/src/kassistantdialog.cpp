/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Olivier Goffart <ogoffart at kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kassistantdialog.h"

#include "kpagedialog_p.h"
#include <QApplication>
#include <QDialogButtonBox>
#include <QIcon>
#include <QPushButton>

#include <QHash>

class KAssistantDialogPrivate : public KPageDialogPrivate
{
    Q_DECLARE_PUBLIC(KAssistantDialog)
    Q_DECLARE_TR_FUNCTIONS(KAssistantDialog)

public:
    KAssistantDialogPrivate(KAssistantDialog *qq)
        : KPageDialogPrivate(qq)
    {
    }

    QHash<KPageWidgetItem *, bool> valid;
    QHash<KPageWidgetItem *, bool> appropriate;
    KPageWidgetModel *pageModel = nullptr;
    QPushButton *backButton = nullptr;
    QPushButton *nextButton = nullptr;
    QPushButton *finishButton = nullptr;

    void init();
    void slotUpdateButtons();

    QModelIndex getNext(QModelIndex nextIndex)
    {
        QModelIndex currentIndex;
        do {
            currentIndex = nextIndex;
            nextIndex = pageModel->index(0, 0, currentIndex);
            if (!nextIndex.isValid()) {
                nextIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
            }
        } while (nextIndex.isValid() && !appropriate.value(pageModel->item(nextIndex), true));
        return nextIndex;
    }

    QModelIndex getPrevious(QModelIndex nextIndex)
    {
        QModelIndex currentIndex;
        do {
            currentIndex = nextIndex;
            nextIndex = currentIndex.sibling(currentIndex.row() - 1, 0);
            if (!nextIndex.isValid()) {
                nextIndex = currentIndex.parent();
            }
        } while (nextIndex.isValid() && !appropriate.value(pageModel->item(nextIndex), true));
        return nextIndex;
    }
};

KAssistantDialog::KAssistantDialog(QWidget *parent, Qt::WindowFlags flags)
    : KPageDialog(*new KAssistantDialogPrivate(this), nullptr, parent, flags)
{
    Q_D(KAssistantDialog);

    d->init();
    // workaround to get the page model
    KPageWidget *pagewidget = findChild<KPageWidget *>();
    Q_ASSERT(pagewidget);
    d->pageModel = static_cast<KPageWidgetModel *>(pagewidget->model());
}

KAssistantDialog::KAssistantDialog(KPageWidget *widget, QWidget *parent, Qt::WindowFlags flags)
    : KPageDialog(*new KAssistantDialogPrivate(this), widget, parent, flags)
{
    Q_D(KAssistantDialog);

    d->init();
    d->pageModel = static_cast<KPageWidgetModel *>(widget->model());
}

KAssistantDialog::~KAssistantDialog() = default;

void KAssistantDialogPrivate::init()
{
    Q_Q(KAssistantDialog);

    QDialogButtonBox *buttonBox = q->buttonBox();

    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Help);
    backButton = new QPushButton;

    const QString iconBack = QApplication::isRightToLeft() ? QStringLiteral("go-next") : QStringLiteral("go-previous");
    const QString iconNext = QApplication::isRightToLeft() ? QStringLiteral("go-previous") : QStringLiteral("go-next");
    backButton->setText(tr("&Back", "@action:button go back"));
    backButton->setIcon(QIcon::fromTheme(iconBack));
    backButton->setToolTip(tr("Go back one step", "@info:tooltip"));
    q->connect(backButton, &QAbstractButton::clicked, q, &KAssistantDialog::back);
    buttonBox->addButton(backButton, QDialogButtonBox::ActionRole);

    nextButton = new QPushButton;
    nextButton->setText(tr("Next", "@action:button Opposite to Back"));
    nextButton->setIcon(QIcon::fromTheme(iconNext));
    nextButton->setDefault(true);
    q->connect(nextButton, &QAbstractButton::clicked, q, &KAssistantDialog::next);
    buttonBox->addButton(nextButton, QDialogButtonBox::ActionRole);

    finishButton = new QPushButton;
    finishButton->setText(tr("Finish", "@action:button"));
    finishButton->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
    buttonBox->addButton(finishButton, QDialogButtonBox::AcceptRole);

    q->setFaceType(KPageDialog::Plain);

    q->connect(q, &KAssistantDialog::currentPageChanged, q, [this]() {
        slotUpdateButtons();
    });
}

void KAssistantDialog::back()
{
    Q_D(KAssistantDialog);

    QModelIndex nextIndex = d->getPrevious(d->pageModel->index(currentPage()));
    if (nextIndex.isValid()) {
        setCurrentPage(d->pageModel->item(nextIndex));
    }
}

void KAssistantDialog::next()
{
    Q_D(KAssistantDialog);

    QModelIndex nextIndex = d->getNext(d->pageModel->index(currentPage()));
    if (nextIndex.isValid()) {
        setCurrentPage(d->pageModel->item(nextIndex));
    } else if (isValid(currentPage())) {
        accept();
    }
}

void KAssistantDialog::setValid(KPageWidgetItem *page, bool enable)
{
    Q_D(KAssistantDialog);

    d->valid[page] = enable;
    if (page == currentPage()) {
        d->slotUpdateButtons();
    }
}

bool KAssistantDialog::isValid(KPageWidgetItem *page) const
{
    Q_D(const KAssistantDialog);

    return d->valid.value(page, true);
}

void KAssistantDialogPrivate::slotUpdateButtons()
{
    Q_Q(KAssistantDialog);

    QModelIndex currentIndex = pageModel->index(q->currentPage());
    // change the title of the next/finish button
    QModelIndex nextIndex = getNext(currentIndex);
    finishButton->setEnabled(!nextIndex.isValid() && q->isValid(q->currentPage()));
    nextButton->setEnabled(nextIndex.isValid() && q->isValid(q->currentPage()));
    finishButton->setDefault(!nextIndex.isValid());
    nextButton->setDefault(nextIndex.isValid());
    // enable or disable the back button;
    nextIndex = getPrevious(currentIndex);
    backButton->setEnabled(nextIndex.isValid());
}

void KAssistantDialog::showEvent(QShowEvent *event)
{
    Q_D(KAssistantDialog);

    d->slotUpdateButtons(); // called because last time that function was called is when the first page was added, so the next button show "finish"
    KPageDialog::showEvent(event);
}

void KAssistantDialog::setAppropriate(KPageWidgetItem *page, bool appropriate)
{
    Q_D(KAssistantDialog);

    d->appropriate[page] = appropriate;
    d->slotUpdateButtons();
}

bool KAssistantDialog::isAppropriate(KPageWidgetItem *page) const
{
    Q_D(const KAssistantDialog);

    return d->appropriate.value(page, true);
}

QPushButton *KAssistantDialog::backButton() const
{
    Q_D(const KAssistantDialog);

    return d->backButton;
}

QPushButton *KAssistantDialog::nextButton() const
{
    Q_D(const KAssistantDialog);

    return d->nextButton;
}

QPushButton *KAssistantDialog::finishButton() const
{
    Q_D(const KAssistantDialog);

    return d->finishButton;
}

#include "moc_kassistantdialog.cpp"
