/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULEDATA_H
#define KCMODULEDATA_H

#include <QObject>
#include <QVariantList>
#include <kcmutilscore_export.h>
#include <memory>

class KCModuleDataPrivate;
class KCoreConfigSkeleton;

/**
 * @short A base class that offers information about a KCModule state
 *
 * @author Benjamin Port <benjamin.port@enioka.com>
 *
 * @since 5.74
 */
class KCMUTILSCORE_EXPORT KCModuleData : public QObject
{
    Q_OBJECT
public:
    explicit KCModuleData(QObject *parent = nullptr);
    ~KCModuleData() override;

    /**
     * Checks if the configuration is identical to the default one.
     *
     * @return @c true if the module configuration is in the default state, @c false otherwise
     */
    virtual bool isDefaults() const;

    /**
     * Revert module to default values and save them.
     */
    virtual void revertToDefaults();

    /**
     * Checks if this module matches a given query.
     * @param query the text user search for, it is not expected to be a regex pattern but a full text search.
     * @return @c true if this module matches a given query, @c false otherwise
     */
    virtual bool matchesQuery(const QString &query) const;

Q_SIGNALS:
    /**
     * This signal is emitted when KCModuleData is loaded.
     */
    void loaded();

    /**
     * Internal use
     *
     * Triggers the emit of @see loaded() signal. This is the default behavior.
     * To handle when loaded() is emitted in subclass, disconnect this signal in derived constructor.
     */
    void aboutToLoad(QPrivateSignal);

protected Q_SLOTS:
    /**
     * Allow to register manually skeleton class.
     * Used by derived class when automatic discovery is not possible.
     */
    void registerSkeleton(KCoreConfigSkeleton *skeleton);

    /**
     * Automatically register child skeletons
     * Call it in your subclass constructor after skeleton creation
     */
    void autoRegisterSkeletons();

private:
    const std::unique_ptr<KCModuleDataPrivate> d;
    friend class KCModuleDataPrivate;
};

#endif
