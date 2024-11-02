#ifndef XATOMHELPER_H
#define XATOMHELPER_H

#include <QObject>
#include <QX11Info>

#include <limits.h>
#include <NETWM>
//#include <X11/X.h>
#include <X11/Xatom.h>

struct UnityCorners {
    ulong topLeft = 0;
    ulong topRight = 0;
    ulong bottomLeft = 0;
    ulong bottomRight = 0;
};

typedef struct {
    ulong flags = 0;
    ulong functions = 0;
    ulong decorations = 0;
    long input_mode = 0;
    ulong status = 0;
} MotifWmHints, MwmHints;

#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_HINTS_INPUT_MODE    (1L << 2)
#define MWM_HINTS_STATUS        (1L << 3)

#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)

#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)

#define MWM_INPUT_MODELESS 0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL 1
#define MWM_INPUT_SYSTEM_MODAL 2
#define MWM_INPUT_FULL_APPLICATION_MODAL 3
#define MWM_INPUT_APPLICATION_MODAL MWM_INPUT_PRIMARY_APPLICATION_MODAL

#define MWM_TEAROFF_WINDOW	(1L<<0)

namespace LINGMO {
class Decoration;
}

class XAtomHelper : public QObject
{
    friend class LINGMO::Decoration;
    Q_OBJECT
public:
    static XAtomHelper *getInstance();

    static bool isFrameLessWindow(int winId);

    bool isWindowDecorateBorderOnly(int winId);
    bool isWindowMotifHintDecorateBorderOnly(const MotifWmHints &hint);
    bool isLINGMOCsdSupported();
    bool isLINGMODecorationWindow(int winId);

    UnityCorners getWindowBorderRadius(int winId);
    void setWindowBorderRadius(int winId, const UnityCorners &data);
    void setWindowBorderRadius(int winId, int topLeft, int topRight, int bottomLeft, int bottomRight);
    void setLINGMODecoraiontHint(int winId, bool set = true);

    void setWindowMotifHint(int winId, const MotifWmHints &hints);
    MotifWmHints getWindowMotifHint(int winId);

private:
    explicit XAtomHelper(QObject *parent = nullptr);

    unsigned long registerLINGMOCsdNetWmSupportAtom();
    void unregisterLINGMOCsdNetWmSupportAtom();

    unsigned long m_motifWMHintsAtom = 0;
    unsigned long m_unityBorderRadiusAtom = 0;
    unsigned long m_lingmoDecorationAtion = 0;
};

#endif // XATOMHELPER_H
