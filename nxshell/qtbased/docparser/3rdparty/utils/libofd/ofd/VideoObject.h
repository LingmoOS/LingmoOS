#ifndef __OFD_VIDEOOBJECT_H__
#define __OFD_VIDEOOBJECT_H__

#include <memory>
#include "ofd/Object.h"

namespace ofd{

    // ======== class OFDVideoObject ========
    // OFD P71，Page.xsd.
    class VideoObject : public Object{
    public:

        VideoObject(LayerPtr layer);
        virtual ~VideoObject();


        // =============== Public Methods ================
    public:
        virtual std::string to_string() const override;

    protected:
        virtual void GenerateAttributesXML(utils::XMLWriter &writer) const override;
        virtual void GenerateElementsXML(utils::XMLWriter &writer) const override;

        virtual bool FromAttributesXML(utils::XMLElementPtr objectElement) override;
        virtual bool IterateElementsXML(utils::XMLElementPtr childElement) override;
        virtual void RecalculateBoundary() override;
    }; // class OFDVideoObject
    typedef std::shared_ptr<VideoObject> VideoObjectPtr;

}; // namespace ofd

#endif // __OFD_VIDEOOBJECT_H__
