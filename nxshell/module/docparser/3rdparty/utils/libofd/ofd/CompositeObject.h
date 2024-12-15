#ifndef __OFD_COMPOSITEOBJECT_H__
#define __OFD_COMPOSITEOBJECT_H__

#include <memory>
#include "ofd/Object.h"

namespace ofd{


    // ======== class CompositeObject ========
    // OFD P52，Page.xsd.
    class CompositeObject : public Object{
    public:

        CompositeObject(LayerPtr layer);
        virtual ~CompositeObject();

        // =============== Public Methods ================
    public:
        virtual std::string to_string() const override;

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;
        virtual void RecalculateBoundary() override;

    private:
        ObjectArray m_objects;
    }; // class OFDCompositeObject
    typedef std::shared_ptr<CompositeObject> CompositeObjectPtr;

}; // namespace ofd

#endif // __OFD_COMPOSITEOBJECT_H__
