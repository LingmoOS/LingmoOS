/*
 * SPDX-FileCopyrightText: 2011~2011 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

/* this file is based on from libgnomekbd/libgnomekbd/gkbd-keyboard-drawing.c */

#ifndef KEYBOARDLAYOUTWIDGET_H
#define KEYBOARDLAYOUTWIDGET_H

#include <QAbstractNativeEventFilter>
#include <QMap>
#include <QWidget>
#include <vector>

class QPainter;
struct Doodad;
struct _XkbDesc;
struct _XkbShapeDoodad;
union _XkbDoodad;

typedef enum {
    KEYBOARD_DRAWING_ITEM_TYPE_INVALID = 0,
    KEYBOARD_DRAWING_ITEM_TYPE_KEY,
    KEYBOARD_DRAWING_ITEM_TYPE_KEY_EXTRA,
    KEYBOARD_DRAWING_ITEM_TYPE_DOODAD
} KeyboardDrawingItemType;

typedef enum {
    KEYBOARD_DRAWING_POS_TOPLEFT,
    KEYBOARD_DRAWING_POS_TOPRIGHT,
    KEYBOARD_DRAWING_POS_BOTTOMLEFT,
    KEYBOARD_DRAWING_POS_BOTTOMRIGHT,
    KEYBOARD_DRAWING_POS_TOTAL,
    KEYBOARD_DRAWING_POS_FIRST = KEYBOARD_DRAWING_POS_TOPLEFT,
    KEYBOARD_DRAWING_POS_LAST = KEYBOARD_DRAWING_POS_BOTTOMRIGHT
} KeyboardDrawingGroupLevelPosition;

namespace fcitx {
namespace kcm {

struct DrawingItem {
    DrawingItem()
        : type(KEYBOARD_DRAWING_ITEM_TYPE_INVALID), originX(0), originY(0),
          angle(0), priority(0) {}

    virtual ~DrawingItem() {}
    KeyboardDrawingItemType type;
    int originX;
    int originY;
    int angle;
    unsigned int priority;
};

struct Doodad : public DrawingItem {
    Doodad() : doodad(0), on(0) {}
    union _XkbDoodad *doodad;
    int on;
};

struct DrawingKey : public DrawingItem {
    DrawingKey() : xkbkey(0), pressed(false), keycode(0) {}
    struct _XkbKey *xkbkey;
    bool pressed;
    unsigned int keycode;
};

struct KeyboardDrawingGroupLevel {
    int group;
    int level;
};

class KeyboardLayoutWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyboardLayoutWidget(QWidget *parent = 0);
    virtual ~KeyboardLayoutWidget();
    void setGroup(int group);
    void setKeyboardLayout(const QString &layout, const QString &variant);
    void generatePixmap(bool force = false);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void init();
    void alloc();
    void release();
    void initInicatorDoodad(union _XkbDoodad *xkbdoodad, Doodad &doodad);
    unsigned int findKeycode(const char *keyName);
    void rotateRectangle(int origin_x, int origin_y, int x, int y, int angle,
                         int &rotated_x, int &rotated_y);
    bool parseXkbColorSpec(char *colorspec, QColor &color);
    void initColors();

    void focusOutEvent(QFocusEvent *event) override;

    void drawKey(QPainter *painter, DrawingKey *item);
    void drawDoodad(QPainter *painter, Doodad *doodad);
    void drawKeyLabel(QPainter *painter, unsigned int keycode, int angle,
                      int arg4, int originY, int x2, int y2, bool pressed);
    void drawKeyLabelHelper(QPainter *painter, const QString &text, int angle,
                            int glp, int end_glp, int x, int y, int width,
                            int height, int padding, bool is_pressed);

    void drawShapeDoodad(QPainter *painter, Doodad *doodad,
                         struct _XkbShapeDoodad *shapeDoodad);
    void drawTextDoodad(QPainter *painter, Doodad *doodad,
                        struct _XkbTextDoodad *textDoodad);
    void drawIndicatorDoodad(QPainter *painter, Doodad *doodad,
                             struct _XkbIndicatorDoodad *indicatorDoodad);

    int calcShapeOriginOffsetX(struct _XkbOutline *outline);
    void drawOutline(QPainter *painter, struct _XkbOutline *outline,
                     QColor color, int angle, int originX, int originY);
    void drawRectangle(QPainter *painter, QColor color, int angle, int xkb_x,
                       int xkb_y, int xkb_width, int xkb_height,
                       unsigned int radius);
    void drawPolygon(QPainter *painter, QColor color, int originX, int originY,
                     struct _XkbPoint *points, unsigned int num_points,
                     unsigned int radius);
    void rotateCoordinate(int originX, int originY, int x, int y, int angle,
                          int *rotated_x, int *rotated_y);
    int xkbToPixmapCoord(int n);
    double xkbToPixmapDouble(double d);
    void roundedPolygon(QPainter *painter, bool filled, double radius,
                        const std::vector<QPointF> &points);
    void drawCurveRectangle(QPainter *painter, bool filled, QColor color, int x,
                            int y, int width, int height, double radius);
    void roundedCorner(QPainterPath &path, QPointF b, QPointF c, double radius);
    void resizeEvent(QResizeEvent *event) override;
    void setKeyboard(struct _XkbComponentNames *xkbDesc);

private:
    void keyEvent(QKeyEvent *keyEvent);
    QString keySymToString(unsigned long keysym);

    QList<DrawingItem *> keyboardItems;
    std::vector<DrawingKey> keys;
    QList<Doodad *> physicalIndicators;
    struct _XkbDesc *xkb = nullptr;
    unsigned int l3mod = 0;
    std::vector<QColor> colors;
    QPixmap image;
    double ratio = 1.0;
    KeyboardDrawingGroupLevel **groupLevels;
    bool trackModifiers = false;
    int mods;
    QMap<unsigned int, unsigned int> deadMap;
};

} // namespace kcm
} // namespace fcitx

#endif
