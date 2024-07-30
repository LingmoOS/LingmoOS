/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLAZYLOCALIZEDSTRING_H
#define KLAZYLOCALIZEDSTRING_H

#include "klocalizedstring.h"

#include <cstddef>

/**
 *  @class KLazyLocalizedString klazylocalizedstring.h <KLazyLocalizedString>
 *
 *  Lazy-initialized variant of KLocalizedString.
 *
 *  This is a safer replacement for the I18N_NOOP set of macros and allows
 *  marking strings for extraction without runtime-initializing KLocalizedString instances,
 *  for storing in static data tables.
 *
 *  Instances of KLazyLocalizedString are not created directly, unless they should be empty.
 *  Instead they have to be obtained via the kli18n* functions (similar to KLocalizedString
 *  and the ki18n* functions).
 *
 *  Example usage in a static message table:
 *  @code
 *  struct {
 *      MyClass::VehicleType type;
 *      KLazyLocalizedString msg;
 *  } static constexpr const vehicle_msg_table[] = {
 *      { MyClass::Train, kli18np("%1 train", "%1 trains") },
 *      { MyClass::Bus, kli18ncp("the vehicle, not the USB one", "%1 bus", "%1 buses") },
 *      ...
 *  };
 *
 *  ...
 *
 *  const auto it = std::find_if(std::begin(vehicle_msg_table), std::end(vehicle_msg_table), [vehicleType](const auto &m) { return m.type == vehicleType; });
 *  QString translatedMessage = (*it).msg.subs(vehicleCount).toString();
 *  @endcode
 *
 *  @note KLazyLocalizedString is primarily meant for storage in e.g. message tables,
 *  not for use in API, especially not across translation domains. This is due to
 *  it not carrying the translation domain in which it was created, but using the
 *  active translation domain at the point of converting to a KLocalizedString.
 *
 *  @since 5.89
 */
class KLazyLocalizedString
{
public:
    /**
     * Construct an empty message.
     *
     * Direct construction is used when e.g. a KLazyLocalizedString field in
     * a data table needs to be set, but there is not message to be used.
     * Directly constructed instances are not valid for
     * finalization by \c toString methods.
     *
     * \see isEmpty
     */
    constexpr inline KLazyLocalizedString() = default;

    /** Convert to a KLocalizedString to actually perform the translation and obtain a concrete
     *  localized string.
     *
     *  The translation domain active at this point will be used. This means this has to be
     *  called in the same translation domain the corresponding @c kli18n call is in.
     */
    Q_REQUIRED_RESULT inline operator KLocalizedString() const
    {
        if (!m_text) {
            return KLocalizedString();
        }
#ifdef TRANSLATION_DOMAIN
        return KLocalizedString(TRANSLATION_DOMAIN, m_context, m_text, m_plural, m_markupAware);
#else
        return KLocalizedString(nullptr, m_context, m_text, m_plural, m_markupAware);
#endif
    }

    /**
     * Check whether the message is empty.
     *
     * The message is considered empty if the object was constructed
     * via the default constructor.
     *
     * Empty messages are not valid for finalization.
     * The behavior of calling toString() on them is undefined.
     * In debug mode, an error mark may appear in the returned string.
     *
     * \return \c true if the message is empty, \c false otherwise
     */
    Q_REQUIRED_RESULT constexpr inline bool isEmpty() const
    {
        return (m_text == nullptr) || (m_text[0] == '\0');
    }

    /** Returns the raw untranslated text as passed to @p kli18n*. */
    Q_REQUIRED_RESULT constexpr inline const char *untranslatedText() const
    {
        return m_text;
    }

    /**
     * Finalize the translation.
     *
     * Creates translated QString. If the string has placeholders,
     * make sure to call instead \c KLazyLocalizedString::subs and
     * further \c KLocalizedString::subs methods before finalizing.
     * Translated text is searched for based on the global locale.
     *
     * \return finalized translation
     */
    Q_REQUIRED_RESULT inline QString toString() const
    {
        return this->operator KLocalizedString().toString();
    }

    /**
     * Like toString(), but look for translation only in given languages.
     *
     * Given languages override languages defined by the global locale.
     * If \p languages is empty, original message is returned.
     *
     * \param languages list of language codes (by decreasing priority)
     * \return finalized translation
     */
    Q_REQUIRED_RESULT inline QString toString(const QStringList &languages) const
    {
        return this->operator KLocalizedString().toString(languages);
    }

    /**
     * Like toString(), but look for translation in the given domain.
     *
     * \param domain the translation domain
     * \return finalized translation
     */
    Q_REQUIRED_RESULT inline QString toString(const char *domain) const
    {
        return this->operator KLocalizedString().toString(domain);
    }

    /**
     * Like toString(), but resolve KUIT markup into given visual format.
     *
     * Given visual format overrides that implied by the context UI marker.
     * If the message is not markup-aware, this is same as toString().
     *
     * \param format the target visual format
     * \return finalized translation
     */
    Q_REQUIRED_RESULT inline QString toString(Kuit::VisualFormat format) const
    {
        return this->operator KLocalizedString().toString(format);
    }

    /**
     * Indicate to look for translation only in given languages.
     *
     * \param languages list of language codes (by decreasing priority)
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString withLanguages(const QStringList &languages) const
    {
        return this->operator KLocalizedString().withLanguages(languages);
    }

    /**
     * Indicate to look for translation in the given domain.
     *
     * \param domain the translation domain
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString withDomain(const char *domain) const
    {
        return this->operator KLocalizedString().withDomain(domain);
    }

    /**
     * Indicate to resolve KUIT markup into given visual format.
     *
     * If the message is not markup-aware, this has no effect.
     *
     * \param format the target visual format
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString withFormat(Kuit::VisualFormat format) const
    {
        return this->operator KLocalizedString().withFormat(format);
    }

    /**
     * Substitute an int argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(int a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    /**
     * Substitute an unsigned int argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(uint a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    /**
     * Substitute a long argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(long a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    /**
     * Substitute an unsigned long argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(ulong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    /**
     * Substitute a long long argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(qlonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }

    /**
     * Substitute an unsigned long long argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(qulonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, base, fillChar);
    }
    /**
     * Substitute a double argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param format type of floating point formatting, like in QString::arg
     * \param precision number of digits after the decimal separator
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(double a, int fieldWidth = 0, char format = 'g', int precision = -1, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, format, precision, fillChar);
    }

    /**
     * Substitute a \c QChar argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(QChar a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, fillChar);
    }

    /**
     * Substitute a \c QString argument into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(const QString &a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, fillChar);
    }

    /**
     * Substitute another KLocalizedString into the message.
     *
     * \param a the argument
     * \param fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     * \param fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString subs(const KLocalizedString &a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const
    {
        return this->operator KLocalizedString().subs(a, fieldWidth, fillChar);
    }

    /**
     * Add dynamic context to the message.
     *
     * See \ref dyn_ctxt for use cases.
     *
     * \param key context key
     * \param value context value
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString inContext(const QString &key, const QString &value) const
    {
        return this->operator KLocalizedString().inContext(key, value);
    }

    /**
     * Relax matching between placeholders and arguments.
     *
     * Normally the placeholders should start from %1 and have no gaps,
     * and on finalization there must be exactly as many arguments
     * supplied through \c subs methods as there are unique plaecholders.
     * If this is not satisfied, in debug mode warnings are printed
     * and the finalized string may contain error marks.
     *
     * This method relaxes the placeholder-argument matching,
     * such that there must only be an argument available for
     * every present unique placeholder (taking placeholder numbers
     * to be 1-based indices into the argument list).
     * This can come useful in some situations.
     *
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString relaxSubs() const
    {
        return this->operator KLocalizedString().relaxSubs();
    }

    /**
     * Do not resolve KUIT markup.
     *
     * If the message is markup-aware
     * (constructed by one of \c \*xi18n\* calls),
     * this function can be used to make it non-markup-aware.
     * This may be useful for debugging markup.
     *
     * \return updated KLocalizedString
     */
    Q_REQUIRED_RESULT inline KLocalizedString ignoreMarkup() const
    {
        return this->operator KLocalizedString().ignoreMarkup();
    }

private:
    template<std::size_t TextSize>
    friend inline constexpr KLazyLocalizedString kli18n(const char (&text)[TextSize]);
    template<std::size_t ContextSize, std::size_t TextSize>
    friend constexpr inline KLazyLocalizedString kli18nc(const char (&context)[ContextSize], const char (&text)[TextSize]);
    template<std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString kli18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize]);
    template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString
    kli18ncp(const char (&context)[ContextSize], const char (&singular)[SingularSize], const char (&plural)[PluralSize]);
    template<std::size_t TextSize>
    friend constexpr inline KLazyLocalizedString klxi18n(const char (&text)[TextSize]);
    template<std::size_t ContextSize, std::size_t TextSize>
    friend constexpr inline KLazyLocalizedString klxi18nc(const char (&context)[ContextSize], const char (&text)[TextSize]);
    template<std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString klxi18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize]);
    template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
    friend constexpr inline KLazyLocalizedString
    klxi18ncp(const char (&context)[ContextSize], const char (&singular)[SingularSize], const char (&plural)[PluralSize]);

    constexpr inline KLazyLocalizedString(const char *context, const char *text, const char *plural, bool markupAware)
        : m_context(context)
        , m_text(text)
        , m_plural(plural)
        , m_markupAware(markupAware)
    {
    }

    const char *m_context = nullptr;
    const char *m_text = nullptr;
    const char *m_plural = nullptr;
    bool m_markupAware = false;
};

/**
 * Mark the string @p text for extraction.
 *
 * \param text string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t TextSize>
constexpr inline KLazyLocalizedString kli18n(const char (&text)[TextSize])
{
    return KLazyLocalizedString(nullptr, text, nullptr, false);
}

/**
 * Mark the string @p text with @p context for extraction.
 *
 * \param context context of the string
 * \param text string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t ContextSize, std::size_t TextSize>
constexpr inline KLazyLocalizedString kli18nc(const char (&context)[ContextSize], const char (&text)[TextSize])
{
    return KLazyLocalizedString(context, text, nullptr, false);
}

/**
 * Mark the string @p singular and @p plural for extraction.
 *
 * \param singular singular form of the string to translate
 * \param plural plural form of the string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString kli18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(nullptr, singular, plural, false);
}

/**
 * Mark the string @p singular and @p plural with @p context for extraction.
 *
 * \param context context of the string
 * \param singular singular form of the string to translate
 * \param plural plural form of the string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString kli18ncp(const char (&context)[ContextSize], const char (&singular)[SingularSize], const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(context, singular, plural, false);
}

/**
 * Mark the markup-aware string @p text for extraction.
 *
 * \param text string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t TextSize>
constexpr inline KLazyLocalizedString klxi18n(const char (&text)[TextSize])
{
    return KLazyLocalizedString(nullptr, text, nullptr, true);
}

/**
 * Mark the markup-aware string @p text with @p context for extraction.
 *
 * \param context context of the string
 * \param text string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t ContextSize, std::size_t TextSize>
constexpr inline KLazyLocalizedString klxi18nc(const char (&context)[ContextSize], const char (&text)[TextSize])
{
    return KLazyLocalizedString(context, text, nullptr, true);
}

/**
 * Mark the markup-aware string @p singular and @p plural for extraction.
 *
 * \param singular singular form of the string to translate
 * \param plural plural form of the string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString klxi18np(const char (&singular)[SingularSize], const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(nullptr, singular, plural, true);
}

/**
 * Mark the markup-aware string @p singular and @p plural with @p context for extraction.
 *
 * \param context context of the string
 * \param singular singular form of the string to translate
 * \param plural plural form of the string to translate
 * \return KLazyLocalizedString for deferred translation.
 * \since 5.89
 */
template<std::size_t ContextSize, std::size_t SingularSize, std::size_t PluralSize>
constexpr inline KLazyLocalizedString klxi18ncp(const char (&context)[ContextSize], const char (&singular)[SingularSize], const char (&plural)[PluralSize])
{
    return KLazyLocalizedString(context, singular, plural, true);
}

#endif // KLAZYLOCALIZEDSTRING_H
