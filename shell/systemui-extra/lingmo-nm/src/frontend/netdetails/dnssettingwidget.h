#ifndef DNSSETTINGWIDGET_H
#define DNSSETTINGWIDGET_H

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QDBusInterface>
#include <QPushButton>

#include "divider.h"

class DnsSettingWidget : public QDialog
{
    Q_OBJECT
public:
    explicit DnsSettingWidget(QString timeout = "", QString retry = "", QString tactic = "", QWidget *parent = nullptr);

    void getDnsSettings(QString& timeout, QString& retry, QString& tactic) {
        timeout = QString::number(m_timeoutComboBox->currentData().toInt());
        retry = QString::number(m_retryComboBox->currentData().toInt());
        tactic = m_tacticComboBox->currentData().toString();
    }

private:
    QWidget* m_titleWidget;
    QWidget* m_centerWidget;
    QWidget* m_bottomWidget;

    QString m_timeout;
    QString m_retry;
    QString m_tactic;

    QLabel* m_titleLabel;
    QLabel* m_tacticLabel;
    QLabel* m_timeoutLabel;
    QLabel* m_retryLabel;

    QComboBox* m_tacticComboBox;
    QComboBox* m_timeoutComboBox;
    QComboBox* m_retryComboBox;

    Divider *m_bottomDivider = nullptr;

    QPushButton *m_closeBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_confirmBtn;

    void initUi();
    void initConnect();

private Q_SLOTS:
    void onPaletteChanged();

};

#endif // DNSSETTINGWIDGET_H
