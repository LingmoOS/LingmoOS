#!/usr/bin/python3
# -*- coding: utf-8 -*-
#
# Copyright © 2019 Endless Mobile, Inc.
#
# SPDX-License-Identifier: LGPL-2.1-or-later
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Appstore Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Appstore
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301  USA

'''eos-updater mock template

This creates a mock eos-updater interface (com.endlessm.Updater), with several
methods on the Mock sidecar interface which allow its internal state flow to be
controlled.

A typical call chain for this would be:
 - Test harness calls SetPollAction('update', {}, '', '')
 - SUT calls Poll()
 - Test harness calls FinishPoll()
 - SUT calls Fetch()
 - Test harness calls FinishFetch()
 - SUT calls Apply()
 - Test harness calls FinishApply()

Errors can be simulated by specifying an `early-error` or `late-error` as the
action in a Set*Action() call. `early-error` will result in the associated
Poll() call (for example) transitioning to the error state. `late-error` will
result in a transition to the error state only once (for example) FinishPoll()
is called.

See the implementation of each Set*Action() method for the set of actions it
supports.

Usage:
   python3 -m dbusmock \
       --template ./eos-updater/tests/eos_updater.py
'''

from enum import IntEnum
from gi.repository import GLib
import time

import dbus
import dbus.mainloop.glib
from dbusmock import MOCK_IFACE


__author__ = 'Philip Withnall'
__email__ = 'pwithnall@endlessos.org'
__copyright__ = '© 2019 Endless Mobile Inc.'
__license__ = 'LGPL 2.1+'


class UpdaterState(IntEnum):
    NONE = 0
    READY = 1
    ERROR = 2
    POLLING = 3
    UPDATE_AVAILABLE = 4
    FETCHING = 5
    UPDATE_READY = 6
    APPLYING_UPDATE = 7
    UPDATE_APPLIED = 8


BUS_NAME = 'com.endlessm.Updater'
MAIN_OBJ = '/com/endlessm/Updater'
MAIN_IFACE = 'com.endlessm.Updater'
SYSTEM_BUS = True


dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)


def load(mock, parameters):
    mock.AddProperties(
        MAIN_IFACE,
        dbus.Dictionary({
            'State': dbus.UInt32(parameters.get('State', 1)),
            'UpdateID': dbus.String(parameters.get('UpdateID', '')),
            'UpdateRefspec': dbus.String(parameters.get('UpdateRefspec', '')),
            'OriginalRefspec':
                dbus.String(parameters.get('OriginalRefspec', '')),
            'CurrentID': dbus.String(parameters.get('CurrentID', '')),
            'UpdateLabel': dbus.String(parameters.get('UpdateLabel', '')),
            'UpdateMessage': dbus.String(parameters.get('UpdateMessage', '')),
            'Version': dbus.String(parameters.get('Version', '')),
            'UpdateIsUserVisible':
                dbus.Boolean(parameters.get('UpdateIsUserVisible', False)),
            'ReleaseNotesUri':
                dbus.String(parameters.get('ReleaseNotesUri', '')),
            'DownloadSize': dbus.Int64(parameters.get('DownloadSize', 0)),
            'DownloadedBytes':
                dbus.Int64(parameters.get('DownloadedBytes', 0)),
            'UnpackedSize': dbus.Int64(parameters.get('UnpackedSize', 0)),
            'FullDownloadSize':
                dbus.Int64(parameters.get('FullDownloadSize', 0)),
            'FullUnpackedSize':
                dbus.Int64(parameters.get('FullUnpackedSize', 0)),
            'ErrorCode': dbus.UInt32(parameters.get('ErrorCode', 0)),
            'ErrorName': dbus.String(parameters.get('ErrorName', '')),
            'ErrorMessage': dbus.String(parameters.get('ErrorMessage', '')),
        }, signature='sv'))

    # Set up initial state
    mock.__poll_action = 'no-update'
    mock.__fetch_action = 'success'
    mock.__apply_action = 'success'

    # Set up private methods
    mock.__set_properties = __set_properties
    mock.__change_state = __change_state
    mock.__set_error = __set_error
    mock.__check_state = __check_state


#
# Internal utility methods
#

# Values in @properties must have variant_level≥1
def __set_properties(self, iface, properties):
    for key, value in properties.items():
        self.props[iface][key] = value
    self.EmitSignal(dbus.PROPERTIES_IFACE, 'PropertiesChanged', 'sa{sv}as', [
        iface,
        properties,
        [],
    ])


def __change_state(self, new_state):
    props = {
        'State': dbus.UInt32(new_state, variant_level=1)
    }

    # Reset error state if necessary.
    if new_state != UpdaterState.ERROR and \
       self.props[MAIN_IFACE]['ErrorName'] != '':
        props['ErrorCode'] = dbus.UInt32(0, variant_level=1)
        props['ErrorName'] = dbus.String('', variant_level=1)
        props['ErrorMessage'] = dbus.String('', variant_level=1)

    self.__set_properties(self, MAIN_IFACE, props)
    self.EmitSignal(MAIN_IFACE, 'StateChanged', 'u', [dbus.UInt32(new_state)])


def __set_error(self, error_name, error_message):
    assert(error_name != '')

    self.__set_properties(self, MAIN_IFACE, {
        'ErrorName': dbus.String(error_name, variant_level=1),
        'ErrorMessage': dbus.String(error_message, variant_level=1),
        'ErrorCode': dbus.UInt32(1, variant_level=1),
    })
    self.__change_state(self, UpdaterState.ERROR)


def __check_state(self, allowed_states):
    if self.props[MAIN_IFACE]['State'] not in allowed_states:
        raise dbus.exceptions.DBusException(
            'Call not allowed in this state',
            name='com.endlessm.Updater.Error.WrongState')


#
# Updater methods which are too big for squeezing into AddMethod()
#

@dbus.service.method(MAIN_IFACE, in_signature='', out_signature='')
def Poll(self):
    self.__check_state(self, set([
        UpdaterState.READY,
        UpdaterState.UPDATE_AVAILABLE,
        UpdaterState.UPDATE_READY,
        UpdaterState.ERROR,
    ]))

    self.__change_state(self, UpdaterState.POLLING)

    if self.__poll_action == 'early-error':
        # Simulate some network polling activity
        time.sleep(0.5)

        self.__set_error(self, self.__poll_error_name,
                         self.__poll_error_message)
    else:
        # we now expect the test harness to call FinishPoll() on the mock
        # interface
        pass


@dbus.service.method(MAIN_IFACE, in_signature='s', out_signature='')
def PollVolume(self, path):
    # FIXME: Currently unsupported
    return self.Poll()


@dbus.service.method(MAIN_IFACE, in_signature='', out_signature='')
def Fetch(self):
    return self.FetchFull()


@dbus.service.method(MAIN_IFACE, in_signature='a{sv}', out_signature='')
def FetchFull(self, options=None):
    self.__check_state(self, set([UpdaterState.UPDATE_AVAILABLE]))

    self.__change_state(self, UpdaterState.FETCHING)

    if self.__fetch_action == 'early-error':
        # Simulate some network fetching activity
        time.sleep(0.5)

        self.__set_error(self, self.__fetch_error_name,
                         self.__fetch_error_message)
    else:
        # we now expect the test harness to call FinishFetch() on the mock
        # interface
        pass


@dbus.service.method(MAIN_IFACE, in_signature='', out_signature='')
def Apply(self):
    self.__check_state(self, set([UpdaterState.UPDATE_READY]))

    self.__change_state(self, UpdaterState.APPLYING_UPDATE)

    if self.__apply_action == 'early-error':
        # Simulate some disk applying activity
        time.sleep(0.5)

        self.__set_error(self, self.__apply_error_name,
                         self.__apply_error_message)
    else:
        # we now expect the test harness to call FinishApply() on the mock
        # interface
        pass


@dbus.service.method(MAIN_IFACE, in_signature='', out_signature='')
def Cancel(self):
    self.__check_state(self, set([
        UpdaterState.POLLING,
        UpdaterState.FETCHING,
        UpdaterState.APPLYING_UPDATE,
    ]))

    # Simulate some work to cancel whatever’s happening
    time.sleep(1)

    self.__set_error(self, 'com.endlessm.Updater.Error.Cancelled',
                     'Update was cancelled')


#
# Convenience methods on the mock
#

@dbus.service.method(MOCK_IFACE, in_signature='sa{sv}ss', out_signature='')
def SetPollAction(self, action, update_properties, error_name, error_message):
    '''Set the action to happen when the SUT calls Poll().

    This sets the action which will happen when Poll() (and subsequently
    FinishPoll()) are called, including the details of the error which will be
    returned or the new update which will be advertised.
    '''
    # Provide a default update.
    if not update_properties:
        update_properties = {
            'UpdateID': dbus.String('f' * 64, variant_level=1),
            'UpdateRefspec':
                dbus.String('remote:new-refspec', variant_level=1),
            'OriginalRefspec':
                dbus.String('remote:old-refspec', variant_level=1),
            'CurrentID': dbus.String('1' * 64, variant_level=1),
            'UpdateLabel': dbus.String('New OS Update', variant_level=1),
            'UpdateMessage':
                dbus.String('Some release notes.', variant_level=1),
            'Version': dbus.String('3.7.0', variant_level=1),
            'UpdateIsUserVisible': dbus.Boolean(False, variant_level=1),
            'ReleaseNotesUri':
                dbus.String('https://example.com/release-notes', variant_level=1),
            'DownloadSize': dbus.Int64(1000000000, variant_level=1),
            'UnpackedSize': dbus.Int64(1500000000, variant_level=1),
            'FullDownloadSize': dbus.Int64(1000000000 * 0.8, variant_level=1),
            'FullUnpackedSize': dbus.Int64(1500000000 * 0.8, variant_level=1),
        }

    self.__poll_action = action
    self.__poll_update_properties = update_properties
    self.__poll_error_name = error_name
    self.__poll_error_message = error_message


@dbus.service.method(MOCK_IFACE, in_signature='', out_signature='')
def FinishPoll(self):
    self.__check_state(self, set([UpdaterState.POLLING]))

    if self.__poll_action == 'no-update':
        self.__change_state(self, UpdaterState.READY)
    elif self.__poll_action == 'update':
        assert(set([
            'UpdateID',
            'UpdateRefspec',
            'OriginalRefspec',
            'CurrentID',
            'UpdateLabel',
            'UpdateMessage',
            'Version',
            'UpdateIsUserVisible',
            'ReleaseNotesUri',
            'FullDownloadSize',
            'FullUnpackedSize',
            'DownloadSize',
            'UnpackedSize',
        ]) <= set(self.__poll_update_properties.keys()))

        # Set the initial DownloadedBytes based on whether we know the full
        # download size.
        props = self.__poll_update_properties
        if props['DownloadSize'] < 0:
            props['DownloadedBytes'] = dbus.Int64(-1, variant_level=1)
        else:
            props['DownloadedBytes'] = dbus.Int64(0, variant_level=1)

        self.__set_properties(self, MAIN_IFACE, props)
        self.__change_state(self, UpdaterState.UPDATE_AVAILABLE)
    elif self.__poll_action == 'early-error':
        # Handled in Poll() itself.
        pass
    elif self.__poll_action == 'late-error':
        self.__set_error(self, self.__poll_error_name,
                         self.__poll_error_message)
    else:
        assert(False)


@dbus.service.method(MOCK_IFACE, in_signature='sss', out_signature='')
def SetFetchAction(self, action, error_name, error_message):
    '''Set the action to happen when the SUT calls Fetch().

    This sets the action which will happen when Fetch() (and subsequently
    FinishFetch()) are called, including the details of the error which will be
    returned, if applicable.
    '''
    self.__fetch_action = action
    self.__fetch_error_name = error_name
    self.__fetch_error_message = error_message


@dbus.service.method(MOCK_IFACE, in_signature='', out_signature='',
                     async_callbacks=('success_cb', 'error_cb'))
def FinishFetch(self, success_cb, error_cb):
    '''Finish a pending client call to Fetch().

    This is implemented using async_callbacks since if the fetch action is
    ‘success’ it will block until the simulated download is complete, emitting
    download progress signals throughout. As it’s implemented asynchronously,
    this allows any calls to Cancel() to be handled by the mock service
    part-way through the fetch.
    '''
    self.__check_state(self, set([UpdaterState.FETCHING]))

    if self.__fetch_action == 'success':
        # Simulate the download.
        i = 0
        download_size = self.props[MAIN_IFACE]['DownloadSize']

        def _download_progress_cb():
            nonlocal i

            # Allow cancellation.
            if self.props[MAIN_IFACE]['State'] != UpdaterState.FETCHING:
                return False

            downloaded_bytes = (i / 100.0) * download_size
            self.__set_properties(self, MAIN_IFACE, {
                'DownloadedBytes':
                    dbus.Int64(downloaded_bytes, variant_level=1),
            })

            i += 1

            # Keep looping until the download is complete.
            if i <= 100:
                return True

            # When the download is complete, change the service state and
            # finish the asynchronous FinishFetch() call.
            self.__change_state(self, UpdaterState.UPDATE_READY)
            success_cb()
            return False

        GLib.timeout_add(100, _download_progress_cb)
    elif self.__fetch_action == 'early-error':
        # Handled in Fetch() itself.
        success_cb()
    elif self.__fetch_action == 'late-error':
        self.__set_error(self, self.__fetch_error_name,
                         self.__fetch_error_message)
        success_cb()
    else:
        assert(False)


@dbus.service.method(MOCK_IFACE, in_signature='sss', out_signature='')
def SetApplyAction(self, action, error_name, error_message):
    '''Set the action to happen when the SUT calls Apply().

    This sets the action which will happen when Apply() (and subsequently
    FinishApply()) are called, including the details of the error which will be
    returned, if applicable.
    '''
    self.__apply_action = action
    self.__apply_error_name = error_name
    self.__apply_error_message = error_message


@dbus.service.method(MOCK_IFACE, in_signature='', out_signature='')
def FinishApply(self):
    self.__check_state(self, set([UpdaterState.APPLYING_UPDATE]))

    if self.__apply_action == 'success':
        self.__change_state(self, UpdaterState.UPDATE_APPLIED)
    elif self.__apply_action == 'early-error':
        # Handled in Apply() itself.
        pass
    elif self.__apply_action == 'late-error':
        self.__set_error(self, self.__apply_error_name,
                         self.__apply_error_message)
    else:
        assert(False)
