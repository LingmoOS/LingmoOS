#ifndef __OFD_DRAWSTATE_H__
#define __OFD_DRAWSTATE_H__

namespace ofd{

    typedef struct DrawState{

        struct DebugParams{
            bool Enabled;
            size_t PageDrawing;
            size_t PathObjectID;
            size_t ImageObjectID;

            DebugParams():
                Enabled(false), 
                PageDrawing(0), 
                PathObjectID(0), 
                ImageObjectID(0){
            }
        };
        DebugParams Debug;


    } DrawSate_t;

}; // namespace ofd

#endif // __OFD_DRAWSTATE_H__
