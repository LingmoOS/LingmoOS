#ifndef SEARCHEDIT_H
#define SEARCHEDIT_H

#include <QLineEdit>
#include <QFocusEvent>
#include <QDebug>
#include "UI/player/searchresult.h"

//#include "kwidget.h"
#include "ksearchlineedit.h"

using namespace  kdk;

class SearchResult;

class SearchEdit : public KSearchLineEdit
{
    Q_OBJECT

public:
    SearchEdit(QWidget *parent = nullptr);

public:
    void setWidget(QWidget *mainWidget);
    void moveSearchResult();

Q_SIGNALS:
    void sigFoucusIn();
    //发送展示搜索结果界面
    void signalReturnPressed(QString searchEdit);
    //搜索的关键字
    void signalReturnText(QString text);

protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);

public Q_SLOTS:
    void slotTextChanged();
    void slotReturnPressed();

public:
    SearchResult *m_result = nullptr;

private:

    QWidget *m_mainWidget = nullptr;
};

#endif // SEARCHEDIT_H
