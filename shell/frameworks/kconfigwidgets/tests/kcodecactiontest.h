#ifndef KCODECACTION_TEST_H
#define KCODECACTION_TEST_H

#include <QMainWindow>

#include "kcodecaction.h"

class CodecActionTest : public QMainWindow
{
    Q_OBJECT

public:
    CodecActionTest(QWidget *parent = nullptr);

public Q_SLOTS:
    void actionTriggered(QAction *action);
    void indexTriggered(int index);
    void textTriggered(const QString &text);
    void nameTriggered(const QByteArray &name);

    void slotActionTriggered(bool state);

private:
    KCodecAction *m_comboCodec;
    KCodecAction *m_buttonCodec;
};

#endif
