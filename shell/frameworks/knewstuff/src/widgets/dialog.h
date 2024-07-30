/*
    SPDX-FileCopyrightText: 2020-2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF_QTQUICKDIALOGWRAPPER_H
#define KNEWSTUFF_QTQUICKDIALOGWRAPPER_H

#include <KNSCore/Entry>
#include <KNSCore/ErrorCode>
#include <QDialog>

#include "knewstuffwidgets_export.h"

namespace KNSCore
{
class EngineBase;
};

namespace KNSWidgets
{
class DialogPrivate;
/**
 * @class Dialog dialog.h <KNSWidgets/Dialog>
 *
 * This class is a wrapper around the QtQuick QML dialog. This dialog content is loaded QQuickWidget.
 * It is recommended to reuse an instance of this class if it is expected that the user reopens the dialog.
 * For most usecases, you should use KNSWidgets::Button or KNSWidgets::Action directly.
 *
 * @since 6.0
 */
class KNEWSTUFFWIDGETS_EXPORT Dialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructs a new Dialog for the given config file and parent widget
     */
    explicit Dialog(const QString &configFile, QWidget *parent = nullptr);
    ~Dialog() override;

    /**
     * Engine that is used by the dialog, might be null if the engine failed to initialize.
     * @return KNSCore::EngineBase used by the dialog
     */
    KNSCore::EngineBase *engine();

    /**
     * Entries that were changed while the user interacted with the dialog
     * @since 5.94
     */
    QList<KNSCore::Entry> changedEntries() const;

    void open() override;

private:
    Q_PRIVATE_SLOT(d, void onEntryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event))
    const std::unique_ptr<DialogPrivate> d;

    Q_DISABLE_COPY(Dialog)
};
}

#endif // KNEWSTUFF_QTQUICKDIALOGWRAPPER_H
