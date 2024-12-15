// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONSEARCHEDIT_H
#define COOPERATIONSEARCHEDIT_H

#include "global_defines.h"

#include <QFrame>

class QLineEdit;
class QToolButton;
namespace cooperation_core {

class CooperationSearchEdit : public QFrame
{
    Q_OBJECT

public:
    explicit CooperationSearchEdit(QWidget *parent = nullptr);

    QString text() const;
    void setPlaceholderText(const QString &text);
    void setPlaceHolder(const QString &text);

Q_SIGNALS:
    void textChanged(const QString &);
    void returnPressed();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    QLineEdit *searchEdit{ nullptr };
    CooperationLabel *searchIcon{ nullptr };
    CooperationLabel *searchText{ nullptr };
    QToolButton *closeBtn{ nullptr };
    QString placeholderText;
};

} // namespace cooperation_core

#endif // COOPERATIONSEARCHEDIT_H
