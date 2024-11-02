#include "searchedit.h"
#include "UI/base/xatom-helper.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include "UIControl/global/global.h"

SearchEdit::SearchEdit(QWidget *parent) : KSearchLineEdit(parent)
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setCompleter(nullptr); // kdk自带记忆功能，取消搜索记忆功能

    connect(this, &SearchEdit::textChanged,this,&SearchEdit::slotTextChanged);
    connect(this, &SearchEdit::returnPressed,this,&SearchEdit::slotReturnPressed);
//    connect(this, &SearchEdit::editingFinished,this,&SearchEdit::slotEditingFinished);

    m_result = new SearchResult(m_mainWidget);
    m_result->setSearchEdit(this);

    if (Global::isWayland) {
        kdk::LingmoUIStyleHelper::self()->removeHeader(this);

    } else {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(m_result->winId(), hints);
    }

//    moveSearchResult();
}

void SearchEdit::keyPressEvent(QKeyEvent *event)
{
    if(m_result == nullptr) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_Up:
        m_result->selectUp();
        break;
    case Qt::Key_Down:
        m_result->selectDown();
        break;
    case Qt::Key_Escape:
        m_result->hide();
    default:
        break;
    }

    QLineEdit::keyPressEvent(event);
}

void SearchEdit::focusInEvent(QFocusEvent *event)
{
    this->setFocus();
    slotTextChanged();
    return QLineEdit::focusInEvent(event);
}

void SearchEdit::focusOutEvent(QFocusEvent *event)
{
    this->clearFocus();

    return QLineEdit::focusOutEvent(event);
}

void SearchEdit::slotTextChanged()
{
    if (m_result == nullptr) {
        return;
    }

    QString alltext = this->text();
    if (alltext.isEmpty()) {
        m_result->hide();
        return;
    }/* else {
        m_result->show();
        m_result->raise();
    }*/

    this->setCursorPosition(this->cursorPosition());

    auto text = this->text().trimmed();

    if (this->text().size() == 0) {
        m_result->hide();
        return;
    }
    if (text.length() >= 1) {
        m_result->setListviewSearchString(text);

        m_result->autoResize();
        moveSearchResult();
//        m_result->activateWindow();
//        m_result->setFocusPolicy(Qt::StrongFocus);
    } else {
        m_result->hide();
    }
}

void SearchEdit::slotReturnPressed()
{
    if (m_result == nullptr) {
        return;
    }

    if (m_result->getCurrentIndex() >= 0) {
        m_result->onReturnPressed();
        m_result->hide();
        return;
    }

    auto text = QString(this->text().trimmed());
    if(text.length() == 0)
        return;
    m_result->hide();
    Q_EMIT signalReturnPressed(tr("Search Result"));
    Q_EMIT signalReturnText(text);
}

void SearchEdit::setWidget(QWidget *mainWidget)
{
    m_mainWidget = mainWidget;
}

void SearchEdit::moveSearchResult()
{
    kdk::LingmoUIStyleHelper::self()->removeHeader(m_result);
    m_result->show();
    kdk::WindowManager::setSkipSwitcher(m_result->windowHandle(),true);
    kdk::WindowManager::setSkipTaskBar(m_result->windowHandle(),true);
//    m_result->raise();
    m_result->setFixedWidth(240);

    QPoint resultPos = this->mapToGlobal(this->rect().bottomLeft());
    resultPos.setX(resultPos.x());
    resultPos.setY(resultPos.y() + 12);

    int newPosX = resultPos.x();
    QSize editSize = this->size();
    int newPosY = resultPos.y() - editSize.height() - 12;

    m_result->changeSrearchResultPos(newPosX, newPosY, editSize.width(), editSize.height());

    if (Global::isWayland) {
        kdk::WindowManager::setGeometry(m_result->windowHandle(), QRect(newPosX, newPosY + 12 + editSize.height() *2 , editSize.width(), editSize.height()));
    } else {
        m_result->move(resultPos);
        kdk::WindowManager::setGeometry(m_result->windowHandle(),m_result->geometry());
    }

}

