/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Christian Ehrlicher <ch.ehrlicher@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMEMFILE_H
#define KMEMFILE_H

#include <QIODevice>
#include <kservice_export.h>
#include <memory>

#ifndef QT_NO_SHAREDMEMORY

/**
 * @internal
 * Simple QIODevice for QSharedMemory to keep ksycoca cache in memory only once
 * The first call to open() loads the file into a shm segment. Every
 * subsequent call only attaches to this segment. When the file content changed,
 * you have to execute KMemFile::fileContentsChanged() to update the internal
 * structures. The next call to open() creates a new shm segment. The old one
 * is automatically destroyed when the last process closed KMemFile.
 */

class KMemFile : public QIODevice
{
    Q_OBJECT
public:
    /**
     * ctor
     *
     * @param filename the file to load into memory
     * @param parent our parent
     */
    explicit KMemFile(const QString &filename, QObject *parent = nullptr);
    /**
     * dtor
     */
    ~KMemFile() override;
    /**
     * closes the KMemFile
     *
     * @reimp
     */
    void close() override;
    /**
     * As KMemFile is a random access device, it returns false
     *
     * @reimp
     */
    bool isSequential() const override;
    /**
     * @reimp
     * @param mode only QIODevice::ReadOnly is accepted
     */
    bool open(OpenMode mode) override;
    /**
     * Sets the current read/write position to pos
     * @reimp
     * @param pos the new read/write position
     */
    bool seek(qint64 pos) override;
    /**
     * Returns the size of the file
     * @reimp
     */
    qint64 size() const override;
    /**
     * This static function updates the internal information about the file
     * loaded into shared memory. The next time the file is opened, the file is
     * reread from the file system.
     */
    static void fileContentsChanged(const QString &filename);

protected:
    /** @reimp */
    qint64 readData(char *data, qint64 maxSize) override;
    /** @reimp */
    qint64 writeData(const char *data, qint64 maxSize) override;

private:
    class Private;
    friend class Private;
    std::unique_ptr<Private> const d;
};

#endif // QT_NO_SHAREDMEMORY

#endif // KMEMFILE_H
