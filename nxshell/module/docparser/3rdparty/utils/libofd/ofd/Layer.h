#ifndef __OFD_LAYER_H__
#define __OFD_LAYER_H__

#include <memory>
#include <string>
#include <vector>
#include "ofd/Common.h"

namespace ofd{

    enum class LayerType{
        BODY,
        BACKGROUND,
        FOREGROUND,
        CUSTOM,
    };

    class Layer : public std::enable_shared_from_this<Layer>{
        public:
            Layer(PagePtr page);
            virtual ~Layer();
            LayerPtr GetSelf(){return shared_from_this();};

            // =============== Public Attributes ================
        public:
            uint64_t    ID;
            LayerType Type;

            // =============== Public Methods ================
        public:
            size_t GetNumObjects() const{return m_objects.size();};
            const ObjectPtr GetObject(size_t idx) const {return m_objects[idx];};
            ObjectPtr GetObject(size_t idx) {return m_objects[idx];};
            void AddObject(ObjectPtr object); 

            // ---------------- Private Attributes ----------------
        public:
            const PagePtr GetPage() const {return m_page.lock();};
            PagePtr GetPage() {return m_page.lock();};
            const ObjectArray& GetObjects() const{return m_objects;};
            ObjectArray& GetObjects() {return m_objects;};

        private:
            std::weak_ptr<Page> m_page;
            ObjectArray m_objects;

    }; // class Layer


}; // namespace ofd

#endif // __OFD_LAYER_H__
