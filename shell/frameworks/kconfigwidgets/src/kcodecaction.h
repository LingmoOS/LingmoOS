/*
    SPDX-FileCopyrightText: 2003 Jason Keirstead <jason@keirstead.org>
    SPDX-FileCopyrightText: 2003-2006 Michel Hermier <michel.hermier@gmail.com>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCODECACTION_H
#define KCODECACTION_H

#include <kconfigwidgets_export.h>

#include <KSelectAction>
#include <memory>

/**
 *  @class KCodecAction kcodecaction.h KCodecAction
 *
 *  @short Action for selecting one of several text codecs..
 *
 *  This action shows up a submenu with a list of the available codecs on the system.
 */
class KCONFIGWIDGETS_EXPORT KCodecAction : public KSelectAction
{
    Q_OBJECT

    Q_PROPERTY(QString codecName READ currentCodecName WRITE setCurrentCodec)

public:
    explicit KCodecAction(QObject *parent, bool showAutoOptions = false);

    KCodecAction(const QString &text, QObject *parent, bool showAutoOptions = false);

    KCodecAction(const QIcon &icon, const QString &text, QObject *parent, bool showAutoOptions = false);

    ~KCodecAction() override;

public:
    QString currentCodecName() const;
    bool setCurrentCodec(const QString &codecName);

Q_SIGNALS:
    /**
     * Emitted when a codec was selected
     *
     * @param name the name of the selected encoding.
     *
     * Note that textTriggered(const QString &) is emitted too (as defined in KSelectAction).
     *
     * @since 5.103
     */
    void codecNameTriggered(const QByteArray &name);

    /**
     * Emitted when the 'Default' codec action is triggered.
     */
    void defaultItemTriggered();

protected Q_SLOTS:
    void slotActionTriggered(QAction *) override;

protected:
    using KSelectAction::actionTriggered;

private:
    friend class KCodecActionPrivate;
    std::unique_ptr<class KCodecActionPrivate> const d;
};

#endif
