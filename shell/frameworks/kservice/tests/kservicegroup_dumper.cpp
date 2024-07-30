/*
    SPDX-FileCopyrightText: 2002-2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QCoreApplication>
#include <QDebug>
#include <kservice.h>
#include <kservicegroup.h>

int main(int argc, char *argv[])
{
    QCoreApplication k(argc, argv);

    KServiceGroup::Ptr root = KServiceGroup::root();
    KServiceGroup::List list = root->entries();

    KServiceGroup::Ptr first;

    qDebug("Found %" PRIdQSIZETYPE " entries", list.count());
    for (const KSycocaEntry::Ptr &p : std::as_const(list)) {
        if (p->isType(KST_KService)) {
            KService::Ptr service(static_cast<KService *>(p.data()));
            qDebug("%s", qPrintable(service->name()));
            qDebug("%s", qPrintable(service->entryPath()));
        } else if (p->isType(KST_KServiceGroup)) {
            KServiceGroup::Ptr serviceGroup(static_cast<KServiceGroup *>(p.data()));
            qDebug("             %s -->", qPrintable(serviceGroup->caption()));
            if (!first) {
                first = serviceGroup;
            }
        } else {
            qDebug("KServiceGroup: Unexpected object in list!");
        }
    }

    if (first) {
        list = first->entries();
        qDebug("Found %" PRIdQSIZETYPE " entries", list.count());
        for (const KSycocaEntry::Ptr &p : std::as_const(list)) {
            if (p->isType(KST_KService)) {
                KService::Ptr service(static_cast<KService *>(p.data()));
                qDebug("             %s", qPrintable(service->name()));
            } else if (p->isType(KST_KServiceGroup)) {
                KServiceGroup::Ptr serviceGroup(static_cast<KServiceGroup *>(p.data()));
                qDebug("             %s -->", qPrintable(serviceGroup->caption()));
            } else {
                qDebug("KServiceGroup: Unexpected object in list!");
            }
        }
    }

    return 0;
}
