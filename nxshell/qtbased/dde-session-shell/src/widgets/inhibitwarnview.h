// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INHIBITWARNVIEW_H
#define INHIBITWARNVIEW_H

#include "warningview.h"
#include "rounditembutton.h"
#include "sessionbasemodel.h"

#include <QWidget>

class QPushButton;
class InhibitorRow : public QWidget
{
    Q_OBJECT
public:
    InhibitorRow(const QString &who, const QString &why, const QIcon &icon = QIcon(), QWidget *parent = nullptr);
    ~InhibitorRow() override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

class InhibitWarnView : public WarningView
{
    Q_OBJECT

public:
    explicit InhibitWarnView(SessionBaseModel::PowerAction inhibitType, QWidget *parent = nullptr);
    ~InhibitWarnView() override;

    struct InhibitorData {
        QString who;
        QString why;
        QString mode;
        quint32 pid;
        QString icon;
    };

    void setInhibitorList(const QList<InhibitorData> & list);
    void setInhibitConfirmMessage(const QString &text);
    void setAcceptReason(const QString &reason) override;
    void setAcceptVisible(const bool acceptable);

    void toggleButtonState() Q_DECL_OVERRIDE;
    void buttonClickHandle() Q_DECL_OVERRIDE;

    SessionBaseModel::PowerAction inhibitType() const;

protected:
    void updateIcon();
    bool focusNextPrevChild(bool next) Q_DECL_OVERRIDE;
    void setCurrentButton(const ButtonType btntype) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void cancelled() const;
    void actionInvoked() const;

private:
    void onOtherPageDataChanged(const QVariant &value);

private:
    SessionBaseModel::PowerAction m_inhibitType;
    QList<QWidget*> m_inhibitorPtrList;
    QVBoxLayout *m_inhibitorListLayout = nullptr;
    QLabel *m_confirmTextLabel = nullptr;
    QPushButton *m_acceptBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_currentBtn;
    int m_dataBindIndex;
};

#endif // INHIBITWARNVIEW_H
