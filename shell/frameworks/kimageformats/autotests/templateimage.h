/*
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TEMPLATEIMAGE_H
#define TEMPLATEIMAGE_H

#include <QFileInfo>

/*!
 * \brief The TemplateImage class
 * Given an image name, it decides the template image to compare it with.
 */
class TemplateImage
{
public:
    /*!
     * \brief TemplateImage
     * \param fi The image to test.
     */
    TemplateImage(const QFileInfo& fi);

    /*!
     * \brief TemplateImage
     * Default copy constructor.
     */
    TemplateImage(const TemplateImage& other) = default;
    /*!
     * \brief operator =
     * Default copy operator
     */
    TemplateImage& operator=(const TemplateImage& other) = default;

    /*!
     * \brief isTemplate
     * \return True if the image is a template, false otherwise.
     * \sa suffixes
     */
    bool isTemplate() const;

    /*!
     * \brief compareImage
     * \param skipTest True if the test should be skipped (e.g. image format not supported by current Qt version).
     * \return The template image to use for the comparison.
     */
    QFileInfo compareImage(bool &skipTest) const;

    /*!
     * \brief suffixes
     * \return The list of suffixes considered templates.
     */
    static QStringList suffixes();

private:
    /*!
     * \brief legacyImage
     * \return The template image calculated from the source image name.
     */
    QFileInfo legacyImage() const;

    /*!
     * \brief jsonImage
     * \param skipTest True if the test should be skipped (not supported).
     * \return The template image read from the corresponding JSON.
     */
    QFileInfo jsonImage(bool &skipTest) const;

private:
    QFileInfo m_fi;
};

#endif // TEMPLATEIMAGE_H
