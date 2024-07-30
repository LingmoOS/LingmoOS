#ifndef _KCOMBOBOXTEST_H
#define _KCOMBOBOXTEST_H

#include <kcompletion_export.h>

#include <QWidget>

class QTimer;
class QComboBox;
class QPushButton;

class KComboBox;

class KComboBoxTest : public QWidget
{
    Q_OBJECT

public:
    KComboBoxTest(QWidget *parent = nullptr);
    ~KComboBoxTest() override;

private Q_SLOTS:
    void quitApp();
    void slotTimeout();
    void slotDisable();
    void slotReturnPressed(const QString &);
    void slotActivated(int);
    void slotTextActivated(const QString &);
    void slotCurrentIndexChanged(int);
    void slotCurrentTextChanged(const QString &);

private:
    void connectComboSignals(QComboBox *combo);

    QComboBox *m_qc;

    KComboBox *m_ro;
    KComboBox *m_rw;
    KComboBox *m_hc;
    KComboBox *m_konqc;

    QPushButton *m_btnExit;
    QPushButton *m_btnEnable;

    QTimer *m_timer;
};

#endif
