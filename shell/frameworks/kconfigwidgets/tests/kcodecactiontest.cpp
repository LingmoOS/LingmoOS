#include "kcodecactiontest.h"

#include <QApplication>
#include <QMenuBar>
#include <QToolBar>

#include <QDebug>

#include <kcodecaction.h>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("kcodecactiontest"));
    QApplication app(argc, argv);

    CodecActionTest *test = new CodecActionTest;
    test->show();

    return app.exec();
}

CodecActionTest::CodecActionTest(QWidget *parent)
    : QMainWindow(parent)
    , m_comboCodec(new KCodecAction(QStringLiteral("Combo Codec Action"), this))
    , m_buttonCodec(new KCodecAction(QStringLiteral("Button Codec Action"), this))
{
    // clang-format off
    m_comboCodec->setToolBarMode(KCodecAction::ComboBoxMode);
    connect(m_comboCodec, &KSelectAction::actionTriggered, this, &CodecActionTest::actionTriggered);
    connect(m_comboCodec, &KSelectAction::indexTriggered, this, &CodecActionTest::indexTriggered);
    connect(m_comboCodec, &KSelectAction::textTriggered, this, &CodecActionTest::textTriggered);
    connect(m_comboCodec, &KCodecAction::codecNameTriggered, this, &CodecActionTest::nameTriggered);

    m_buttonCodec->setToolBarMode(KCodecAction::MenuMode);
    connect(m_buttonCodec, &KSelectAction::actionTriggered, this, &CodecActionTest::actionTriggered);
    connect(m_buttonCodec, &KSelectAction::indexTriggered, this, &CodecActionTest::indexTriggered);
    connect(m_buttonCodec, &KSelectAction::textTriggered, this, &CodecActionTest::textTriggered);
    connect(m_buttonCodec, &KCodecAction::codecNameTriggered, this, &CodecActionTest::nameTriggered);
    // clang-format on

    menuBar()->addAction(m_comboCodec);
    menuBar()->addAction(m_buttonCodec);

    QToolBar *toolBar = addToolBar(QStringLiteral("Test"));
    toolBar->addAction(m_comboCodec);
    toolBar->addAction(m_buttonCodec);
}

void CodecActionTest::actionTriggered(QAction *action)
{
    qDebug() << action;
}

void CodecActionTest::indexTriggered(int index)
{
    qDebug() << index;
}

void CodecActionTest::textTriggered(const QString &text)
{
    qDebug() << text;
}

void CodecActionTest::nameTriggered(const QByteArray &codecName)
{
    qDebug() << codecName;
}

void CodecActionTest::slotActionTriggered(bool state)
{
    qDebug() << sender() << " state " << state;
}

#include "moc_kcodecactiontest.cpp"
