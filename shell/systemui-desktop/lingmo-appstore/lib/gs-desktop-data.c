/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2015-2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>

#include "gs-desktop-data.h"

static const GsDesktopMap map_create[] = {
	{ "all",		NC_("Menu of Graphics & Photography", "All"),
					{ "Graphics",
					  "AudioVideo",
					  NULL } },
	{ "featured",		NC_("Menu of Graphics & Photography", "Featured"),
					{ "Graphics::Featured",
					  "AudioVideo::Featured",
					  NULL} },
	{ "3d",			NC_("Menu of Graphics & Photography", "3D Graphics"),
					{ "Graphics::3DGraphics",
					  NULL} },
	{ "photography",	NC_("Menu of Graphics & Photography", "Photography"),
					{ "Graphics::Photography",
					  NULL} },
	{ "scanning",		NC_("Menu of Graphics & Photography", "Scanning"),
					{ "Graphics::Scanning",
					  NULL} },
	{ "vector",		NC_("Menu of Graphics & Photography", "Vector Graphics"),
					{ "Graphics::VectorGraphics",
					  NULL} },
	{ "viewers",		NC_("Menu of Graphics & Photography", "Viewers"),
					{ "Graphics::Viewer",
					  NULL} },
	{ "creation-editing",	NC_("Menu of Audio & Video", "Audio Creation & Editing"),
					{ "AudioVideo::AudioVideoEditing",
					  "AudioVideo::Midi",
					  "AudioVideo::DiscBurning",
					  "AudioVideo::Sequencer",
					  NULL} },
	{ "music-players",	NC_("Menu of Audio & Video", "Music Players"),
					{ "AudioVideo::Music",
					  "AudioVideo::Player",
					  NULL} },
	{ NULL }
};

static const GsDesktopMap map_work[] = {
	{ "all",		NC_("Menu of Productivity", "All"),
					{ "Office",
					  "Utility",
					  "Network::WebBrowser",
					  NULL } },
	{ "featured",		NC_("Menu of Productivity", "Featured"),
					{ "Office::Featured",
					  "Utility::Featured",
					  NULL} },
	{ "calendar",		NC_("Menu of Productivity", "Calendar"),
					{ "Office::Calendar",
					  "Office::ProjectManagement",
					  NULL} },
	{ "database",		NC_("Menu of Productivity", "Database"),
					{ "Office::Database",
					  NULL} },
	{ "finance",		NC_("Menu of Productivity", "Finance"),
					{ "Office::Finance",
					  "Office::Spreadsheet",
					  NULL} },
	{ "word-processor",	NC_("Menu of Productivity", "Word Processor"),
					{ "Office::WordProcessor",
					  "Office::Dictionary",
					  NULL} },
	{ "text-editors",	NC_("Menu of Utilities", "Text Editors"),
					{ "Utility::TextEditor",
					  NULL} },
	{ "web-browsers",	NC_("Menu of Communication & News", "Web Browsers"),
					{ "Network::WebBrowser",
					  NULL} },
	{ NULL }
};

static const GsDesktopMap map_play[] = {
	{ "all",		NC_("Menu of Audio & Video", "All"),
					{ "Game",
					  NULL } },
	{ "featured",		NC_("Menu of Audio & Video", "Featured"),
					{ "Game::Featured",
					  NULL} },
	{ "action",		NC_("Menu of Games", "Action"),
					{ "Game::ActionGame",
					  NULL} },
	{ "adventure",		NC_("Menu of Games", "Adventure"),
					{ "Game::AdventureGame",
					  NULL} },
	{ "arcade",		NC_("Menu of Games", "Arcade"),
					{ "Game::ArcadeGame",
					  NULL} },
	{ "blocks",		NC_("Menu of Games", "Blocks"),
					{ "Game::BlocksGame",
					  NULL} },
	{ "board",		NC_("Menu of Games", "Board"),
					{ "Game::BoardGame",
					  NULL} },
	{ "card",		NC_("Menu of Games", "Card"),
					{ "Game::CardGame",
					  NULL} },
	{ "emulator",		NC_("Menu of Games", "Emulators"),
					{ "Game::Emulator",
					  NULL} },
	{ "kids",		NC_("Menu of Games", "Kids"),
					{ "Game::KidsGame",
					  NULL} },
	{ "logic",		NC_("Menu of Games", "Logic"),
					{ "Game::LogicGame",
					  "Game::Simulation",
					  NULL} },
	{ "role-playing",	NC_("Menu of Games", "Role Playing"),
					{ "Game::RolePlaying",
					  NULL} },
	{ "sports",		NC_("Menu of Games", "Sports"),
					{ "Game::SportsGame",
					  NULL} },
	{ "strategy",		NC_("Menu of Games", "Strategy"),
					{ "Game::StrategyGame",
					  NULL} },
	{ NULL }
};

static const GsDesktopMap map_socialize[] = {
	{ "all",		NC_("Menu of Communication & News", "All"),
					{ "Network",
					  NULL } },
	{ "featured",		NC_("Menu of Communication & News", "Featured"),
					{ "Network::Featured",
					  NULL} },
	{ "chat",		NC_("Menu of Communication & News", "Chat"),
					{ "Network::Chat",
					  "Network::IRCClient",
					  "Network::Telephony",
					  "Network::VideoConference",
					  "Network::Email",
					  NULL} },
	{ NULL }
};

static const GsDesktopMap map_learn[] = {
	{ "all",		NC_("Menu of Education & Science", "All"),
					{ "Education",
					  "Science",
					  "Reference",
					  "Network::Feed",
					  "Network::News",
					  NULL } },
	{ "featured",		NC_("Menu of Education & Science", "Featured"),
					{ "Education::Featured",
					  "Science::Featured",
					  "Reference::Featured",
					  NULL} },
	{ "artificial-intelligence", NC_("Menu of Education & Science", "Artificial Intelligence"),
					{ "Science::ArtificialIntelligence",
					  NULL} },
	{ "astronomy",		NC_("Menu of Education & Science", "Astronomy"),
					{ "Education::Astronomy",
					  "Science::Astronomy",
					  NULL} },
	{ "chemistry",		NC_("Menu of Education & Science", "Chemistry"),
					{ "Education::Chemistry",
					  "Science::Chemistry",
					  NULL} },
	{ "languages",		NC_("Menu of Education & Science", "Languages"),
					{ "Education::Languages",
					  "Education::Literature",
					  NULL} },
	{ "math",		NC_("Menu of Education & Science", "Math"),
					{ "Education::Math",
					  "Education::NumericalAnalysis",
					  "Science::Math",
					  "Science::Physics",
					  "Science::NumericalAnalysis",
					  NULL} },
	{ "news",		NC_("Menu of Communication & News", "News"),
					{ "Network::Feed",
					  "Network::News",
					  NULL} },
	{ "robotics",		NC_("Menu of Education & Science", "Robotics"),
					{ "Science::Robotics",
					  NULL} },
	{ "art",		NC_("Menu of Art", "Art"),
					{ "Reference::Art",
					  NULL} },
	{ "biography",		NC_("Menu of Reference", "Biography"),
					{ "Reference::Biography",
					  NULL} },
	{ "comics",		NC_("Menu of Reference", "Comics"),
					{ "Reference::Comics",
					  NULL} },
	{ "fiction",		NC_("Menu of Reference", "Fiction"),
					{ "Reference::Fiction",
					  NULL} },
	{ "health",		NC_("Menu of Reference", "Health"),
					{ "Reference::Health",
					  NULL} },
	{ "history",		NC_("Menu of Reference", "History"),
					{ "Reference::History",
					  NULL} },
	{ "lifestyle",		NC_("Menu of Reference", "Lifestyle"),
					{ "Reference::Lifestyle",
					  NULL} },
	{ "politics",		NC_("Menu of Reference", "Politics"),
					{ "Reference::Politics",
					  NULL} },
	{ "sports",		NC_("Menu of Reference", "Sports"),
					{ "Reference::Sports",
					  NULL} },
	{ NULL }
};

static const GsDesktopMap map_develop[] = {
	{ "all",		NC_("Menu of Developer Tools", "All"),
					{ "Development",
					  NULL } },
	{ "featured",		NC_("Menu of Developer Tools", "Featured"),
					{ "Development::Featured",
					  NULL} },
	{ "debuggers",		NC_("Menu of Developer Tools", "Debuggers"),
					{ "Development::Debugger",
					  NULL} },
	{ "ide",		NC_("Menu of Developer Tools", "IDEs"),
					{ "Development::IDE",
					  "Development::GUIDesigner",
					  NULL} },
	{ NULL }
};

static const GsDesktopMap map_addon_codecs[] = {
	{ "all",		NC_("Menu of Add-ons", "Codecs"),
					{ "Addon::Codec",
					  NULL } },
	{ NULL }
};

static const GsDesktopMap map_addon_drivers[] = {
	{ "all",		NC_("Menu of Add-ons", "Hardware Drivers"),
					{ "Addon::Driver",
					  NULL } },
	{ NULL }
};

static const GsDesktopMap map_addon_fonts[] = {
	{ "all",		NC_("Menu of Add-ons", "Fonts"),
					{ "Addon::Font",
					  NULL } },
	{ NULL }
};

static const GsDesktopMap map_addon_input_sources[] = {
	{ "all",		NC_("Menu of Add-ons", "Input Sources"),
					{ "Addon::InputSource",
					  NULL } },
	{ NULL }
};

static const GsDesktopMap map_addon_language_packs[] = {
	{ "all",		NC_("Menu of Add-ons", "Language Packs"),
					{ "Addon::LanguagePack",
					  NULL } },
	{ NULL }
};

static const GsDesktopMap map_addon_localization[] = {
	{ "all",		NC_("Menu of Add-ons", "Localization"),
					{ "Addon::Localization",
					  NULL } },
	{ NULL }
};

/* main categories */
/* Please keep category name and subcategory context synchronized!!! */
static const GsDesktopData msdata[] = {
	/* Translators: this is a menu category */
	{ "create", map_create, N_("Create"), "org.lingmo.Appstore.Create", 100 },
	/* Translators: this is a menu category */
	{ "work", map_work, N_("Work"), "org.lingmo.Appstore.Work", 90 },
	/* Translators: this is a menu category */
	{ "play", map_play, N_("Play"), "org.lingmo.Appstore.Play", 80 },
	/* Translators: this is a menu category */
	{ "socialize", map_socialize, N_("Socialize"), "org.lingmo.Appstore.Socialize", 70 },
	/* Translators: this is a menu category */
	{ "learn", map_learn, N_("Learn"), "org.lingmo.Appstore.Learn", 60 },
	/* Translators: this is a menu category */
	{ "develop", map_develop, N_("Develop"), "org.lingmo.Appstore.Develop", 50 },

	/* Translators: this is a menu category */
	{ "codecs", map_addon_codecs, N_("Codecs"), NULL, 10 },
	/* Translators: this is a menu category */
	{ "drivers", map_addon_drivers, N_("Hardware Drivers"), NULL, 10 },
	/* Translators: this is a menu category */
	{ "fonts", map_addon_fonts, N_("Fonts"), NULL, 10 },
	/* Translators: this is a menu category */
	{ "input-sources", map_addon_input_sources, N_("Input Sources"), NULL, 10 },
	/* Translators: this is a menu category */
	{ "language-packs", map_addon_language_packs, N_("Language Packs"), NULL, 10 },
	/* Translators: this is a menu category */
	{ "localization", map_addon_localization, N_("Localization"), NULL, 10 },

	{ NULL }
};

/* the -1 is for the NULL terminator */
G_STATIC_ASSERT (G_N_ELEMENTS (msdata) - 1 == GS_DESKTOP_DATA_N_ENTRIES);

const GsDesktopData *
gs_desktop_get_data (void)
{
	return msdata;
}
