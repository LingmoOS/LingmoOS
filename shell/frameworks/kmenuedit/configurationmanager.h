/*
 *   SPDX-FileCopyrightText: 2013 Julien Borderie <frajibe@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef configurationmanager_h
#define configurationmanager_h

#include <QMutex>
#include <QObject>

#include <KConfigGroup>

/**
 * @brief The ConfigurationManager class contains allows to access/edit the application configuration.
 */
class ConfigurationManager : public QObject
{
    Q_OBJECT

protected:
    static const QString GENERAL_CONFIG_GROUP;
    static const QString SHOW_HIDDEN_ENTRIES_PROPERTY_NAME;
    static const QString SPLITTER_SIZES_PROPERTY_NAME;

    static ConfigurationManager *m_instance;

    KConfigGroup m_configGroup;

public:
    /**
     * @brief Return the singleton instance.
     *
     * @return Instance.
     */
    static ConfigurationManager *getInstance()
    {
        static QMutex mutex;
        if (!m_instance) {
            mutex.lock();
            if (!m_instance) {
                m_instance = new ConfigurationManager();
            }
            mutex.unlock();
        }
        return m_instance;
    }

    /**
     * @brief Determine if hidden entries must be visible or not.
     *
     * @return True to show hidden entries, false else.
     */
    bool hiddenEntriesVisible() const;

    /**
     * @brief Replace the hidden entries visibility by the new one.
     *
     * @param visible True to show hidden entries, false else.
     */
    void setHiddenEntriesVisible(bool visible);

    /**
     * @brief Get the splitter sizes.
     *
     * @return Splitter sizes.
     */
    QList<int> getSplitterSizes() const;

    /**
     * @brief Replace splitter sizes by the new ones.
     *
     * @param sizes Splitter sizes.
     */
    void setSplitterSizes(const QList<int> &sizes);

protected:
    ConfigurationManager();
    ConfigurationManager(const ConfigurationManager &); // hide copy constructor
    ConfigurationManager &operator=(const ConfigurationManager &); // hide assign op
};

#endif
