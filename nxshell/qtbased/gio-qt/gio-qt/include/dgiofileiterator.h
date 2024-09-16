// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOFILEITERATOR_H
#define DGIOFILEITERATOR_H

#include <QObject>
#include <QSharedData>

namespace Gio {
class FileEnumerator;
}

enum DGioIOPriority
{
  /*! Use this for high priority event sources.  It is not used within
   * GLib or GTK+.<br><br>
   */
  PRIORITY_HIGH = -100,

  /*! Use this for default priority event sources.  In glibmm this
   * priority is used by default when installing timeout handlers with
   * SignalTimeout::connect().  In GDK this priority is used for events
   * from the X server.<br><br>
   */
  PRIORITY_DEFAULT = 0,

  /*! Use this for high priority idle functions.  GTK+ uses
   * <tt>PRIORITY_HIGH_IDLE&nbsp;+&nbsp;10</tt> for resizing operations, and
   * <tt>PRIORITY_HIGH_IDLE&nbsp;+&nbsp;20</tt> for redrawing operations.
   * (This is done to ensure that any pending resizes are processed before
   * any pending redraws, so that widgets are not redrawn twice unnecessarily.)
   * <br><br>
   */
  PRIORITY_HIGH_IDLE = 100,

  /*! Use this for default priority idle functions.  In glibmm this priority is
   * used by default when installing idle handlers with SignalIdle::connect().
   * <br><br>
   */
  PRIORITY_DEFAULT_IDLE = 200,

  /*! Use this for very low priority background tasks.  It is not used within
   * GLib or GTK+.
   */
  PRIORITY_LOW = 300
};

Q_ENUMS(DGioIOPriority);

class DGioFileInfo;
class DGioFileIteratorPrivate;
class DGioFileIterator : public QObject, public QSharedData
{
    Q_OBJECT
public:
    explicit DGioFileIterator(Gio::FileEnumerator *gmmFileEnumeratorPtr, QObject *parent = nullptr);
    ~DGioFileIterator();

    QExplicitlySharedDataPointer<DGioFileInfo> nextFile();
    void nextFilesAsync(int numberOfFiles = 1, DGioIOPriority io_priority = PRIORITY_DEFAULT);

Q_SIGNALS:
    void nextFilesReady(QList<QExplicitlySharedDataPointer<DGioFileInfo> > fileInfoList);
    void nextFilesCancelled();

private:
    QScopedPointer<DGioFileIteratorPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DGioFileIterator)
};

#endif // DGIOFILEITERATOR_H
