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
    connect(m_comboCodec, qOverload<QAction *>(&KSelectAction::triggered), this, &CodecActionTest::actionTriggered);
    connect(m_comboCodec, &KSelectAction::indexTriggered, this, &CodecActionTest::indexTriggered);
    connect(m_comboCodec, &KSelectAction::textTriggered, this, &CodecActionTest::textTriggered);
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
    connect(m_comboCodec, &KCodecAction::codecTriggered, this, &CodecActionTest::codecTriggered);
#endif
    connect(m_comboCodec, &KCodecAction::codecNameTriggered, this, &CodecActionTest::nameTriggered);

    m_buttonCodec->setToolBarMode(KCodecAction::MenuMode);
    connect(m_buttonCodec, qOverload<QAction *>(&KSelectAction::triggered), this, &CodecActionTest::actionTriggered);
    connect(m_buttonCodec, &KSelectAction::indexTriggered, this, &CodecActionTest::indexTriggered);
    connect(m_buttonCodec, &KSelectAction::textTriggered, this, &CodecActionTest::textTriggered);
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
    connect(m_buttonCodec, &KCodecAction::codecTriggered, this, &CodecActionTest::codecTriggered);
#endif
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

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
void CodecActionTest::codecTriggered(QTextCodec *codec)
{
    qDebug() << codec->name() << ':' << codec->mibEnum();
}
#endif

void CodecActionTest::nameTriggered(const QString &codecName)
{
    qDebug() << codecName;
}

void CodecActionTest::slotActionTriggered(bool state)
{
    qDebug() << sender() << " state " << state;
}
