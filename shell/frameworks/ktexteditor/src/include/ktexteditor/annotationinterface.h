/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_ANNOTATIONINTERFACE_H
#define KTEXTEDITOR_ANNOTATIONINTERFACE_H

#include <ktexteditor_export.h>

#include <QObject>

class QMenu;

namespace KTextEditor
{
class View;
class AbstractAnnotationItemDelegate;

/**
 * \brief An model for providing line annotation information
 *
 * \section annomodel_intro Introduction
 *
 * AnnotationModel is a model-like interface that can be implemented
 * to provide annotation information for each line in a document. It provides
 * means to retrieve several kinds of data for a given line in the document.
 *
 * \section annomodel_impl Implementing a AnnotationModel
 *
 * The public interface of this class is loosely based on the QAbstractItemModel
 * interfaces. It only has a single method to override which is the \ref data()
 * method to provide the actual data for a line and role combination.
 *
 * \since 4.1
 * \see KTextEditor::AnnotationInterface, KTextEditor::AnnotationViewInterface
 */
class KTEXTEDITOR_EXPORT AnnotationModel : public QObject
{
    Q_OBJECT
public:
    ~AnnotationModel() override;

    enum { GroupIdentifierRole = Qt::UserRole };
    // KF6: add AnnotationModelUserRole = Qt::UserRole + 0x100

    /**
     * data() is used to retrieve the information needed to present the
     * annotation information from the annotation model. The provider
     * should return useful information for the line and the data role.
     *
     * The following roles are supported:
     * - Qt::DisplayRole - a short display text to be placed in the border
     * - Qt::TooltipRole - a tooltip information, longer text possible
     * - Qt::BackgroundRole - a brush to be used to paint the background on the border
     * - Qt::ForegroundRole - a brush to be used to paint the text on the border
     * - AnnotationModel::GroupIdentifierRole - a string which identifies a
     *   group of items which will be highlighted on mouseover; return the same
     *   string for all items in a group (KDevelop uses a VCS revision number, for example)
     *
     *
     * \param line the line for which the data is to be retrieved
     * \param role the role to identify which kind of annotation is to be retrieved
     *
     * \returns a QVariant that contains the data for the given role.
     */
    virtual QVariant data(int line, Qt::ItemDataRole role) const = 0; // KF6: use int for role

Q_SIGNALS:
    /**
     * The model should emit the signal reset() when the text of almost all
     * lines changes. In most cases it is enough to call lineChanged().
     *
     * \note Kate Part implementation details: Whenever reset() is emitted Kate
     *       Part iterates over all lines of the document. Kate Part searches
     *       for the longest text to determine the annotation border's width.
     *
     * \see lineChanged()
     */
    void reset();

    /**
     * The model should emit the signal lineChanged() when a line has to be
     * updated.
     *
     * \note Kate Part implementation details: lineChanged() repaints the whole
     *       annotation border automatically.
     */
    void lineChanged(int line);
};

} // namespace KTextEditor

#endif
