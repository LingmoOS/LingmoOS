#ifndef KACTIONCATEGORYTEST_H
#define KACTIONCATEGORYTEST_H

#include <QObject>

class tst_KActionCategory : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    // Test the creation of action categories,
    void tstCreation();

    // Check the synchronization between the action category and the
    // collection
    void tstSynchronization();

    // Test the creation of actions
    void tstActionCreation();

}; // class tst_KActionCategory

#endif
