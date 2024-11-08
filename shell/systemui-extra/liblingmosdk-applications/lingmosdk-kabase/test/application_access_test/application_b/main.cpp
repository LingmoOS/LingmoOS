/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include <QList>
#include <QVariant>
#include <QDebug>

#include "../../../kabase/application_access.hpp"

#include "fun.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    kdk::kabase::ApplicationAccess applicationAccess;

    Fun fun;
    applicationAccess.exportFunc(kdk::kabase::LingmoIpmsg, &fun);

    QList<QVariant> args;
    args << 2 << 4;

    QList<QVariant> ret = applicationAccess.callMethod(kdk::kabase::LingmoFontViewer, "add", args);
    if (!ret.isEmpty()) {
        qInfo() << "call success ret is " << ret.at(0).toInt();
    }

    return app.exec();
}
