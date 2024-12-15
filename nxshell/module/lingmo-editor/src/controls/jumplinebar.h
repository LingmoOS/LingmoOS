// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JUMPLINEBAR_H
#define JUMPLINEBAR_H

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QWidget>
#include <DApplicationHelper>
#include <DFloatingWidget>
#include <DSpinBox>
#include <QLineEdit>
#include <QEvent>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

class JumpLineBar : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit JumpLineBar(DFloatingWidget *parent = 0);
    ~JumpLineBar();

public slots:
    void focus();
    bool isFocus();
    void activeInput(QString file, int row, int column, int lineCount, int scrollOffset);
    void handleFocusOut();
    void handleLineChanged();
    void jumpCancel();
    void jumpConfirm();
    void slotFocusChanged(bool bFocus);
    void hide();
    int getLineCount();

Q_SIGNALS:
    void backToPosition(QString file, int row, int column, int scrollOffset);
    void jumpToLine(QString file, int line, bool focusEditor);
    void lostFocusExit();
    void pressEsc();

protected:
    bool eventFilter(QObject *pObject, QEvent *pEvent);
    Q_SLOT void updateSizeMode();

private:
    DSpinBox *m_pSpinBoxInput {nullptr};
    QHBoxLayout *m_layout {nullptr};
    QLabel *m_label {nullptr};
    QString m_jumpFile;
    int m_jumpFileScrollOffset;
    int m_rowBeforeJump;
    int m_columnBeforeJump;
    int m_lineCount;
    QColor m_backgroundColor;
    DIconButton *m_closeButton=nullptr;
};

#endif
