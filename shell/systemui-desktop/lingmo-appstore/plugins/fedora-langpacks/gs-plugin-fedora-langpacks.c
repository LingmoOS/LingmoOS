/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2019 Sundeep Anand <suanand@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This plugin does following..
 *  1. locates the active locale, say, xx
 *  2. identifies related langpacks-xx
 *  3. tries to add langpack-xx in app list
 *  4. logs install information; not to try again
 */

#include <lingmo-appstore.h>

#include "gs-plugin-fedora-langpacks.h"

struct _GsPluginFedoraLangpacks {
	GsPlugin	 parent;

	GHashTable	*locale_langpack_map;
};

G_DEFINE_TYPE (GsPluginFedoraLangpacks, gs_plugin_fedora_langpacks, GS_TYPE_PLUGIN)

static void
gs_plugin_fedora_langpacks_init (GsPluginFedoraLangpacks *self)
{
	GsPlugin *plugin = GS_PLUGIN (self);

	/* this plugin should be fedora specific */
	if (!gs_plugin_check_distro_id (plugin, "fedora")) {
		gs_plugin_set_enabled (plugin, FALSE);
		return;
	}

	gs_plugin_add_rule (plugin, GS_PLUGIN_RULE_RUN_AFTER, "appstream");

	/*
	* A few language code may have more than one language packs.
	* Example: en {en_GB}, pt {pt_BR}, zh {zh_CN, zh_TW, zh_HK}
	*/
	self->locale_langpack_map = g_hash_table_new (g_str_hash, g_str_equal);
	g_hash_table_insert (self->locale_langpack_map, (gpointer) "en_GB", (gpointer) "langpacks-en_GB");
	g_hash_table_insert (self->locale_langpack_map, (gpointer) "pt_BR", (gpointer) "langpacks-pt_BR");
	g_hash_table_insert (self->locale_langpack_map, (gpointer) "zh_CN", (gpointer) "langpacks-zh_CN");
	g_hash_table_insert (self->locale_langpack_map, (gpointer) "zh_TW", (gpointer) "langpacks-zh_TW");
	g_hash_table_insert (self->locale_langpack_map, (gpointer) "zh_HK", (gpointer) "langpacks-zh_HK");
}

static void
gs_plugin_fedora_langpacks_dispose (GObject *object)
{
	GsPluginFedoraLangpacks *self = GS_PLUGIN_FEDORA_LANGPACKS (object);

	g_clear_pointer (&self->locale_langpack_map, g_hash_table_unref);

	G_OBJECT_CLASS (gs_plugin_fedora_langpacks_parent_class)->dispose (object);
}

gboolean
gs_plugin_add_langpacks (GsPlugin *plugin,
			 GsAppList *list,
			 const gchar *locale,
			 GCancellable *cancellable,
			 GError **error)
{
	GsPluginFedoraLangpacks *self = GS_PLUGIN_FEDORA_LANGPACKS (plugin);
	gchar *separator;
	const gchar *language_code;
	g_autofree gchar *cachefn = NULL;
	g_autofree gchar *langpack_pkgname = NULL;
	g_auto(GStrv) language_region = NULL;

	/* This plugin may receive user locale in the form as documented in `man 3 setlocale`:
	 *
	 * language[_territory][.codeset][@modifier]
	 *
	 * e.g. `ja_JP.UTF-8` or `en_GB.iso88591` or `uz_UZ.utf8@cyrillic` or `de_DE@euro`
	 * Get the locale without codeset and modifier as required for langpacks.
	 */
	separator = strpbrk (locale, ".@");
	if (separator != NULL)
		*separator = '\0';

	if (g_strrstr (locale, "_") != NULL &&
	    !g_hash_table_lookup (self->locale_langpack_map, locale)) {
		/*
		 * language_code should be the langpack_source_id
		 * if input language_code is a locale and it doesn't
		 * not found in locale_langpack_map
		 */
		language_region = g_strsplit (locale, "_", 2);
		language_code = language_region[0];
	} else {
		language_code = locale;
	}

	/* per-user cache */
	langpack_pkgname = g_strconcat ("langpacks-", language_code, NULL);
	cachefn = gs_utils_get_cache_filename ("langpacks", langpack_pkgname,
					       GS_UTILS_CACHE_FLAG_WRITEABLE |
					       GS_UTILS_CACHE_FLAG_CREATE_DIRECTORY,
					       error);
	if (cachefn == NULL)
		return FALSE;
	if (!g_file_test (cachefn, G_FILE_TEST_EXISTS)) {
		g_autoptr(GsApp) app = gs_app_new (NULL);
		gs_app_set_metadata (app, "LingmoAppstore::Creator", gs_plugin_get_name (plugin));
		gs_app_set_kind (app, AS_COMPONENT_KIND_LOCALIZATION);
		gs_app_set_bundle_kind (app, AS_BUNDLE_KIND_PACKAGE);
		gs_app_set_scope (app, AS_COMPONENT_SCOPE_SYSTEM);
		gs_app_add_source (app, langpack_pkgname);
		gs_app_list_add (list, app);

		/* ensure we do not keep trying to install the langpack */
		if (!g_file_set_contents (cachefn, language_code, -1, error))
			return FALSE;
	}

	return TRUE;
}

static void
gs_plugin_fedora_langpacks_class_init (GsPluginFedoraLangpacksClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gs_plugin_fedora_langpacks_dispose;
}

GType
gs_plugin_query_type (void)
{
	return GS_TYPE_PLUGIN_FEDORA_LANGPACKS;
}
