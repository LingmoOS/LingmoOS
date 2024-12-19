/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2017 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "lingmo-appstore-private.h"

#include "gs-markdown.h"
#include "gs-test.h"

static void
gs_markdown_func (void)
{
	gchar *text;
	const gchar *markdown;
	const gchar *markdown_expected;
	g_autoptr(GsMarkdown) md = NULL;

	/* get GsMarkdown object */
	md = gs_markdown_new (GS_MARKDOWN_OUTPUT_PANGO);
	g_assert (md);

	markdown = "OEMs\n"
		   "====\n"
		   " - Bullett\n";
	markdown_expected =
		   "<big>OEMs</big>\n\n"
		   "• Bullett";
	/* markdown (type2 header) */
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	/* markdown (autocode) */
	markdown = "this is http://www.hughsie.com/with_spaces_in_url inline link\n";
	markdown_expected = "this is <tt>http://www.hughsie.com/with_spaces_in_url</tt> inline link";
	gs_markdown_set_autocode (md, TRUE);
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	/* markdown some invalid header */
	markdown = "*** This software is currently in alpha state ***\n";
	markdown_expected = "<b><i> This software is currently in alpha state </b></i>";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	/* markdown (complex1) */
	markdown = " - This is a *very*\n"
		   "   short paragraph\n"
		   "   that is not usual.\n"
		   " - Another";
	markdown_expected =
		   "• This is a <i>very</i> short paragraph that is not usual.\n"
		   "• Another";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	/* markdown (complex1) */
	markdown = "*  This is a *very*\n"
		   "   short paragraph\n"
		   "   that is not usual.\n"
		   "*  This is the second\n"
		   "   bullett point.\n"
		   "*  And the third.\n"
		   " \n"
		   "* * *\n"
		   " \n"
		   "Paragraph one\n"
		   "isn't __very__ long at all.\n"
		   "\n"
		   "Paragraph two\n"
		   "isn't much better.";
	markdown_expected =
		   "• This is a <i>very</i> short paragraph that is not usual.\n"
		   "• This is the second bullett point.\n"
		   "• And the third.\n"
		   "⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯\n"
		   "Paragraph one isn&apos;t <b>very</b> long at all.\n"
		   "Paragraph two isn&apos;t much better.";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	markdown = "This is a spec file description or\n"
		   "an **update** description in bohdi.\n"
		   "\n"
		   "* * *\n"
		   "# Big title #\n"
		   "\n"
		   "The *following* things 'were' fixed:\n"
		   "- Fix `dave`\n"
		   "* Fubar update because of \"security\"\n";
	markdown_expected =
		   "This is a spec file description or an <b>update</b> description in bohdi.\n"
		   "⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯⎯\n"
		   "\n<big>Big title</big>\n\n"
		   "The <i>following</i> things 'were' fixed:\n"
		   "• Fix <tt>dave</tt>\n"
		   "• Fubar update because of \"security\"";
	/* markdown (complex2) */
	text = gs_markdown_parse (md, markdown);
	if (g_strcmp0 (text, markdown_expected) == 0)
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	/* markdown (list with spaces) */
	markdown = "* list seporated with spaces -\n"
		   "  first item\n"
		   "\n"
		   "* second item\n"
		   "\n"
		   "* third item\n";
	markdown_expected =
		   "• list seporated with spaces - first item\n"
		   "• second item\n"
		   "• third item";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	gs_markdown_set_max_lines (md, 1);

	/* markdown (one line limit) */
	markdown = "* list seporated with spaces -\n"
		   "  first item\n"
		   "* second item\n";
	markdown_expected =
		   "• list seporated with spaces - first item";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	gs_markdown_set_max_lines (md, 1);

	/* markdown (escaping) */
	markdown = "* list & <spaces>";
	markdown_expected =
		   "• list &amp; &lt;spaces&gt;";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	/* markdown (URLs) */
	markdown = "* Upstream [release notes](https://www.lingmo.org/release-notes.html) there";
	markdown_expected =
		   "• Upstream "
		   "<a href=\"https://www.lingmo.org/release-notes.html\">release notes</a>"
		   " there";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	markdown = "Links: [link1](https://www.lingmo.org/1); [Link 2](https://www.lingmo.org/2)";
	markdown_expected =
		   "Links: "
		   "<a href=\"https://www.lingmo.org/1\">link1</a>; "
		   "<a href=\"https://www.lingmo.org/2\">Link 2</a>";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	markdown = "this is the http://www.hughsie.com/ coolest site";
	markdown_expected =
		   "this is the "
		   "<a href=\"http://www.hughsie.com/\">http://www.hughsie.com/</a>"
		   " coolest site";
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);

	/* markdown (free text) */
	gs_markdown_set_escape (md, FALSE);
	text = gs_markdown_parse (md, "This isn't a present");
	g_assert_cmpstr (text, ==, "This isn't a present");
	g_free (text);

	/* markdown (autotext underscore) */
	text = gs_markdown_parse (md, "This isn't CONFIG_UEVENT_HELPER_PATH present");
	g_assert_cmpstr (text, ==, "This isn't <tt>CONFIG_UEVENT_HELPER_PATH</tt> present");
	g_free (text);

	/* markdown (end of bullett) */
	markdown = "*Thu Mar 12 12:00:00 2009* Dan Walsh <dwalsh@redhat.com> - 2.0.79-1\n"
		   "- Update to upstream \n"
		   " * Netlink socket handoff patch from Adam Jackson.\n"
		   " * AVC caching of compute_create results by Eric Paris.\n"
		   "\n"
		   "*Tue Mar 10 12:00:00 2009* Dan Walsh <dwalsh@redhat.com> - 2.0.78-5\n"
		   "- Add patch from ajax to accellerate X SELinux \n"
		   "- Update eparis patch\n";
	markdown_expected =
		   "<i>Thu Mar 12 12:00:00 2009</i> Dan Walsh <tt>&lt;dwalsh@redhat.com&gt;</tt> - 2.0.79-1\n"
		   "• Update to upstream\n"
		   "• Netlink socket handoff patch from Adam Jackson.\n"
		   "• AVC caching of compute_create results by Eric Paris.\n"
		   "<i>Tue Mar 10 12:00:00 2009</i> Dan Walsh <tt>&lt;dwalsh@redhat.com&gt;</tt> - 2.0.78-5\n"
		   "• Add patch from ajax to accellerate X SELinux\n"
		   "• Update eparis patch";
	gs_markdown_set_escape (md, TRUE);
	gs_markdown_set_max_lines (md, 1024);
	text = gs_markdown_parse (md, markdown);
	g_assert_cmpstr (text, ==, markdown_expected);
	g_free (text);
}

static void
gs_plugins_packagekit_local_func (GsPluginLoader *plugin_loader)
{
	g_autoptr(GsApp) app = NULL;
	g_autoptr(GError) error = NULL;
	g_autofree gchar *fn = NULL;
	g_autoptr(GFile) file = NULL;
	g_autoptr(GsPluginJob) plugin_job = NULL;

	/* no packagekit, abort */
	if (!gs_plugin_loader_get_enabled (plugin_loader, "packagekit")) {
		g_test_skip ("not enabled");
		return;
	}

	/* load local file */
	fn = gs_test_get_filename (TESTDATADIR, "chiron-1.1-1.fc24.x86_64.rpm");
	g_assert (fn != NULL);
	file = g_file_new_for_path (fn);
	plugin_job = gs_plugin_job_newv (GS_PLUGIN_ACTION_FILE_TO_APP,
					 "file", file,
					 NULL);
	app = gs_plugin_loader_job_process_app (plugin_loader, plugin_job, NULL, &error);
	gs_test_flush_main_context ();
	if (g_error_matches (error, GS_PLUGIN_ERROR, GS_PLUGIN_ERROR_NOT_SUPPORTED)) {
		g_test_skip ("rpm files not supported");
		return;
	}
	g_assert_no_error (error);
	g_assert (app != NULL);
	g_assert_cmpstr (gs_app_get_source_default (app), ==, "chiron");
	g_assert_cmpstr (gs_app_get_url (app, AS_URL_KIND_HOMEPAGE), ==, "http://127.0.0.1/");
	g_assert_cmpstr (gs_app_get_name (app), ==, "chiron");
	g_assert_cmpstr (gs_app_get_version (app), ==, "1.1-1.fc24");
	g_assert_cmpstr (gs_app_get_summary (app), ==, "Single line synopsis");
	g_assert_cmpstr (gs_app_get_description (app), ==,
			 "This is the first paragraph in the example "
			 "package spec file.\n\nThis is the second paragraph.");
}

int
main (int argc, char **argv)
{
	gboolean ret;
	g_autoptr(GError) error = NULL;
	g_autoptr(GsPluginLoader) plugin_loader = NULL;
	const gchar * const allowlist[] = {
		"packagekit",
		NULL
	};

	/* The tests access the system proxy schemas, so pre-load those before
	 * %G_TEST_OPTION_ISOLATE_DIRS resets the XDG system dirs. */
	g_settings_schema_source_get_default ();

	gs_test_init (&argc, &argv);

	/* generic tests go here */
	g_test_add_func ("/lingmo-appstore/markdown", gs_markdown_func);

	/* we can only load this once per process */
	plugin_loader = gs_plugin_loader_new (NULL, NULL);
	gs_plugin_loader_add_location (plugin_loader, LOCALPLUGINDIR);
	ret = gs_plugin_loader_setup (plugin_loader,
				      allowlist,
				      NULL,
				      NULL,
				      &error);
	g_assert_no_error (error);
	g_assert (ret);

	/* plugin tests go here */
	if (!g_file_test ("/run/ostree-booted", G_FILE_TEST_EXISTS)) {
		g_test_add_data_func ("/lingmo-appstore/plugins/packagekit/local",
				      plugin_loader,
				      (GTestDataFunc) gs_plugins_packagekit_local_func);
	}

	return g_test_run ();
}
