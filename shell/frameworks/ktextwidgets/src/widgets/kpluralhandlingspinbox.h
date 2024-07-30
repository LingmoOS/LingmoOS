/*
    SPDX-FileCopyrightText: 2014 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KPLURALHANDLINGSPINBOX_H
#define KPLURALHANDLINGSPINBOX_H

#include <ktextwidgets_export.h>

#include <KLocalizedString>

#include <QSpinBox>

#include <memory>

/**
 * @class KPluralHandlingSpinBox kpluralhandlingspinbox.h <KPluralHandlingSpinBox>
 *
 * @brief A QSpinBox with plural handling for the suffix.
 *
 * @author Laurent Montel <montel@kde.org>
 *
 * @since 5.0
 */
class KTEXTWIDGETS_EXPORT KPluralHandlingSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */

    explicit KPluralHandlingSpinBox(QWidget *parent = nullptr);
    ~KPluralHandlingSpinBox() override;

    /**
     * Sets the suffix to @p suffix.
     * Use this to add a plural-aware suffix, e.g. by using ki18np("singular", "plural").
     */
    void setSuffix(const KLocalizedString &suffix);

private:
    friend class KPluralHandlingSpinBoxPrivate;
    std::unique_ptr<class KPluralHandlingSpinBoxPrivate> const d;

    Q_DISABLE_COPY(KPluralHandlingSpinBox)
};

#endif // KPLURALHANDLINGSPINBOX_H
