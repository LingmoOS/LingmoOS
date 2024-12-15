#include "ofd/Render.h"
#include "ofd/Page.h"
//#include "ofd/CairoRender.h"

using namespace ofd;

Render::Render() : m_visibleParams(std::make_tuple(0.0, 0.0, 1.0)){
    // TODO Debug page drawing by filter page and object
    // 针对sample2仅绘制第6页的ID=71的PathObject(国网标志，径向渐变）

    //m_drawState.Debug.Enabled = true;
    //m_drawState.Debug.PageDrawing = 6;
    //m_drawState.Debug.PathObjectID = 71;

}

Render::~Render(){
}

void Render::DrawPage(PagePtr page, VisibleParams visibleParams){
    m_visibleParams = visibleParams;
}

VisibleParams Render::GetVisibleParams() const{
    return m_visibleParams;
}

void Render::SetVisibleParams(VisibleParams visibleParams){
    m_visibleParams = visibleParams;
}

//RenderPtr RenderFactory::CreateRenderInstance(RenderType renderType){
    //if ( renderType == RenderType::Cairo ){
        //return std::make_shared<OFDCairoRender>();
    //} else {
        //return nullptr;
    //}
//}
