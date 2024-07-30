/*
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_TEXTHINTINTERFACE_H
#define KTEXTEDITOR_TEXTHINTINTERFACE_H

#include <QString>

#include <ktexteditor_export.h>

#include <ktexteditor/cursor.h>

namespace KTextEditor
{
class TextHintProvider;
class View;

/**
 * \brief Class to provide text hints for a View.
 *
 * The class TextHintProvider is used in combination with TextHintInterface.
 * TextHintProvider allows to provide text hint information for text under
 * the mouse cursor.
 *
 * To use this class, derive your provider from TextHintProvider and register
 * it with TextHintInterface::registerTextHintProvider(). When not needed
 * anymore, make sure to remove your provider by calling
 * TextHintInterface::unregisterTextHintProvider(), otherwise the View will
 * contain a dangling pointer to your potentially deleted provider.
 *
 * Detailed information about how to use the TextHintInterface can be found
 * in the documentation about the TextHintInterface.
 *
 * \see TextHintInterface
 * \p since 5.0
 */
class KTEXTEDITOR_EXPORT TextHintProvider
{
public:
    /**
     * Default constructor.
     */
    TextHintProvider();

    /**
     * Virtual destructor to allow inheritance.
     */
    virtual ~TextHintProvider();

    /**
     * This function is called whenever the users hovers over text such
     * that the text hint delay passes. Then, textHint() is called
     * for each registered TextHintProvider.
     *
     * Return the text hint (possibly Qt richtext) for @p view at @p position.
     *
     * If you do not have any contents to show, just return an empty QString().
     *
     * \param view the view that requests the text hint
     * \param position text cursor under the mouse position
     * \return text tool tip to be displayed, may be Qt richtext
     */
    virtual QString textHint(KTextEditor::View *view, const KTextEditor::Cursor &position) = 0;

private:
    class TextHintProviderPrivate *const d = nullptr;
};

}

#endif
