/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2012 Kevin Ottens <ervin+bluesystems@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDRAGWIDGETDECORATOR_H
#define KDRAGWIDGETDECORATOR_H

#include <QObject>

#include <kwidgetsaddons_export.h>

#include <memory>

class QDrag;

/**
 * @brief A decorator which adds drag-support to widgets
 *
 * This is a decorator using an event filter to implement drag-support
 * in widgets.
 * You must override the virtual method dragObject() to specify the
 * QDrag to be used.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KWIDGETSADDONS_EXPORT KDragWidgetDecoratorBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDragEnabled READ isDragEnabled WRITE setDragEnabled)

public:
    /**
     * Default constructor.
     */
    explicit KDragWidgetDecoratorBase(QWidget *parent = nullptr);

    /**
     * Destructs the decorator.
     */
    ~KDragWidgetDecoratorBase() override;

    /**
     * Enables/disables drag-support. Default is enabled.
     */
    void setDragEnabled(bool enable);

    /**
     * @returns if drag support is enabled or not.
     */
    bool isDragEnabled() const;

protected:
    /**
     * @return the widget this decorator is attached to
     */
    QWidget *decoratedWidget() const;

    /**
     * Reimplement this and return the QDrag object that should be used
     * for the drag. Remember to give it "decoratedWidget()" as parent.
     *
     * Default implementation returns a null pointer, so that no drag is initiated.
     */
    virtual QDrag *dragObject();

    /**
     * Reimplemented to add drag-support
     */
    bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * Starts a drag (Copy by default) using dragObject()
     */
    virtual void startDrag();

private:
    std::unique_ptr<class KDragWidgetDecoratorBasePrivate> const d;
};

/**
 * @class KDragWidgetDecorator kdragwidgetdecorator.h KDragWidgetDecorator
 *
 * @brief A decorator which adds drag-support to widgets
 *
 * This is a decorator using an event filter to implement drag-support
 * in widgets.
 * You must set the dragObjectFactory to specify the QDrag to be used.
 *
 * @author Kevin Ottens <ervin@kde.org>
 */
template<class Widget>
class KDragWidgetDecorator : public KDragWidgetDecoratorBase
{
public:
    typedef QDrag *(*DragObjectFactory)(Widget *);

    KDragWidgetDecorator(Widget *parent = nullptr)
        : KDragWidgetDecoratorBase(parent)
        , m_factory(nullptr)
    {
    }

    /**
     * @return the QDrag factory used by this decorator
     */
    DragObjectFactory dragObjectFactory() const
    {
        return m_factory;
    }

    /**
     * Set a factory to be used by this decorator
     *
     * @param factory the new QDrag factory to use
     */
    void setDragObjectFactory(DragObjectFactory factory)
    {
        m_factory = factory;
    }

private:
    /**
     * Reimplemented to use the QDrag factory
     */
    QDrag *dragObject() override
    {
        if (m_factory) {
            Widget *w = static_cast<Widget *>(decoratedWidget());
            return m_factory(w);
        } else {
            return KDragWidgetDecoratorBase::dragObject();
        }
    }

    DragObjectFactory m_factory;
};

#endif // KDRAGWIDGETDECORATOR_H
