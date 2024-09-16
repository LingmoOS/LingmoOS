// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REPLACEBAR_H
#define REPLACEBAR_H

#include <QPushButton>
#include "linebar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QWidget>
#include "dimagebutton.h"
#include <DIconButton>
#include <DApplicationHelper>
#include <DFloatingWidget>
#include <DAbstractDialog>

DWIDGET_USE_NAMESPACE

class ReplaceBar : public DFloatingWidget
{
    Q_OBJECT

public:
    explicit ReplaceBar(QWidget *parent = 0);

    bool isFocus();
    void focus();

    void activeInput(QString text, QString file, int row, int column, int scrollOffset);
    void setMismatchAlert(bool isAlert);
    void setsearched(bool _);

Q_SIGNALS:
    void pressEsc();
    void replaceNext(QString file, QString replaceText, QString withText);
    void replaceSkip(QString file, QString keyword);
    void replaceRest(QString replaceText, QString withText);
    void replaceAll(QString replaceText, QString withText);
    void beforeReplace(QString _);

    void backToPosition(QString file, int row, int column, int scrollOffset);

    void removeSearchKeyword();
    void updateSearchKeyword(QString file, QString keyword);

    void sigReplacebarClose();

public Q_SLOTS:
    void change();
    void replaceClose();
    void handleContentChanged();
    void handleReplaceAll();
    void handleReplaceNext();
    void handleReplaceRest();
    void handleSkip();

protected:
    void hideEvent(QHideEvent *event);
    bool focusNextPrevChild(bool next);
    void keyPressEvent(QKeyEvent *e);

private:
    Q_SLOT void updateSizeMode();
    QVBoxLayout *createVerticalLine(QWidget *content) const;

private:
    QPushButton *m_replaceAllButton;
    QPushButton *m_replaceButton;
    QPushButton *m_replaceRestButton;
    QPushButton *m_replaceSkipButton;
    DIconButton *m_closeButton;
    LineBar *m_replaceLine;
    LineBar *m_withLine;
    QHBoxLayout *m_layout;
    QLabel *m_replaceLabel;
    QLabel *m_withLabel;
    QString m_replaceFile;
    int m_replaceFileColumn;
    int m_replaceFileRow;
    int m_replaceFileSrollOffset;
    QColor m_backgroundColor;
    bool searched = false;

    QPoint last;
};

#endif
