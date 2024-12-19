#!/usr/bin/python3

# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Appstore Foundation; either version 2.1+ of the License, or (at your option)
# any later version.  See http://www.gnu.org/copyleft/lgpl.html for the full
# text of the license.
#
# The LGPL 2.1+ has been chosen as that’s the license eos-updater is under.


from enum import IntEnum
import os
import time
import unittest
import dbus
import dbusmock
import ddt


__author__ = 'Philip Withnall'
__email__ = 'withnall@endlessm.com'
__copyright__ = '© 2019 Endless Mobile Inc.'
__license__ = 'LGPL 2.1+'


class UpdaterState(IntEnum):
    '''eos-updater states; see its State property'''
    NONE = 0
    READY = 1
    ERROR = 2
    POLLING = 3
    UPDATE_AVAILABLE = 4
    FETCHING = 5
    UPDATE_READY = 6
    APPLYING_UPDATE = 7
    UPDATE_APPLIED = 8


@ddt.ddt
class ManualTest(dbusmock.DBusTestCase):
    '''A manual test of the eos-updater plugin in lingmo-appstore.

    It creates a mock eos-updater D-Bus daemon, on the real system bus (because
    otherwise lingmo-appstore’s other plugins can’t communicate with their
    system daemons; to fix this, we’d need to mock those up too). The test
    harness provides the user with instructions about how to run lingmo-appstore
    and what to do in it, waiting for them to press enter between steps.

    FIXME: This test could potentially eventually be automated by doing the UI
    steps using Dogtail or OpenQA.

    It tests various classes of interaction between the plugin and the daemon:
    normal update process (with and without an update available); error returns
    from the daemon; cancellation of the daemon by another process;
    cancellation of the daemon from lingmo-appstore; and the daemon unexpectedly
    going away (i.e. crashing).
    '''

    @classmethod
    def setUpClass(cls):
        # FIXME: See the comment below about why we currently run on the actual
        # system bus.
        # cls.start_system_bus()
        cls.dbus_con = cls.get_dbus(True)

    def setUp(self):
        # Work out the path to the dbusmock template in the same directory as
        # this file.
        self_path = os.path.dirname(os.path.realpath(__file__))
        template_path = os.path.join(self_path, 'eos_updater.py')

        # Spawn a python-dbusmock server. Use the actual system bus, since
        # lingmo-appstore needs to access various other services (such as
        # packagekit) which we don’t currently mock (FIXME).
        (self.p_mock, self.obj_eos_updater) = self.spawn_server_template(
            template_path, {}, stdout=None)
        self.dbusmock = dbus.Interface(self.obj_eos_updater,
                                       dbusmock.MOCK_IFACE)

    def tearDown(self):
        self.kill_lingmo_software()
        self.p_mock.terminate()
        self.p_mock.wait()

    def launch_lingmo_software(self):
        '''Instruct the user to launch lingmo-appstore'''
        print('Launch lingmo-appstore with:')
        print('lingmo-appstore --verbose')
        self.manual_check('Press enter to continue')

    def kill_lingmo_software(self):
        '''Instruct the user to kill lingmo-appstore'''
        print('Kill lingmo-appstore with:')
        print('pkill lingmo-appstore')
        self.manual_check('Press enter to continue')

    def await_state(self, state):
        '''Block until eos-updater reaches the given `state`'''
        print('Awaiting state %u' % state)
        props_iface = dbus.Interface(self.obj_eos_updater,
                                     dbus.PROPERTIES_IFACE)
        while props_iface.Get('com.endlessm.Updater', 'State') != state:
            time.sleep(0.2)

    def manual_check(self, prompt):
        '''Instruct the user to do a manual check and block until done'''
        input('\033[92;1m' + prompt + '\033[0m\n')

    def test_poll_no_update(self):
        '''Test that no updates are shown if eos-updater successfully says
        there are none.'''
        self.dbusmock.SetPollAction(
            'no-update', dbus.Dictionary({}, signature='sv'), '', '')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check there are no EOS updates listed')
        self.await_state(UpdaterState.READY)

    @ddt.data('com.endlessm.Updater.Error.WrongState',
              'com.endlessm.Updater.Error.LiveBoot',
              'com.endlessm.Updater.Error.WrongConfiguration',
              'com.endlessm.Updater.Error.NotOstreeSystem',
              'com.endlessm.Updater.Error.Cancelled')
    def test_poll_early_error(self, error_name):
        '''Test that a D-Bus error return from Poll() is handled correctly.'''
        self.dbusmock.SetPollAction(
            'early-error', dbus.Dictionary({}, signature='sv'),
            error_name, 'Some error message.')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.ERROR)

        if error_name != 'com.endlessm.Updater.Error.Cancelled':
            self.manual_check('Check there are no EOS updates listed, and a '
                              'GsPluginEosUpdater error is printed on the '
                              'terminal')
        else:
            self.manual_check('Check there are no EOS updates listed, and no '
                              'GsPluginEosUpdater cancellation error is '
                              'printed on the terminal')

    @ddt.data('com.endlessm.Updater.Error.WrongState',
              'com.endlessm.Updater.Error.LiveBoot',
              'com.endlessm.Updater.Error.WrongConfiguration',
              'com.endlessm.Updater.Error.NotOstreeSystem',
              'com.endlessm.Updater.Error.Cancelled')
    def test_poll_late_error(self, error_name):
        '''Test that a transition to the Error state after successfully calling
        Poll() is handled correctly.'''
        self.dbusmock.SetPollAction(
            'late-error', dbus.Dictionary({}, signature='sv'),
            error_name, 'Some error message.')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        if error_name != 'com.endlessm.Updater.Error.Cancelled':
            self.manual_check('Check there are no EOS updates listed, and a '
                              'GsPluginEosUpdater error is printed on the '
                              'terminal')
        else:
            self.manual_check('Check there are no EOS updates listed, and no '
                              'GsPluginEosUpdater cancellation error is '
                              'printed on the terminal')
        self.await_state(UpdaterState.ERROR)

    @ddt.data(True, False)
    def test_update_available(self, manually_refresh):
        '''Test that the entire update process works if an update is
        available.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('success', '', '')
        self.dbusmock.SetApplyAction('success', '', '')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        if manually_refresh:
            self.manual_check('Check an EOS update is listed; press the '
                              'Refresh button')

        # TODO: if you proceed through the test slowly, this sometimes doesn’t
        # work
        self.manual_check('Check an EOS update is listed; press the Download '
                          'button')
        self.await_state(UpdaterState.FETCHING)
        self.dbusmock.FinishFetch()

        self.manual_check('Check the download has paused at ~75% complete '
                          '(waiting to apply)')
        self.await_state(UpdaterState.APPLYING_UPDATE)
        self.dbusmock.FinishApply()

        self.manual_check('Check the banner says to ‘Restart Now’ (don’t '
                          'click it)')
        self.await_state(UpdaterState.UPDATE_APPLIED)

    @ddt.data('com.endlessm.Updater.Error.WrongState',
              'com.endlessm.Updater.Error.WrongConfiguration',
              'com.endlessm.Updater.Error.Fetching',
              'com.endlessm.Updater.Error.MalformedAutoinstallSpec',
              'com.endlessm.Updater.Error.UnknownEntryInAutoinstallSpec',
              'com.endlessm.Updater.Error.FlatpakRemoteConflict',
              'com.endlessm.Updater.Error.MeteredConnection',
              'com.endlessm.Updater.Error.Cancelled')
    def test_fetch_early_error(self, error_name):
        '''Test that a D-Bus error return from Fetch() is handled correctly.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('early-error', error_name,
                                     'Some error or other.')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check an EOS update is listed; press the Download '
                          'button')

        if error_name != 'com.endlessm.Updater.Error.Cancelled':
            self.manual_check('Check a fetch error is displayed')
        else:
            self.manual_check('Check no cancellation error is displayed')

        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()
        self.manual_check('Check an EOS update is listed again')

    @ddt.data('com.endlessm.Updater.Error.WrongState',
              'com.endlessm.Updater.Error.WrongConfiguration',
              'com.endlessm.Updater.Error.Fetching',
              'com.endlessm.Updater.Error.MalformedAutoinstallSpec',
              'com.endlessm.Updater.Error.UnknownEntryInAutoinstallSpec',
              'com.endlessm.Updater.Error.FlatpakRemoteConflict',
              'com.endlessm.Updater.Error.MeteredConnection',
              'com.endlessm.Updater.Error.Cancelled')
    def test_fetch_late_error(self, error_name):
        '''Test that a transition to the Error state after successfully calling
        Fetch() is handled correctly.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('late-error', error_name,
                                     'Some error or other.')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check an EOS update is listed; press the Download '
                          'button')
        self.await_state(UpdaterState.FETCHING)
        self.dbusmock.FinishFetch()

        self.await_state(UpdaterState.ERROR)
        if error_name != 'com.endlessm.Updater.Error.Cancelled':
            self.manual_check('Check a fetch error is displayed')
        else:
            self.manual_check('Check no cancellation error is displayed')

        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()
        self.manual_check('Check an EOS update is listed again')

    @ddt.data('com.endlessm.Updater.Error.WrongState',
              'com.endlessm.Updater.Error.WrongConfiguration',
              'com.endlessm.Updater.Error.Cancelled')
    def test_apply_early_error(self, error_name):
        '''Test that a D-Bus error return from Apply() is handled correctly.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('success', '', '')
        self.dbusmock.SetApplyAction('early-error', error_name,
                                     'Some error or other.')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check an EOS update is listed; press the Download '
                          'button')
        self.await_state(UpdaterState.FETCHING)
        self.dbusmock.FinishFetch()

        self.await_state(UpdaterState.ERROR)
        if error_name != 'com.endlessm.Updater.Error.Cancelled':
            self.manual_check('Check an apply error is displayed after the '
                              'update reached ~75% completion')
        else:
            self.manual_check('Check no cancellation error is displayed after '
                              'the update reached ~75% completion')

        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()
        self.manual_check('Check an EOS update is listed again')

    @ddt.data('com.endlessm.Updater.Error.WrongState',
              'com.endlessm.Updater.Error.WrongConfiguration',
              'com.endlessm.Updater.Error.Cancelled')
    def test_apply_late_error(self, error_name):
        '''Test that a transition to the Error state after successfully calling
        Apply() is handled correctly.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('success', '', '')
        self.dbusmock.SetApplyAction('late-error', error_name,
                                     'Some error or other.')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check an EOS update is listed; press the Download '
                          'button')
        self.await_state(UpdaterState.FETCHING)
        self.dbusmock.FinishFetch()

        self.manual_check('Check the download has paused at ~75% complete '
                          '(waiting to apply)')
        self.await_state(UpdaterState.APPLYING_UPDATE)
        self.dbusmock.FinishApply()

        self.await_state(UpdaterState.ERROR)
        if error_name != 'com.endlessm.Updater.Error.Cancelled':
            self.manual_check('Check an apply error is displayed')
        else:
            self.manual_check('Check no cancellation error is displayed')

        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()
        self.manual_check('Check an EOS update is listed again')

    def test_no_eos_updater_running(self):
        '''Test that the plugin doesn’t make a fuss if eos-updater is
        unavailable.'''
        self.p_mock.kill()

        self.launch_lingmo_software()

        self.manual_check('Check there are no EOS updates listed, and no '
                          'errors shown')

    def test_fetch_ui_cancellation(self):
        '''Test that cancelling a download from the UI works correctly.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('success', '', '')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check an EOS update is listed; press the Download '
                          'button, then shortly afterwards press the Cancel '
                          'button')
        self.await_state(UpdaterState.FETCHING)
        self.dbusmock.FinishFetch()

        self.await_state(UpdaterState.ERROR)
        self.manual_check('Check a fetch cancellation error is displayed')

    def test_poll_eos_updater_dies(self):
        '''Test that lingmo-appstore recovers if eos-updater dies while
        polling for updates.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.p_mock.kill()

        self.manual_check('Check no error is shown for the poll failure')
        self.setUp()
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')

        self.manual_check('Press the Refresh button and check an update is '
                          'shown')
        # TODO: It may take a few minutes for the update to appear on the
        # updates page
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

    def test_fetch_eos_updater_dies(self):
        '''Test that lingmo-appstore recovers if eos-updater dies while
        fetching an update.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('success', '', '')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check an EOS update is listed; press the Download '
                          'button')
        self.await_state(UpdaterState.FETCHING)
        self.p_mock.kill()

        self.manual_check('Check an error is shown for the fetch failure')

    def test_apply_eos_updater_dies(self):
        '''Test that lingmo-appstore recovers if eos-updater dies while
        applying an update.'''
        self.dbusmock.SetPollAction(
            'update', dbus.Dictionary({}, signature='sv'), '', '')
        self.dbusmock.SetFetchAction('success', '', '')
        self.dbusmock.SetApplyAction('success', '', '')

        self.launch_lingmo_software()
        self.await_state(UpdaterState.POLLING)
        self.dbusmock.FinishPoll()

        self.manual_check('Check an EOS update is listed; press the Download '
                          'button')
        self.await_state(UpdaterState.FETCHING)
        self.dbusmock.FinishFetch()

        self.manual_check('Check the download has paused at ~75% complete '
                          '(waiting to apply)')
        self.await_state(UpdaterState.APPLYING_UPDATE)
        self.p_mock.kill()

        self.manual_check('Check an error is shown for the apply failure')


if __name__ == '__main__':
    unittest.main()
