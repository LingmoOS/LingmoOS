/*
    SPDX-FileCopyrightText: 2023 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_TRANSACTION_H
#define KNEWSTUFF3_TRANSACTION_H

#include <QObject>
#include <memory>

#include "entry.h"
#include "errorcode.h"

#include "knewstuffcore_export.h"

namespace KNSCore
{
class EngineBase;
class TransactionPrivate;

/**
 * KNewStuff Transaction
 *
 * Exposes different actions that can be done on an entry and means to track them to completion.
 *
 * To create a Transaction we should call one of the static methods that
 * represent the different actions we can take. These will return the Transaction
 * and we can use it to track mesages, the entries' states and eventually its
 * completion using the @m finished signal.
 *
 * The Transaction will delete itself once it has finished.
 *
 * @since 6.0
 */
class KNEWSTUFFCORE_EXPORT Transaction : public QObject
{
    Q_OBJECT
public:
    ~Transaction() override;

    /**
     * Performs an install on the given @p entry from the @p engine.
     *
     * @param linkId specifies which of the assets we want to see installed.
     * @returns a Transaction object that we can use to track the progress to completion
     */
    static Transaction *install(EngineBase *engine, const Entry &entry, int linkId = 1);

    /**
     * Uninstalls the given @p entry from the @p engine.
     *
     * It reverses the step done when @m install was called.
     * @returns a Transaction object that we can use to track the progress to completion
     */
    static Transaction *uninstall(EngineBase *engine, const Entry &entry);

    /**
     * Adopt the @p entry from @p engine using the adoption command.
     *
     * For more information, see the documentation about AdoptionCommand from
     * the knsrc file.
     */
    static Transaction *adopt(EngineBase *engine, const Entry &entry);

    /**
     * @returns true as soon as the Transaction is completed as it gets ready to
     * clean itself up
     */
    bool isFinished() const;

Q_SIGNALS:
    void finished();

    /**
     * Provides the @p message to update our users about how the Transaction progressed
     */
    void signalMessage(const QString &message);

    /**
     * Informs about how the @p entry has changed
     *
     * @param event nature of the change
     */
    void signalEntryEvent(const KNSCore::Entry &entry, KNSCore::Entry::EntryEvent event);

    /**
     * Fires in the case of any critical or serious errors, such as network or API problems.
     * @param errorCode Represents the specific type of error which has occurred
     * @param message A human-readable message which can be shown to the end user
     * @param metadata Any additional data which might be hepful to further work out the details of the error (see KNSCore::Entry::ErrorCode for the
     * metadata details)
     * @see KNSCore::Entry::ErrorCode
     */
    void signalErrorCode(KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata);

private:
    Transaction(const KNSCore::Entry &entry, EngineBase *engine);
    void downloadLinkLoaded(const KNSCore::Entry &entry);

    std::unique_ptr<TransactionPrivate> d;
};

}

#endif
