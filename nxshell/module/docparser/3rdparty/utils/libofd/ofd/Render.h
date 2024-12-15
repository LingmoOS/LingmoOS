#ifndef __OFD_RENDER_H__
#define __OFD_RENDER_H__

#include <memory>
#include <tuple>
#include "ofd/Common.h"
#include "ofd/DrawState.h"

namespace ofd{

    typedef std::tuple<double, double, double> VisibleParams;

    // ======== class Render ========
    class Render {
    public:
        Render();
        virtual ~Render();

        virtual void DrawPage(PagePtr page, VisibleParams visibleParams);
        VisibleParams GetVisibleParams() const;
        void SetVisibleParams(VisibleParams visibleParams);

        const DrawState& GetDrawState() const {return m_drawState;};
        DrawState& GetDrawState() {return m_drawState;};
        void SetDrawState(const DrawState &drawState){m_drawState = drawState;};

    private:
        VisibleParams m_visibleParams;
        DrawState m_drawState;

    }; // class Render

    typedef std::shared_ptr<Render> RenderPtr;

    enum class RenderType{
        Cairo,
    };

    class RenderFactory{
    public:
        //static RenderPtr CreateRenderInstance(RenderType renderType); 
    }; // class RenderFactory

}; // namespace ofd

#endif // __OFD_RENDER_H__
