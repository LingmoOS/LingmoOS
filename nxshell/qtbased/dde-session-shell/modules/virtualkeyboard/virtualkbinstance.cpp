// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "virtualkbinstance.h"

#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <QProcess>
#include <QResizeEvent>

const static QSize VIRTUAL_KB_SIZE = QSize(600, 200);

VirtualKBInstance &VirtualKBInstance::Instance()
{
    static VirtualKBInstance virtualKB;
    return virtualKB;
}

VirtualKBInstance::~VirtualKBInstance()
{
    stopVirtualKBProcess();
}

void VirtualKBInstance::init()
{
    if (m_virtualKBWidget) {
        return;
    }

    // 初始化启动onborad进程
    if (!m_virtualKBProcess) {
        m_virtualKBProcess = new QProcess(this);

        connect(m_virtualKBProcess, &QProcess::readyReadStandardOutput, [ = ]{
            // 启动完成onborad进程后，获取onborad主界面，将主界面显示在锁屏界面上
            QByteArray output = m_virtualKBProcess->readAllStandardOutput();

            if (output.isEmpty())
                return;

            int xid = atoi(output.trimmed().toStdString().c_str());
            QWindow *w = QWindow::fromWinId(static_cast<WId>(xid));
            m_virtualKBWidget = QWidget::createWindowContainer(w);
            m_virtualKBWidget->setAccessibleName("VirtualKBWidget");
            m_virtualKBWidget->setFixedSize(VIRTUAL_KB_SIZE);
            m_virtualKBWidget->hide();

            QTimer::singleShot(300, [=] {
                showKeyboardWidget(m_parentWidget);
            });
        });

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        auto finished = QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished);
#else
        auto finished = QOverload<int>::of(&QProcess::finished);
#endif
        connect(m_virtualKBProcess, finished, this, &VirtualKBInstance::onVirtualKBProcessFinished);
        m_virtualKBProcess->start("onboard", QStringList() << "-e" << "--layout" << "Small" << "--size" << "600x200" << "-a");
    }
}

void VirtualKBInstance::stopVirtualKBProcess()
{
    // 结束onborad进程
    if (m_virtualKBProcess) {
        m_virtualKBProcess->close();
        m_virtualKBWidget = nullptr;
    }
}

void VirtualKBInstance::showKeyboardWidget(QWidget *parent)
{
    if (!parent) {
        return;
    }

    m_parentWidget = parent;
    if (!m_virtualKBWidget) {
        init();
        return;
    }

    m_virtualKBWidget->setParent(parent);
    m_virtualKBWidget->setVisible(!m_virtualKBWidget->isVisible());

    if (m_virtualKBWidget->isVisible()) {
        m_virtualKBWidget->raise();
        m_virtualKBWidget->resize(VIRTUAL_KB_SIZE);

        // 关联了父窗口后，移动位置是相对父窗口移动，而不是扩展屏幕的位置坐标
        QPoint point = QPoint((parent->width() - m_virtualKBWidget->width()) / 2,
                              parent->height() - m_virtualKBWidget->height() - 50);
        m_virtualKBWidget->move(point);
    }
}

void VirtualKBInstance::hideKeyboardWidget()
{
    if (m_virtualKBWidget && m_virtualKBWidget->isVisible()) {
        m_virtualKBWidget->setVisible(false);
    }
}

VirtualKBInstance::VirtualKBInstance(QObject *parent)
    : QObject(parent)
    , m_parentWidget(nullptr)
    , m_virtualKBWidget(nullptr)
    , m_virtualKBProcess(nullptr)
{

}

void VirtualKBInstance::onVirtualKBProcessFinished(int exitCode)
{
    Q_UNUSED(exitCode)
    m_virtualKBProcess = nullptr;
}
