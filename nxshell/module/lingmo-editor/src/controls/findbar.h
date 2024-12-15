// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FINDBAR_H
#define FINDBAR_H

#include "linebar.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QWidget>
#include "dimagebutton.h"
#include <QColor>
#include <DIconButton>
#include <DApplicationHelper>
#include <DFloatingWidget>
#include <QMouseEvent>
#include <qmouseeventtransition.h>
#include <QMouseEventTransition>

#include <DPalette>
#include <DAbstractDialog>

DWIDGET_USE_NAMESPACE

class FindBar : public DFloatingWidget
{
    Q_OBJECT

public:
    explicit FindBar(QWidget *parent = 0);

    bool isFocus();
    void focus();

    void activeInput(QString text, QString file, int row, int column, int scrollOffset);
    void setMismatchAlert(bool isAlert);
    void receiveText(QString t);
    void setSearched(bool _);
    void findPreClicked();

Q_SIGNALS:
    void pressEsc();
    void findNext(const QString &keyword);
    void findPrev(const QString &keyword);

    void removeSearchKeyword();
    void updateSearchKeyword(QString file, QString keyword);

    //add guoshao
    void sigFindbarClose();

public Q_SLOTS:
    void findCancel();
    void handleContentChanged();
    void handleFindNext();
    void handleFindPrev();

protected:
    void hideEvent(QHideEvent *event) override;
    bool focusNextPrevChild(bool next) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    Q_SLOT void updateSizeMode();

private:
    QPushButton *m_findNextButton;
    QPushButton *m_findPrevButton;
    DIconButton *m_closeButton;
    LineBar *m_editLine;
    QHBoxLayout *m_layout;
    QLabel *m_findLabel;
    QString m_findFile;
    int m_findFileColumn;
    int m_findFileRow;
    int m_findFileSrollOffset;
    QColor m_backgroundColor;
    QString m_receivedText = " ";
    bool searched = false;

    QPoint last;
};

#endif
