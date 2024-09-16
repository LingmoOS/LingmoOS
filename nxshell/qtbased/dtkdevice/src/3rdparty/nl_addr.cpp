
#include "nl_addr.h"


NLAddr::NLAddr(struct rtnl_addr *addr, bool hold)
    : m_addr(addr)
    , m_hold(hold)
{
    Q_ASSERT(m_addr != nullptr);
}

NLAddr::~NLAddr()
{
    if (m_hold)
        rtnl_addr_put(m_addr);
}


