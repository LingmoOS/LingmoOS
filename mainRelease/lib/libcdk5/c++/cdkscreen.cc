#include "cdkscreen.h"

CDKScreen::CDKScreen() 
{
  cdkscreen = initCDKScreen(NULL);
  // Now, set up color.
  initCDKColor();
}

CDKScreen::~CDKScreen() 
{
  destroyCDKScreen(cdkscreen);
  endCDK();
}

CDKSCREEN *CDKScreen::screen(void) 
{
  return cdkscreen;
}

void CDKScreen::refreshscr(void) 
{
  refreshCDKScreen(cdkscreen);
}

void CDKScreen::erasescr(void) 
{
  eraseCDKScreen(cdkscreen);
}

