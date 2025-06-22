# KXMLGUI

Framework for managing menu and toolbar actions

## Introduction

KXMLGUI provides a framework for managing menu and toolbar actions in an
abstract way. The actions are configured through a XML description and hooks
in the application code. The framework supports merging of multiple
description for example for integrating actions from plugins.

## Kiosk

KXMLGui makes use of the Kiosk authorization functionality of KConfig (see the
KAuthorized namespace in that framework).  Notably, QAction instances added to a
KActionCollection are disabled if KAuthorized::authorizeAction() reports that
they are not authorized.  The items on the standard help menu (KHelpMenu) can
likewise be disabled based on Kiosk settings, and toolbar editing can be
restricted.

See KActionCollection, KHelpMenu and KToolBar documentation for more
information.


