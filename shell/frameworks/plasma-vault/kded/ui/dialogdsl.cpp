/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "dialogdsl.h"

#include <QVBoxLayout>

namespace DialogDsl
{
Key::Key(const QByteArray &key, const QString &translation)
    : QByteArray(key)
    , m_translation(translation)
{
}

QString Key::translation() const
{
    return m_translation;
}

DialogModule::DialogModule(bool valid)
    : m_isValid(valid)
{
}

void DialogModule::setIsValid(bool valid)
{
    if (valid == m_isValid)
        return;

    m_isValid = valid;

    Q_EMIT isValidChanged(valid);
}

bool DialogModule::shouldBeShown() const
{
    return true;
}

void DialogModule::aboutToBeShown()
{
}

void DialogModule::aboutToBeHidden()
{
}

bool DialogModule::isValid() const
{
    return m_isValid;
}

void DialogModule::init(const PlasmaVault::Vault::Payload &payload)
{
    Q_UNUSED(payload);
}

CompoundDialogModule::CompoundDialogModule(const step &children)
    : DialogDsl::DialogModule(false)
{
    auto layout = new QVBoxLayout(this);
    setLayout(layout);

    for (const auto &childFactory : children) {
        auto child = childFactory();
        child->setParent(this);
        child->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
        m_children << child;

        if (!child->isValid()) {
            m_invalidChildren << child;
        }

        connect(child, &DialogModule::isValidChanged, this, [this, child](bool valid) {
            if (valid) {
                m_invalidChildren.remove(child);
            } else {
                m_invalidChildren << child;
            }

            setIsValid(m_invalidChildren.isEmpty());
        });

        connect(child, &DialogModule::requestCancellation, this, &DialogModule::requestCancellation);

        layout->addWidget(child);
    }

    setIsValid(m_invalidChildren.isEmpty());

    auto spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addSpacerItem(spacer);
}

PlasmaVault::Vault::Payload CompoundDialogModule::fields() const
{
    PlasmaVault::Vault::Payload result;

    for (const auto &child : m_children) {
        result.insert(child->fields());
    }

    return result;
}

void CompoundDialogModule::init(const PlasmaVault::Vault::Payload &payload)
{
    for (const auto &child : m_children) {
        child->init(payload);
    }
}

} // namespace DialogDsl

#include "moc_dialogdsl.cpp"
