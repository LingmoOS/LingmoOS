

#include "display.h"

bool scan_display(hwNode & n)
{
  if((n.getClass() == hw::display) && n.isCapable("vga"))
  {
    n.claim(true);
  }

  for(unsigned int i=0; i<n.countChildren(); i++)
    scan_display(*n.getChild(i));

  return true;
}
