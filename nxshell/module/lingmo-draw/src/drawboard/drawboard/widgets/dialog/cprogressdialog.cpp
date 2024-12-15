// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cprogressdialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>

ProgressDialog::ProgressDialog(const QString &text, QWidget *parent):
#ifdef USE_DTK
DDialog(parent)
#else
QDialog(parent)
#endif
{
    initUI();
    _titleLabel->setText(text);
}

void ProgressDialog::initUI()
{
    setFixedSize(QSize(400, 120));
    setModal(true);
#ifdef USE_DTK
    setCloseButtonVisible(false);
#endif

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedHeight(8);


    QWidget *widget = new QWidget(this);
    _titleLabel = new QLabel(this);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->addWidget(_titleLabel);
    layout->addWidget(m_progressBar);
#ifdef USE_DTK
    addContent(widget, Qt::AlignVCenter);
#else
    setLayout(layout);
#endif
}

void ProgressDialog::setProcess(int process, int total)
{
    m_progressBar->setRange(0, total);
    m_progressBar->setValue(process);
}

int ProgressDialog::exec()
{
    m_progressBar->reset();

    if (!isVisible()) {
        if (parentWidget() != nullptr) {
            QRect rct = parentWidget()->window()->geometry();
#ifdef USE_DTK
            this->moveToCenterByRect(rct);
#endif
        }

        show();
    }
    return 0;
}

void ProgressDialog::setText(const QString &text)
{
    _titleLabel->setText(text);
}

void CAbstractProcessDialog::setTitle(const QString &title)
{
    _titleLabel->setText(title);
}

void CAbstractProcessDialog::setProcess(int process)
{
    _progressBar->setValue(process);
}

CAbstractProcessDialog::CAbstractProcessDialog(QWidget *parent):
#ifdef USE_DTK
DAbstractDialog(parent)
#else
QDialog(parent)
#endif
{
    setFixedSize(QSize(480, 80));

    _titleLabel = new QLabel(this);
    _titleLabel->setGeometry(QRect(40, 20, 400, 24));
    _titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _titleLabel->setText(tr("Export"));

    _progressBar = new QProgressBar(this);
    _progressBar->setGeometry(QRect(40, 54, 400, 6));
    _progressBar->setRange(0, 100);

    setWindowModality(Qt::WindowModal);
}
void CAbstractProcessDialog::paintEvent(QPaintEvent *event)
{
#ifdef USE_DTK
    DAbstractDialog::paintEvent(event);
#else
    QDialog::paintEvent(event);
#endif
}
