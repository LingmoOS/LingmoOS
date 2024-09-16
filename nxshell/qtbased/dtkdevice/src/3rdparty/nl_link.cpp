
#include "nl_link.h"


NLLink::NLLink(struct rtnl_link *link, bool hold)
    : m_link(link)
    , m_hold(hold)
{
    Q_ASSERT(m_link != nullptr);
}

NLLink::~NLLink()
{
    if (m_hold && m_link)
        rtnl_link_put(m_link);
}

