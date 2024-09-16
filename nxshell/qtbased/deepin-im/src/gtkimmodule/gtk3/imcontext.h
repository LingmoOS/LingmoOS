// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _DIM_IM_CONTEXT_H
#define _DIM_IM_CONTEXT_H

#include "wl/client/ConnectionBase.h"

#include <gtk/gtk.h>

class DimGtkTextInputV3;
class XdgSurface;
class XdgToplevel;
class Keyboard;

/*
 * Type macros.
 */
#define DIM_TYPE_IM_CONTEXT (dim_im_context_get_type())
#define DIM_IM_CONTEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), DIM_TYPE_IM_CONTEXT, DimIMContext))

struct DimIMContextWaylandGlobal
{
    GtkIMContext *current;
    std::shared_ptr<DimGtkTextInputV3> ti;
    wl::client::ConnectionBase *wl;
    std::shared_ptr<XdgSurface> xdgSurface_;
    std::shared_ptr<XdgToplevel> xdgToplevel_;
    std::shared_ptr<Keyboard> keyboard_;

    guint serial;
    guint doneSerial;
};

struct preedit
{
    char *text;
    int cursorBegin;
    int cursorEnd;
};

struct surroundingDelete
{
    guint beforeLength;
    guint afterLength;
};

struct DimIMContext
{
    GtkIMContextSimple parent;
    GtkIMContext *slave;

#if GTK_CHECK_VERSION(4, 0, 0)
    GtkWidget *window;
#else
    GdkWindow *window;
#endif

    struct
    {
        char *text;
        int cursorIdx;
    } surrounding;

    enum zwp_text_input_v3_change_cause surroundingChange;
    surroundingDelete pendingSurroundingDelete;
    preedit currentPreedit;
    preedit pendingPreedit;

    char *pendingCommit;

    cairo_rectangle_int_t cursorRect;
    guint usePreedit : 1;
};

typedef struct _DimIMContextClass DimIMContextClass;

struct _DimIMContextClass
{
    GtkIMContextClass parent_class;
};

void textInputDeleteSurroundingTextApply(DimIMContextWaylandGlobal *global);
void textInputCommitApply(DimIMContextWaylandGlobal *global);
void textInputPreeditApply(DimIMContextWaylandGlobal *global);
void notifyImChange(DimIMContext *context, enum zwp_text_input_v3_change_cause cause);
void enable(DimIMContext *context, DimIMContextWaylandGlobal *global);
void disable(DimIMContext *context, DimIMContextWaylandGlobal *global);

G_BEGIN_DECLS
GType dim_im_context_get_type(void) G_GNUC_CONST;
DimIMContext *dimImContextNew(void);
void dim_im_context_register(GTypeModule *type_module);
G_END_DECLS

#endif // !_DIM_IM_CONTEXT_H
