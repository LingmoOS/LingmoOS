// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>
#include <QCoreApplication>
#include <QExplicitlySharedDataPointer>

#include <dglibutils.h>
#include <dgiofile.h>
#include <dgiomount.h>
#include <dgiovolume.h>
#include <dgiodrive.h>
#include <dgiovolumemanager.h>
#include <dgiofileinfo.h>
#include <dgiofileiterator.h>

int main(int argc, char * argv[])
{
    qDebug() << DGlibUtils::systemDataDirs();

    qDebug() << "----------------------";

    DGioFile *networkFile = DGioFile::createFromUri("network:///");
    if (networkFile) {
        QExplicitlySharedDataPointer<DGioFileIterator> iter = networkFile->createFileIterator("standard::*,mountable::can-mount");
        if (iter) {
            while (QExplicitlySharedDataPointer<DGioFileInfo> fi = iter->nextFile()) {
                if (fi) {
                    qDebug() << fi->displayName() << fi->fileType();
                }
            }
        }
        delete networkFile;
    }

//    // Can't do asynchronous next_files() on a file enumerator created synchronously
//    DGioFile *recentFile1 = DGioFile::createFromUri("recent:///");
//    QExplicitlySharedDataPointer<DGioFileIterator> iter1;
//    if (recentFile1) {
//        iter1 = recentFile1->createFileIterator("standard::*");
//        if (iter1) {
//            iter1->nextFilesAsync(5);
//            QObject::connect(iter1.data(), &DGioFileIterator::nextFilesReady, [](QList<QExplicitlySharedDataPointer<DGioFileInfo> > fileInfoList){
//                for (auto fi : fileInfoList) {
//                    qDebug() << "xxxxxx" << fi->displayName() << fi->fileType();
//                }
//            });
//        }
//    }

    DGioFile *recentFile = DGioFile::createFromUri("recent:///");
    QExplicitlySharedDataPointer<DGioFileIterator> iter;
    if (recentFile) {
        recentFile->createFileIteratorAsync("standard::*");
        QObject::connect(recentFile, &DGioFile::createFileIteratorReady, [&iter](QExplicitlySharedDataPointer<DGioFileIterator> iterr){
            iter = iterr;
            if (iter) {
                iter->nextFilesAsync(5);
                QObject::connect(iter.data(), &DGioFileIterator::nextFilesReady, [](QList<QExplicitlySharedDataPointer<DGioFileInfo> > fileInfoList){
                    for (auto fi : fileInfoList) {
                        qDebug() << "under recent:" << fi->displayName() << fi->fileType();
                    }
                });
            }
        });
    }

    qDebug() << "----------------------";

    DGioFile * f = DGioFile::createFromPath("/media/wzc/_dde_data");
    if (f) {
        qDebug() << f->basename() << f->path() << f->uri();
        QExplicitlySharedDataPointer<DGioFileInfo> fi = f->createFileSystemInfo();
        if (fi) {
            qDebug() << fi->fsFreeBytes() << fi->fsUsedBytes() << fi->fsTotalBytes() << fi->fileType() << fi->displayName();
        }
        delete f;
    }

    qDebug() << "----------------------";

    DGioMount * m = DGioMount::createFromPath("/media/wzc/_dde_data");
    if (m) {
        QExplicitlySharedDataPointer<DGioFile> f = m->getRootFile();
        QExplicitlySharedDataPointer<DGioFile> f2 = m->getDefaultLocationFile();
        qDebug() << m->name() << m->themedIconNames() << f->createFileSystemInfo()->fsTotalBytes() << f->uri() << f2->uri();
        qDebug() << m->name() << m->themedIconNames() << f->createFileSystemInfo()->fsTotalBytes() << f->uri() << f2->uri();
//        m->unmount();
        delete m;
    }

    qDebug() << "---------mounts-------------";

    const QList<QExplicitlySharedDataPointer<DGioMount> > mnts = DGioVolumeManager::getMounts();

    for (const QExplicitlySharedDataPointer<DGioMount> &p : mnts) {
        QExplicitlySharedDataPointer<DGioFile> f = p->getRootFile();
        QExplicitlySharedDataPointer<DGioFile> f2 = p->getDefaultLocationFile();
        qDebug() << f->uri() << f2->uri() << f->path() << f2->path();
        qDebug() << f->uri() << f2->uri();
        qDebug() << p->name() << p->uuid() << p->mountClass() << p->canUnmount() << p->themedIconNames() << p->themedIconNames();
    }

    qDebug() << "--------volumes--------------";

    const QList<QExplicitlySharedDataPointer<DGioVolume> > vols = DGioVolumeManager::getVolumes();

    for (const QExplicitlySharedDataPointer<DGioVolume> &p : vols) {
        qDebug() << p->name() << p->volumeClass() << p->volumeMonitorName()
                 << p->identifier(DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_UUID)
                 << p->identifier(DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_CLASS)
                 << p->identifier(DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_LABEL)
                 << p->identifier(DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_NFS_MOUNT)
                 << p->identifier(DGioVolumeIdentifierType::VOLUME_IDENTIFIER_TYPE_UNIX_DEVICE);
    }

    qDebug() << "----------drives------------";

    const QList<QExplicitlySharedDataPointer<DGioDrive> > drvs = DGioVolumeManager::getDrives();

    for (const QExplicitlySharedDataPointer<DGioDrive> &p : drvs) {
        qDebug() << p->name() << p->isRemovable() << p->isMediaRemovable();
    }

    qDebug() << "----------------------";

    QCoreApplication app(argc, argv);

    DGioVolumeManager vm;

    QObject::connect(&vm, &DGioVolumeManager::mountAdded, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountAdded" << mnt->name();
    });

    QObject::connect(&vm, &DGioVolumeManager::mountPreRemoved, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountPreRemoved" << mnt->name();
    });

    QObject::connect(&vm, &DGioVolumeManager::mountRemoved, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountRemoved" << mnt->name();
    });

    QObject::connect(&vm, &DGioVolumeManager::mountChanged, [](QExplicitlySharedDataPointer<DGioMount> mnt){
        qDebug() << "MountChanged" << mnt->name();
    });

    app.exec();

    return 0;
}
