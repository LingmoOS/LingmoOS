/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_UI_DIALOG_DSL_H
#define PLASMAVAULT_KDED_UI_DIALOG_DSL_H

#include <QHash>
#include <QString>
#include <QVariant>
#include <QWidget>

#include <functional>

#include <vault.h>

namespace DialogDsl
{
// We want to have a normal ID for the QMap or QHash,
// but we also want it to have a translated name
class Key : public QByteArray
{
public:
    Key(const QByteArray &id, const QString &translation = QString());

    QString translation() const;

private:
    QString m_translation;
};

namespace operators
{
// A nicer way to define a Key and its translation
inline Key operator/(const char *id, const QString &name)
{
    return Key(id, name);
}
}

// A dialog module can return a set of configured key-value pairs
class DialogModule : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool isValid READ isValid WRITE setIsValid NOTIFY isValidChanged)

public:
    DialogModule(bool isValid);

    virtual PlasmaVault::Vault::Payload fields() const = 0;
    virtual void init(const PlasmaVault::Vault::Payload &payload);

    virtual void aboutToBeShown();
    virtual bool shouldBeShown() const;
    virtual void aboutToBeHidden();

    bool isValid() const;
    void setIsValid(bool valid);

Q_SIGNALS:
    void isValidChanged(bool valid);
    void requestCancellation();

private:
    bool m_isValid;
};

typedef std::function<DialogModule *()> ModuleFactory;

class step : public QList<ModuleFactory>
{
public:
    step() = default;

    step(const std::initializer_list<ModuleFactory> &modules)
        : QList<ModuleFactory>(modules)
    {
    }

    friend step operator/(const QString &title, step &&from)
    {
        step result(std::move(from));
        result.m_title = title;
        return result;
    }

    QString title() const
    {
        return m_title;
    }

private:
    QString m_title;
};

typedef QList<step> steps;
typedef QMap<Key, steps> Logic;

// If we want to have a single page containing multiple modules
class CompoundDialogModule : public DialogModule
{
public:
    CompoundDialogModule(const step &children);

    PlasmaVault::Vault::Payload fields() const override;
    void init(const PlasmaVault::Vault::Payload &payload) override;

private:
    QList<DialogModule *> m_children;
    QSet<DialogModule *> m_invalidChildren;
};

} // namespace DialogDsl

#endif // include guard
