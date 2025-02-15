/*
    SPDX-FileCopyrightText: 2003 Jason Keirstead <jason@keirstead.org>
    SPDX-FileCopyrightText: 2003-2006 Michel Hermier <michel.hermier@gmail.com>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCODECACTION_H
#define KCODECACTION_H

#include <kconfigwidgets_export.h>

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 102)
#include <KEncodingProber>
#endif

#include <KSelectAction>
#include <memory>

class QTextCodec;

/**
 *  @class KCodecAction kcodecaction.h KCodecAction
 *
 *  @short Action for selecting one of several QTextCodec.
 *
 *  This action shows up a submenu with a list of the available codecs on the system.
 */
class KCONFIGWIDGETS_EXPORT KCodecAction : public KSelectAction
{
    Q_OBJECT

    Q_PROPERTY(QString codecName READ currentCodecName WRITE setCurrentCodec)
#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 103)
    Q_PROPERTY(int codecMib READ currentCodecMib)
#endif

public:
    explicit KCodecAction(QObject *parent, bool showAutoOptions = false);

    KCodecAction(const QString &text, QObject *parent, bool showAutoOptions = false);

    KCodecAction(const QIcon &icon, const QString &text, QObject *parent, bool showAutoOptions = false);

    ~KCodecAction() override;

public:
#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 103)
    /**
     * @deprecated since 5.103, use QTextCodec or QStringConverter.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 103, "Use QTextCodec or QStringConverter")
    int mibForName(const QString &codecName, bool *ok = nullptr) const;
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 103)
    /**
     * @deprecated since 5.103, use QTextCodec or QStringConverter.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 103, "Use QTextCodec or QStringConverter")
    QTextCodec *codecForMib(int mib) const;
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 103)
    /**
     * @deprecated since 5.103, use currentCodecName
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 103, "Use currentCodecName")
    QTextCodec *currentCodec() const;
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 103)
    /**
     * @deprecated since 5.103, use setCurrentCodecName
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 103, "Use setCurrentCodecName")
    bool setCurrentCodec(QTextCodec *codec);
#endif

    QString currentCodecName() const;
    bool setCurrentCodec(const QString &codecName);

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 103)
    /**
     * @deprecated since 5.103, use currentCodecName
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 103, "Use currentCodecName")
    int currentCodecMib() const;
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 103)
    /**
     * @deprecated since 5.103, use setCurrentCodecName
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 103, "Use setCurrentCodecName")
    bool setCurrentCodec(int mib);
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 102)
    /**
     * Applicable only if showAutoOptions in c'tor was true
     *
     * @returns KEncodingProber::None if specific encoding is selected, not autodetection, otherwise... you know it!
     *
     * @deprecated since 5.102, no known users.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 102, "No known users")
    KEncodingProber::ProberType currentProberType() const;
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 102)
    /**
     * Applicable only if showAutoOptions in c'tor was true
     *
     * KEncodingProber::Universal means 'Default' item
     *
     * @deprecated since 5.102, no known users.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 102, "No known users")
    bool setCurrentProberType(KEncodingProber::ProberType);
#endif

Q_SIGNALS:
#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 78)
    /**
     * Specific (proper) codec was selected
     *
     * Note that triggered(const QString &) is emitted too (as defined in KSelectAction).
     * @deprecated Since 5.78, use codecTriggered(QTextCodec *)
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 78, "Use KCodecAction::codecTriggered(QTextCodec *)")
    void triggered(QTextCodec *codec); // clazy:exclude=overloaded-signal
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 103)
    /**
     * Specific (proper) codec was selected
     *
     * Note that textTriggered(const QString &) is emitted too (as defined in KSelectAction).
     *
     * In your KCodecAction subclass to be backward-compatible to KF < 5.78, emit instead
     * just the deprecated signal triggered(QTextCodec *). That will also automatically
     * emit codecTriggered(QTextCodec *) because this signal is connected to the deprevcated
     * one in the KCodecAction constructor.
     * Only if you compile against a variant of KConfigWidgets built without all API
     * deprecated up to version 5.78, then emit this signal directly.
     * Your code would be like this:
     * @code
     * #if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 78)
     *     // will also indirectly emit codecTriggered since 5.78
     *     Q_EMIT triggered(codec);
     * #else
     *     Q_EMIT codecTriggered(codec);
     * #endif
     * @endcode
     *
     * @since 5.78
     *
     * @deprecated since 5.103, use codecNameTriggered()
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 103, "Use codecNameTriggered().")
    void codecTriggered(QTextCodec *codec);
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 78)
    /**
     * Autodetection has been selected.
     * emits KEncodingProber::Universal if Default was selected.
     *
     * Applicable only if showAutoOptions in c'tor was true
     * @deprecated Since 5.78, use encodingProberTriggered(KEncodingProber::ProberType)
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 78, "Use KCodecAction::encodingProberTriggered(KEncodingProber::ProberType)")
    void triggered(KEncodingProber::ProberType); // clazy:exclude=overloaded-signal
#endif

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 102)
    /**
     * Autodetection has been selected.
     * emits KEncodingProber::Universal if Default was selected.
     *
     * Applicable only if showAutoOptions in c'tor was true
     *
     * In your KCodecAction subclass to be backward-compatible to KF < 5.78, emit instead
     * just the deprecated signal triggered(KEncodingProber::ProberType). That will also
     * automatically emit encodingProberTriggered(KEncodingProber::ProberType) because this
     * signal is connected to the deprecated one in the KCodecAction constructor.
     * Only if you compile against a variant of KConfigWidgets built without all API
     * deprecated up to version 5.78, then emit this signal directly.
     * Your code would be like this:
     * @code
     * #if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 78)
     *     // will also indirectly emit encodingProberTriggered since 5.78
     *     Q_EMIT triggered(encodingProber);
     * #else
     *     Q_EMIT encodingProberTriggered(encodingProber);
     * #endif
     * @endcode
     *
     * @since 5.78
     *
     * @deprecated since 5.102, no known users.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 102, "No known users")
    void encodingProberTriggered(KEncodingProber::ProberType);
#endif

    /**
     * Emitted when a codec was selected
     *
     * @param name the name of the selected encoding.
     *
     * Note that textTriggered(const QString &) is emitted too (as defined in KSelectAction).
     *
     * @since 5.103
     */
    void codecNameTriggered(const QString &name);

    /**
     * Emitted when the 'Default' codec action is triggered.
     */
    void defaultItemTriggered();

protected Q_SLOTS:
    void actionTriggered(QAction *) override;

protected:
    using KSelectAction::triggered;

private:
    friend class KCodecActionPrivate;
    std::unique_ptr<class KCodecActionPrivate> const d;
};

#endif
