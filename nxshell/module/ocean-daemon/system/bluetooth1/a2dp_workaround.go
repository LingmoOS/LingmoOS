// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

package bluetooth

func (b *SysBluetooth) disconnectA2DPDeviceExcept(d *device) {
	for _, devices := range b.devices {
		for _, device := range devices {
			if device == nil || device.Path == d.Path {
				continue
			}
			for _, uuid := range device.UUIDs {
				if uuid == A2DP_SINK_UUID && device.connected {
					logger.Infof("disconnect A2DP %s", device)
					device.Disconnect()
				}
			}
		}
	}
}
