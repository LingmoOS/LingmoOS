// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfiledrag.h"
#include "dfiledragserver.h"
#include "private/dfiledragcommon_p.h"
#include "private/dfiledragserver_p.h"

#include <DObjectPrivate>

#include <QVariant>

DGUI_BEGIN_NAMESPACE

/*!
 @private
 */
class DFileDragPrivate : DCORE_NAMESPACE::DObjectPrivate
{
    DFileDragPrivate(DFileDrag *q, DFileDragServer *_srv)
        : DObjectPrivate(q)
        , srv(_srv) {}
    DFileDragServer *srv;

    D_DECLARE_PUBLIC(DFileDrag)
};

/*!
  \class Dtk::Gui::DFileDrag
  \inmodule dtkgui
  \brief 继承自QDrag，一般在文件拖拽发送方 mouseMoveEvent 中发起拖拽，设置发送数据 和 DFileDragServer 配合使用.

  \sa Dtk::Gui::DFileDragServer
 */

/*!
  \fn void DFileDrag::targetUrlChanged(QUrl url)

  \brief 信号会在接收方调用 setTargetUrl 时被发送.
  可以用于获取被拖拽至的目标目录

  \a url 发生改变的目标链接.
 */

DFileDrag::DFileDrag(QObject *source, DFileDragServer *server)
    : QDrag(source)
    , DObject(*new DFileDragPrivate(this, server))
{
    Q_D(DFileDrag);

    connect(d->srv, &DFileDragServer::targetDataChanged, this, [this, d](const QString &key) {
        if (key == DND_TARGET_URL_KEY) {
            Q_EMIT this->targetUrlChanged(QUrl(d->srv->targetData(key).value<QString>()));
        }
    });
}

/*!
  \brief DFileDrag::targetUrl
  \return 返回拖拽文件接收方设置的接收路径
 */
QUrl DFileDrag::targetUrl()
{
    Q_D(DFileDrag);

    return QUrl(d->srv->targetData(DND_TARGET_URL_KEY).value<QString>());
}

/*!
  \brief DFileDrag::setMimeData
  \a data
  \brief 发起拖拽文件前设置发送数据接口
 */
void DFileDrag::setMimeData(QMimeData *data)
{
    Q_D(DFileDrag);

    d->srv->d_func()->writeMimeData(data);
    QDrag::setMimeData(data);
}

DGUI_END_NAMESPACE
