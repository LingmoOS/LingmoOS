/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <string.h>
#include <glib/gi18n.h>

#include "gs-fwupd-app.h"

const gchar *
gs_fwupd_app_get_device_id (GsApp *app)
{
	return gs_app_get_metadata_item (app, "fwupd::DeviceID");
}

const gchar *
gs_fwupd_app_get_update_uri (GsApp *app)
{
	return gs_app_get_metadata_item (app, "fwupd::UpdateID");
}

gboolean
gs_fwupd_app_get_is_locked (GsApp *app)
{
	GVariant *tmp = gs_app_get_metadata_variant (app, "fwupd::IsLocked");
	if (tmp == NULL)
		return FALSE;
	return g_variant_get_boolean (tmp);
}

void
gs_fwupd_app_set_device_id (GsApp *app, const gchar *device_id)
{
	gs_app_set_metadata (app, "fwupd::DeviceID", device_id);
}

void
gs_fwupd_app_set_update_uri (GsApp *app, const gchar *update_uri)
{
	gs_app_set_metadata (app, "fwupd::UpdateID", update_uri);
}

void
gs_fwupd_app_set_is_locked (GsApp *app, gboolean is_locked)
{
	g_autoptr(GVariant) tmp = g_variant_new_boolean (is_locked);
	gs_app_set_metadata_variant (app, "fwupd::IsLocked", tmp);
}

#if FWUPD_CHECK_VERSION(1, 8, 1)
static gchar * /* (transfer full) */
gs_fwupd_problem_to_string (FwupdClient *client,
			    FwupdDevice *dev,
			    FwupdDeviceProblem problem)
{
	if (problem == FWUPD_DEVICE_PROBLEM_SYSTEM_POWER_TOO_LOW) {
		if (fwupd_client_get_battery_level (client) == FWUPD_BATTERY_LEVEL_INVALID ||
		    fwupd_client_get_battery_threshold (client) == FWUPD_BATTERY_LEVEL_INVALID) {
			/* TRANSLATORS: as in laptop battery power */
			return g_strdup (_("System power is too low to perform the update"));
		}
		return g_strdup_printf (
		    /* TRANSLATORS: as in laptop battery power */
		    _("System power is too low to perform the update (%u%%, requires %u%%)"),
		    fwupd_client_get_battery_level (client),
		    fwupd_client_get_battery_threshold (client));
	}
	if (problem == FWUPD_DEVICE_PROBLEM_UNREACHABLE) {
		/* TRANSLATORS: for example, a Bluetooth mouse that is in powersave mode */
		return g_strdup (_("Device is unreachable, or out of wireless range"));
	}
	if (problem == FWUPD_DEVICE_PROBLEM_POWER_TOO_LOW) {
		if (fwupd_device_get_battery_level (dev) == FWUPD_BATTERY_LEVEL_INVALID ||
		    fwupd_device_get_battery_threshold (dev) == FWUPD_BATTERY_LEVEL_INVALID) {
			/* TRANSLATORS: for example the batteries *inside* the Bluetooth mouse */
			return g_strdup_printf (_("Device battery power is too low"));
		}
		/* TRANSLATORS: for example the batteries *inside* the Bluetooth mouse */
		return g_strdup_printf (_("Device battery power is too low (%u%%, requires %u%%)"),
				        fwupd_device_get_battery_level (dev),
				        fwupd_device_get_battery_threshold (dev));
	}
	if (problem == FWUPD_DEVICE_PROBLEM_UPDATE_PENDING) {
		/* TRANSLATORS: usually this is when we're waiting for a reboot */
		return g_strdup (_("Device is waiting for the update to be applied"));
	}
	if (problem == FWUPD_DEVICE_PROBLEM_REQUIRE_AC_POWER) {
		/* TRANSLATORS: as in, wired mains power for a laptop */
		return g_strdup (_("Device requires AC power to be connected"));
	}
	if (problem == FWUPD_DEVICE_PROBLEM_LID_IS_CLOSED) {
		/* TRANSLATORS: lid means "laptop top cover" */
		return g_strdup (_("Device cannot be used while the lid is closed"));
	}
	return NULL;
}
#endif

void
gs_fwupd_app_set_from_device (GsApp *app,
			      FwupdClient *client,
			      FwupdDevice *dev)
{
	GPtrArray *guids;

	/* something can be done */
	if (fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_UPDATABLE)
#if FWUPD_CHECK_VERSION(1, 8, 1)
	    || fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_UPDATABLE_HIDDEN)
#endif
	    )
		gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);

	/* only can be applied in systemd-offline */
	if (fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_ONLY_OFFLINE))
		gs_app_set_metadata (app, "fwupd::OnlyOffline", "");


	/* reboot required to apply update */
	if (fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_NEEDS_REBOOT))
		gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT);

	/* is removable or cannot be used during update */
	if (!fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_INTERNAL) ||
	    !fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_USABLE_DURING_UPDATE))
		gs_app_add_quirk (app, GS_APP_QUIRK_UNUSABLE_DURING_UPDATE);

	guids = fwupd_device_get_guids (dev);
	if (guids->len > 0) {
		g_autofree gchar *guid_str = NULL;
		g_auto(GStrv) tmp = g_new0 (gchar *, guids->len + 1);
		for (guint i = 0; i < guids->len; i++)
			tmp[i] = g_strdup (g_ptr_array_index (guids, i));
		guid_str = g_strjoinv (",", tmp);
		gs_app_set_metadata (app, "fwupd::Guid", guid_str);
	}
	if (fwupd_device_get_name (dev) != NULL) {
		g_autofree gchar *vendor_name = NULL;
		if (fwupd_device_get_vendor (dev) == NULL ||
		    g_str_has_prefix (fwupd_device_get_name (dev),
				      fwupd_device_get_vendor (dev))) {
			vendor_name = g_strdup (fwupd_device_get_name (dev));
		} else {
			vendor_name = g_strdup_printf ("%s %s",
						       fwupd_device_get_vendor (dev),
						       fwupd_device_get_name (dev));
		}
		gs_app_set_name (app, GS_APP_QUALITY_NORMAL, vendor_name);
	}
	if (fwupd_device_get_summary (dev) != NULL) {
		gs_app_set_summary (app, GS_APP_QUALITY_NORMAL,
				    fwupd_device_get_summary (dev));
	}
	if (fwupd_device_get_version (dev) != NULL) {
		gs_app_set_version (app, fwupd_device_get_version (dev));
	}
	if (fwupd_device_get_created (dev) != 0)
		gs_app_set_install_date (app, fwupd_device_get_created (dev));

#if FWUPD_CHECK_VERSION(1, 8, 1)
	if (fwupd_device_get_problems (dev) != FWUPD_DEVICE_PROBLEM_NONE) {
		g_autoptr(GString) problems = g_string_new (NULL);
		for (guint i = 0; i < sizeof (FwupdDeviceProblem) * 8; i++) {
			FwupdDeviceProblem problem = 1ull << i;
			g_autofree gchar *tmp = NULL;
			if (!fwupd_device_has_problem (dev, problem))
				continue;
			tmp = gs_fwupd_problem_to_string (client, dev, problem);
			if (tmp == NULL)
				continue;
			if (problems->len)
				g_string_append_c (problems, '\n');
			g_string_append (problems, tmp);
		}
		if (problems->len)
			gs_app_set_metadata (app, "LingmoAppstore::problems", problems->str);
		else
			gs_app_set_metadata (app, "LingmoAppstore::problems", NULL);
	} else {
		gs_app_set_metadata (app, "LingmoAppstore::problems", NULL);
	}
#endif

	/* needs action */
	if (fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_NEEDS_BOOTLOADER)
#if FWUPD_CHECK_VERSION(1, 8, 1)
	    || fwupd_device_has_flag (dev, FWUPD_DEVICE_FLAG_UPDATABLE_HIDDEN)
	    || fwupd_device_get_problems (dev) != FWUPD_DEVICE_PROBLEM_NONE
#endif
	   )
		gs_app_add_quirk (app, GS_APP_QUIRK_NEEDS_USER_ACTION);
	else
		gs_app_remove_quirk (app, GS_APP_QUIRK_NEEDS_USER_ACTION);
}

static gchar *
gs_fwupd_release_get_name (FwupdRelease *release)
{
	const gchar *name = fwupd_release_get_name (release);
	GPtrArray *cats = fwupd_release_get_categories (release);

	for (guint i = 0; i < cats->len; i++) {
		const gchar *cat = g_ptr_array_index (cats, i);
		if (g_strcmp0 (cat, "X-Device") == 0) {
			/* TRANSLATORS: a specific part of hardware,
			 * the first %s is the device name, e.g. 'Unifying Receiver` */
			return g_strdup_printf (_("%s Device Update"), name);
		}
		if (g_strcmp0 (cat, "X-System") == 0) {
			/* TRANSLATORS: the entire system, e.g. all internal devices,
			 * the first %s is the device name, e.g. 'ThinkPad P50` */
			return g_strdup_printf (_("%s System Update"), name);
		}
		if (g_strcmp0 (cat, "X-EmbeddedController") == 0) {
			/* TRANSLATORS: the EC is typically the keyboard controller chip,
			 * the first %s is the device name, e.g. 'ThinkPad P50` */
			return g_strdup_printf (_("%s Embedded Controller Update"), name);
		}
		if (g_strcmp0 (cat, "X-ManagementEngine") == 0) {
			/* TRANSLATORS: ME stands for Management Engine, the Intel AMT thing,
			 * the first %s is the device name, e.g. 'ThinkPad P50` */
			return g_strdup_printf (_("%s ME Update"), name);
		}
		if (g_strcmp0 (cat, "X-CorporateManagementEngine") == 0) {
			/* TRANSLATORS: ME stands for Management Engine (with Intel AMT),
			 * where the first %s is the device name, e.g. 'ThinkPad P50` */
			return g_strdup_printf (_("%s Corporate ME Update"), name);
		}
		if (g_strcmp0 (cat, "X-ConsumerManagementEngine") == 0) {
			/* TRANSLATORS: ME stands for Management Engine, where
			 * the first %s is the device name, e.g. 'ThinkPad P50` */
			return g_strdup_printf (_("%s Consumer ME Update"), name);
		}
		if (g_strcmp0 (cat, "X-Controller") == 0) {
			/* TRANSLATORS: the controller is a device that has other devices
			 * plugged into it, for example ThunderBolt, FireWire or USB,
			 * the first %s is the device name, e.g. 'Intel ThunderBolt` */
			return g_strdup_printf (_("%s Controller Update"), name);
		}
		if (g_strcmp0 (cat, "X-ThunderboltController") == 0) {
			/* TRANSLATORS: the Thunderbolt controller is a device that
			 * has other high speed Thunderbolt devices plugged into it;
			 * the first %s is the system name, e.g. 'ThinkPad P50` */
			return g_strdup_printf (_("%s Thunderbolt Controller Update"), name);
		}
		if (g_strcmp0 (cat, "X-CpuMicrocode") == 0) {
			/* TRANSLATORS: the CPU microcode is firmware loaded onto the CPU
			 * at system bootup */
			return g_strdup_printf (_("%s CPU Microcode Update"), name);
		}
		if (g_strcmp0 (cat, "X-Configuration") == 0) {
			/* TRANSLATORS: configuration refers to hardware state,
			 * e.g. a security database or a default power value */
			return g_strdup_printf (_("%s Configuration Update"), name);
		}
		if (g_strcmp0 (cat, "X-Battery") == 0) {
			/* TRANSLATORS: battery refers to the system power source */
			return g_strdup_printf (_("%s Battery Update"), name);
		}
		if (g_strcmp0 (cat, "X-Camera") == 0) {
			/* TRANSLATORS: camera can refer to the laptop internal
			 * camera in the bezel or external USB webcam */
			return g_strdup_printf (_("%s Camera Update"), name);
		}
		if (g_strcmp0 (cat, "X-TPM") == 0) {
			/* TRANSLATORS: TPM refers to a Trusted Platform Module */
			return g_strdup_printf (_("%s TPM Update"), name);
		}
		if (g_strcmp0 (cat, "X-Touchpad") == 0) {
			/* TRANSLATORS: TouchPad refers to a flat input device */
			return g_strdup_printf (_("%s Touchpad Update"), name);
		}
		if (g_strcmp0 (cat, "X-Mouse") == 0) {
			/* TRANSLATORS: Mouse refers to a handheld input device */
			return g_strdup_printf (_("%s Mouse Update"), name);
		}
		if (g_strcmp0 (cat, "X-Keyboard") == 0) {
			/* TRANSLATORS: Keyboard refers to an input device for typing */
			return g_strdup_printf (_("%s Keyboard Update"), name);
		}
		if (g_strcmp0 (cat, "X-StorageController") == 0) {
			/* TRANSLATORS: Storage Controller is typically a RAID or SAS adapter */
			return g_strdup_printf (_("%s Storage Controller Update"), name);
		}
		if (g_strcmp0 (cat, "X-NetworkInterface") == 0) {
			/* TRANSLATORS: Network Interface refers to the physical
			 * PCI card, not the logical wired connection */
			return g_strdup_printf (_("%s Network Interface Update"), name);
		}
		if (g_strcmp0 (cat, "X-VideoDisplay") == 0) {
			/* TRANSLATORS: Video Display refers to the laptop internal display or
			 * external monitor */
			return g_strdup_printf (_("%s Display Update"), name);
		}
		if (g_strcmp0 (cat, "X-BaseboardManagementController") == 0) {
			/* TRANSLATORS: BMC refers to baseboard management controller which
			 * is the device that updates all the other firmware on the system */
			return g_strdup_printf (_("%s BMC Update"), name);
		}
		if (g_strcmp0 (cat, "X-UsbReceiver") == 0) {
			/* TRANSLATORS: Receiver refers to a radio device, e.g. a tiny Bluetooth
			 * device that stays in the USB port so the wireless peripheral works */
			return g_strdup_printf (_("%s USB Receiver Update"), name);
		}
		if (g_strcmp0 (cat, "X-Drive") == 0) {
			/* TRANSLATORS: drive refers to a storage device, e.g. SATA disk */
			return g_strdup_printf (_("%s Drive Update"), name);
		}
		if (g_strcmp0 (cat, "X-FlashDrive") == 0) {
			/* TRANSLATORS: flash refers to solid state storage, e.g. UFS or eMMC */
			return g_strdup_printf (_("%s Flash Drive Update"), name);
		}
		if (g_strcmp0 (cat, "X-SolidStateDrive") == 0) {
			/* TRANSLATORS: SSD refers to a Solid State Drive, e.g. non-rotating
			 * SATA or NVMe disk */
			return g_strdup_printf (_("%s SSD Update"), name);
		}
		if (g_strcmp0 (cat, "X-Gpu") == 0) {
			/* TRANSLATORS: GPU refers to a Graphics Processing Unit, e.g.
			 * the "video card" */
			return g_strdup_printf (_("%s GPU Update"), name);
		}
		if (g_strcmp0 (cat, "X-Dock") == 0) {
			/* TRANSLATORS: Dock refers to the port replicator hardware laptops are
			 * cradled in, or lowered onto */
			return g_strdup_printf (_("%s Dock Update"), name);
		}
		if (g_strcmp0 (cat, "X-UsbDock") == 0) {
			/* TRANSLATORS: Dock refers to the port replicator device connected
			 * by plugging in a USB cable -- which may or may not also provide power */
			return g_strdup_printf (_("%s USB Dock Update"), name);
		}
	}

	/* default fallback */
	return g_strdup (name);
}

static AsUrgencyKind
gs_fwupd_release_urgency_to_as_urgency_kind (FwupdReleaseUrgency urgency)
{
	switch (urgency) {
	case FWUPD_RELEASE_URGENCY_LOW:
		return AS_URGENCY_KIND_LOW;
	case FWUPD_RELEASE_URGENCY_MEDIUM:
		return AS_URGENCY_KIND_MEDIUM;
	case FWUPD_RELEASE_URGENCY_HIGH:
		return AS_URGENCY_KIND_HIGH;
	case FWUPD_RELEASE_URGENCY_CRITICAL:
		return AS_URGENCY_KIND_CRITICAL;
	case FWUPD_RELEASE_URGENCY_UNKNOWN:
	default:
		return AS_URGENCY_KIND_UNKNOWN;
	}
}

void
gs_fwupd_app_set_from_release (GsApp *app, FwupdRelease *rel)
{
	GPtrArray *locations = fwupd_release_get_locations (rel);

	if (fwupd_release_get_name (rel) != NULL) {
		g_autofree gchar *tmp = gs_fwupd_release_get_name (rel);
		gs_app_set_name (app, GS_APP_QUALITY_NORMAL, tmp);
	}
	if (fwupd_release_get_summary (rel) != NULL) {
		gs_app_set_summary (app, GS_APP_QUALITY_NORMAL,
				    fwupd_release_get_summary (rel));
	}
	if (fwupd_release_get_homepage (rel) != NULL) {
		gs_app_set_url (app, AS_URL_KIND_HOMEPAGE,
				fwupd_release_get_homepage (rel));
	}
	if (fwupd_release_get_size (rel) != 0) {
		gs_app_set_size_installed (app, GS_SIZE_TYPE_VALID, 0);
		gs_app_set_size_download (app, GS_SIZE_TYPE_VALID, fwupd_release_get_size (rel));
	}
	if (fwupd_release_get_version (rel) != NULL)
		gs_app_set_update_version (app, fwupd_release_get_version (rel));
	if (fwupd_release_get_license (rel) != NULL) {
		gs_app_set_license (app, GS_APP_QUALITY_NORMAL,
				    fwupd_release_get_license (rel));
	}
	if (locations->len > 0) {
		const gchar *uri = g_ptr_array_index (locations, 0);
		/* typically the first URI will be the main HTTP mirror, and we
		 * don't have the capability to use an IPFS/IPNS URL anyway */
		gs_app_set_origin_hostname (app, uri);
		gs_fwupd_app_set_update_uri (app, uri);
	}
	if (fwupd_release_get_description (rel) != NULL) {
		g_autofree gchar *tmp = NULL;
#if AS_CHECK_VERSION(1, 0, 0)
		tmp = as_markup_convert (fwupd_release_get_description (rel), AS_MARKUP_KIND_TEXT, NULL);
#else
		tmp = as_markup_convert_simple (fwupd_release_get_description (rel), NULL);
#endif
		if (tmp != NULL)
			gs_app_set_update_details_text (app, tmp);
	}
	if (fwupd_release_get_detach_image (rel) != NULL) {
		g_autoptr(AsScreenshot) ss = as_screenshot_new ();
		g_autoptr(AsImage) im = as_image_new ();
		as_image_set_kind (im, AS_IMAGE_KIND_SOURCE);
		as_image_set_url (im, fwupd_release_get_detach_image (rel));
		as_screenshot_set_kind (ss, AS_SCREENSHOT_KIND_DEFAULT);
		as_screenshot_add_image (ss, im);
		if (fwupd_release_get_detach_caption (rel) != NULL)
			as_screenshot_set_caption (ss, fwupd_release_get_detach_caption (rel), NULL);
		gs_app_set_action_screenshot (app, ss);
	}

	gs_app_set_update_urgency (app, gs_fwupd_release_urgency_to_as_urgency_kind (fwupd_release_get_urgency (rel)));
}
