/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "path-edit.h"
#include "path-bar-model.h"
#include "path-completer.h"
#include "file-utils.h"

#include <QKeyEvent>
#include <QAction>
#include <QUrl>
#include <QAbstractItemView>
#include <QVector4D>

#include <QDebug>

using namespace Peony;

PathEdit::PathEdit(QWidget *parent) : QLineEdit(parent)
{
    setContentsMargins(1, 0, 0, 0);
    setFocusPolicy(Qt::ClickFocus);

    m_model = new PathBarModel(this);
    m_completer = new PathCompleter(this);
    m_completer->setModel(m_model);
    //m_completer->setCompletionMode(QCompleter::CompletionMode::UnfilteredPopupCompletion);

    m_completer->popup()->setAttribute(Qt::WA_TranslucentBackground);

    m_completer->popup()->setProperty("useCustomShadow", true);
    m_completer->popup()->setProperty("customShadowDarkness", 0.5);
    m_completer->popup()->setProperty("customShadowWidth", 20);
    m_completer->popup()->setProperty("customShadowRadius", QVector4D(6, 6, 6, 6));
    m_completer->popup()->setProperty("customShadowMargins", QVector4D(20, 20, 20, 20));

    m_completer->setCaseSensitivity(Qt::CaseInsensitive);

    setLayoutDirection(Qt::LeftToRight);

    QAction *goToAction = new QAction(QIcon::fromTheme("lingmo-end-symbolic", QIcon(":/icons/lingmo-end-symbolic")), tr("Go To"), this);
    addAction(goToAction, QLineEdit::TrailingPosition);
    goToAction->setProperty("isWindowButton", 1);
    goToAction->setProperty("useIconHighlightEffect", 2);
    goToAction->setProperty("isIcon", true);

    connect(goToAction, &QAction::triggered, this, &QLineEdit::returnPressed);

    setCompleter(m_completer);

    connect(this, &QLineEdit::returnPressed, [=] {
        if (this->text().isEmpty()) {
            this->setText(m_last_uri);
            this->editCancelled();
            return;
        } else {
            Q_EMIT this->uriChangeRequest(this->text());
            //NOTE: we have send the signal for location change.
            //so we can use editCancelled hide the path edit.
            this->editCancelled();
        }
    });

    connect(this, &PathEdit::textChanged, this, [=](){
        if (!isVisible())
            return;

        auto uri = text();
        if (uri.endsWith("/")) {
            m_model->setRootUri(uri);
        }
//        auto parentUri = FileUtils::getParentUri(uri);
//        if (parentUri == m_model->currentDirUri() && !m_model->stringList().isEmpty())
//            return;

//        if (uri.endsWith("/")) {
//            //m_model->setRootUri(uri);
//        } else {
//            m_model->setRootUri(parentUri);
//        }
    });

    connect(m_model, &PathBarModel::updated, this, [=](){
        if (m_model->stringList().isEmpty())
            return;

        m_completer->complete();
    });
}

void PathEdit::setUri(const QString &uri)
{
    m_last_uri = uri;
    setText(QUrl::fromPercentEncoding(m_last_uri.toLocal8Bit()));
}

void PathEdit::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    if (! m_right_click) {
        Q_EMIT editCancelled();
    }
    m_right_click = false;
}

void PathEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
}

void PathEdit::cancelList()
{
    m_completer->activated(m_completer->currentIndex());
}

void PathEdit::keyPressEvent(QKeyEvent *e)
{
    QLineEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Escape) {
        Q_EMIT editCancelled();
    }
}

void PathEdit::mousePressEvent(QMouseEvent *e)
{
    QLineEdit::mousePressEvent(e);
    if (e->button() == Qt::RightButton)
        m_right_click = true;
    else
        m_right_click = false;
}
