#include "kwineffectsex.h"

namespace KWin
{

const QSet<QString> EffectsHandlerEx::motionEffectList = {
    "kwin4_effect_maximize", "kwin4_effect_scale", "kwin4_effect_squash", "kwin4_effect_fadingpopups",
    "magiclamp", "slide", "maximizeex"
};

EffectsHandlerEx::EffectsHandlerEx(CompositingType type)
    : EffectsHandler (type)
{
    KWin::effectsEx = this;
}

EffectsHandlerEx::~EffectsHandlerEx()
{
    KWin::effectsEx = nullptr;
}

EffectsHandlerEx* effectsEx = nullptr;

EffectFrameEx::EffectFrameEx()
    : EffectFrame()
    // , QObject(nullptr)
{

}

EffectFrameEx::~EffectFrameEx()
{

}

}
