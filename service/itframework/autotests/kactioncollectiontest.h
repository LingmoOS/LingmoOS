#ifndef KACTIONCOLLECTIONTEST_H
#define KACTIONCOLLECTIONTEST_H

#include "kactioncollection.h"
#include <KConfigGroup>

class tst_KActionCollection : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void init();
    void cleanup();

private Q_SLOTS:
    void clear();
    void deleted();
    void take();
    void writeSettings();
    void readSettings();
    void insertReplaces1();
    void insertReplaces2();
    void testSetShortcuts();
    void implicitStandardActionInsertionUsingCreate();
    void implicitStandardActionInsertionUsingCut();
    void shouldEmitSignals();
    void addStandardActionFunctorSignal();

private:
    KConfigGroup clearConfig();

    KActionCollection *collection;
};

#endif // KACTIONCOLLECTIONTEST_H
