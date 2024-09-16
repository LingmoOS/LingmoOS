#include "optionsmanager.h"

OptionsManager::OptionsManager() : QObject() { }

OptionsManager::~OptionsManager() { }

OptionsManager *OptionsManager::instance()
{
    static OptionsManager ins;
    return &ins;
}

QMap<QString, QStringList> OptionsManager::getUserOptions() const
{
    return userOptions;
}

QStringList OptionsManager::getUserOption(const QString &option) const
{
    if (userOptions.contains(option))
        return userOptions[option];
    else
        return QStringList();
}

void OptionsManager::setUserOptions(const QMap<QString, QStringList> &value)
{
    userOptions = value;
}

void OptionsManager::addUserOption(const QString &option, const QStringList &value)
{
    // update user options
    if (userOptions.contains(option))
        userOptions.remove(option);
    userOptions[option] = value;
}

void OptionsManager::clear()
{
    userOptions.clear();
}
