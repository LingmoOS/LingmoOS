#! /usr/bin/python

# This a simple test, using the dogtail framework:
#
# Click on All, Installed, Updates and check that the button state
# and the main page content update as expected. Type a few characters
# on the overview page, hit Enter, and verify that we end up on the
# search page. Hit Escape and verify that we go back to the overview
# page.

from gi.repository import Gio

settings = Gio.Settings.new("org.gnome.desktop.interface")
settings.set_boolean ("toolkit-accessibility", True)

import os
from dogtail.tree import *
from dogtail.utils import *
from dogtail.procedural import *

try:
    run('lingmo-appstore')

    app_name = 'lingmo-appstore'
    app = root.application(app_name)

    all_button = app.child('All')
    installed_button = app.child('Installed')
    updates_button = app.child('Updates')
    back_button = app.child('Go back')

    overview_page = app.child('Overview page')
    installed_page = app.child('Installed page')
    updates_page = app.child('Updates page')
    search_page = app.child('Search page')
    details_page = app.child('Details page')
    install_button = details_page.child('Install')
    uninstall_button = details_page.child('Uninstall')

    search_page_listbox = search_page.child(roleName='list box')

    all_button.click()
    assert (all_button.checked)
    assert (not installed_button.checked)
    assert (not updates_button.checked)
    assert (overview_page.showing)
    assert (not installed_page.showing)
    assert (not updates_page.showing)
    assert (not search_page.showing)

    installed_button.click()
    assert (not all_button.checked)
    assert (installed_button.checked)
    assert (not updates_button.checked)
    assert (not overview_page.showing)
    assert (installed_page.showing)
    assert (not updates_page.showing)
    assert (not search_page.showing)

    updates_button.click()
    assert (not all_button.checked)
    assert (not installed_button.checked)
    assert (updates_button.checked)
    assert (not overview_page.showing)
    assert (not installed_page.showing)
    assert (updates_page.showing)
    assert (not search_page.showing)

    installed_button.click()
    assert (not all_button.checked)
    assert (installed_button.checked)
    assert (not updates_button.checked)
    assert (not overview_page.showing)
    assert (installed_page.showing)
    assert (not updates_page.showing)
    assert (not search_page.showing)

    all_button.click()
    assert (all_button.checked)
    assert (not installed_button.checked)
    assert (not updates_button.checked)
    assert (overview_page.showing)
    assert (not installed_page.showing)
    assert (not updates_page.showing)
    assert (not search_page.showing)

    type("archive\n")
    doDelay(2)
    assert (not all_button.checked)
    assert (not installed_button.checked)
    assert (not updates_button.checked)
    assert (not overview_page.showing)
    assert (not installed_page.showing)
    assert (not updates_page.showing)
    assert (search_page.showing)

    """Details page test section"""
    search_page_listbox.child(roleName='label', name='Archive Manager').click()
    doDelay(4)
    assert (not overview_page.showing)
    assert (not installed_page.showing)
    assert (not updates_page.showing)
    assert (not search_page.showing)
    assert (details_page.showing)
    assert (install_button.showing or uninstall_button.showing)
    assert (back_button.showing)
    assert (root.application(app_name).child('Details page')
            .child(roleName='push button', name='History')
            .states.contains(pyatspi.STATE_VISIBLE))
    assert (root.application(app_name).child('Details page')
            .child(roleName='push button', name='Website')
            .states.contains(pyatspi.STATE_VISIBLE))

    if install_button.showing:
        assert (not root.application(app_name).child('Details page')
                    .child(roleName='push button', name='Launch')
                    .states.contains(pyatspi.STATE_VISIBLE))
    else:
        assert (root.application(app_name).child('Details page')
                .child(roleName='push button', name='Launch')
                .states.contains(pyatspi.STATE_VISIBLE))

    back_button.click()
    assert (not all_button.checked)
    assert (not installed_button.checked)
    assert (not updates_button.checked)
    assert (not overview_page.showing)
    assert (not installed_page.showing)
    assert (not updates_page.showing)
    assert (search_page.showing)
    assert (not install_button.showing)
    assert (not uninstall_button.showing)
    assert (not back_button.showing)

    keyCombo("Escape")
    assert (all_button.checked)
    assert (not installed_button.checked)
    assert (not updates_button.checked)
    assert (overview_page.showing)
    assert (not installed_page.showing)
    assert (not updates_page.showing)
    assert (not search_page.showing)

    super_menu = root.application('lingmo-shell').child(roleName='menu', name='Appstore')
    super_menu.click()
    root.application('lingmo-shell').child(roleName='label', name='Appstore Sources').click()
    assert (len(app.children) == 2)
    sources_dialog = app.children[-1]
    assert (sources_dialog.child(roleName='label', name='Appstore Sources'))
finally:
    os.system("killall lingmo-appstore")
