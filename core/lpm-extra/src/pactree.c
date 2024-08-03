/*
 *  pactree.c - a simple dependency tree viewer
 *
 *  Copyright (c) 2010-2016 Lpm Development Team <lpm-dev@archlinux.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <alpm.h>
#include <alpm_list.h>
#include <langinfo.h>

#define LINE_MAX     512

#include <limits.h>

typedef struct tdepth {
	struct tdepth *prev;
	struct tdepth *next;
	int level;
} tdepth;

/* output */
struct graph_style {
	const char *provides;
	const char *optional;
	const char *tip;
	const char *last;
	const char *limb;
	int indent;
};

#define UTF_V   "\342\224\202"  /* U+2502, Vertical line drawing char */
#define UTF_VR  "\342\224\234"  /* U+251C, Vertical and right */
#define UTF_H   "\342\224\200"  /* U+2500, Horizontal */
#define UTF_UR  "\342\224\224"  /* U+2514, Up and right */

static struct graph_style graph_utf8 = {
	" provides",
	" (optional)",
	UTF_VR UTF_H,
	UTF_UR UTF_H,
	UTF_V " ",
	2
};

static struct graph_style graph_default = {
	" provides",
	" (optional)",
	"|-",
	"`-",
	"|",
	2
};

static struct graph_style graph_linear = {
	"",
	"",
	"",
	"",
	"",
	0
};

/* color choices */
struct color_choices {
	const char *branch1;
	const char *branch2;
	const char *leaf1;
	const char *leaf2;
	const char *error;
	const char *warning;
	const char *off;
};

static struct color_choices use_color = {
	"\033[0;33m", /* yellow */
	"\033[0;37m", /* white */
	"\033[1;32m", /* bold green */
	"\033[0;32m", /* green */
	"\033[1;31m", /* bold red */
	"\033[1;33m", /* bold yellow */
	"\033[0m"
};

static struct color_choices no_color = {
	"",
	"",
	"",
	"",
	"",
	"",
	""
};

/* long operations */
enum {
	OP_CONFIG = 1000,
	OP_DEBUG,
	OP_GPGDIR
};

/* globals */
static alpm_handle_t *handle = NULL;
static alpm_list_t *walked = NULL;
static alpm_list_t *provisions = NULL;

/* options */
static struct color_choices *color = &no_color;
static struct graph_style *style = &graph_utf8;
static int graphviz = 0;
static int max_depth = -1;
static int reverse = 0;
static int unique = 0;
static int searchsyncs = 0;
static int debug = 0;
static int opt_level = 0;
static const char *dbpath = DBPATH;
static const char *configfile = CONFFILE;
static const char *gpgdir = GPGDIR;

void cb_log(void *ctx, alpm_loglevel_t level, const char *fmt, va_list args)
{
	(void)ctx;

	switch(level) {
		case ALPM_LOG_ERROR:
			fprintf(stderr, "%s%s%s", color->error, "error: ",
					color->off);
			break;
		case ALPM_LOG_WARNING:
			fprintf(stderr, "%s%s%s", color->warning, "warning: ",
					color->off);
			break;
		case ALPM_LOG_DEBUG:
			fprintf(stderr, "debug: ");
			break;
		case ALPM_LOG_FUNCTION:
			fprintf(stderr, "function: ");
			break;
		}

	vfprintf(stderr, fmt, args);
}

/* Trim whitespace and newlines from a string
 */
static size_t strtrim(char *str)
{
	char *end, *pch = str;

	if(str == NULL || *str == '\0') {
		/* string is empty, so we're done. */
		return 0;
	}

	while(isspace((unsigned char)*pch)) {
		pch++;
	}
	if(pch != str) {
		size_t len = strlen(pch);
		/* check if there wasn't anything but whitespace in the string. */
		if(len == 0) {
			*str = '\0';
			return 0;
		}
		memmove(str, pch, len + 1);
		pch = str;
	}

	end = (str + strlen(str) - 1);
	while(isspace((unsigned char)*end)) {
		end--;
	}
	*++end = '\0';

	return end - pch;
}

static int register_syncs(void)
{
	FILE *fp;
	char *section = NULL;
	char line[LINE_MAX];
	const alpm_siglevel_t level = ALPM_SIG_DATABASE | ALPM_SIG_DATABASE_OPTIONAL;

	fp = fopen(configfile, "r");
	if(!fp) {
		fprintf(stderr, "error: config file %s could not be read\n", configfile);
		return 1;
	}

	while(fgets(line, LINE_MAX, fp)) {
		size_t linelen;
		char *ptr;

		/* ignore whole line and end of line comments */
		if((ptr = strchr(line, '#'))) {
			*ptr = '\0';
		}

		linelen = strtrim(line);

		if(linelen == 0) {
			continue;
		}

		if(line[0] == '[' && line[linelen - 1] == ']') {
			free(section);
			section = strndup(&line[1], linelen - 2);

			if(section && strcmp(section, "options") != 0) {
				alpm_db_t *db = alpm_register_syncdb(handle, section, level);
				alpm_db_set_usage(db, ALPM_DB_USAGE_ALL);
			}
		}
	}

	free(section);
	fclose(fp);

	return 0;
}

static void cleanup(int ret)
{
	alpm_list_free(walked);
	FREELIST(provisions);
	alpm_release(handle);

	exit(ret);
}

static void usage(void)
{
	fprintf(stdout, "pactree v" PACKAGE_VERSION "\n\n"
			"Package dependency tree viewer.\n\n"
			"Usage: pactree [options] <package>\n\n"
			"Options:\n"
			"  -a, --ascii             use ASCII characters for tree formatting\n"
			"  -c, --color             colorize output\n"
			"      --config <path>     set an alternate configuration file\n"
			"  -b, --dbpath <path>     set an alternate database location\n"
			"      --debug             display debug messages\n"
			"  -d, --depth <#>         limit the depth of recursion\n"
			"      --gpgdir <path>     set an alternate home directory for GnuPG\n"
			"  -g, --graph             generate output for graphviz's dot\n"
			"  -l, --linear            enable linear output\n"
			"  -o, --optional[=depth]  controls at which depth to stop printing optional deps\n"
			"                          (-1 for no limit)\n"
			"  -r, --reverse           list packages that depend on the named package\n"
			"  -s, --sync              search sync databases instead of local\n"
			"  -u, --unique            show dependencies with no duplicates (implies -l)\n"
			"  -h, --help              display this help message and exit\n"
			"  -V, --version           display version information and exit\n");
}

static void version(void)
{
	fprintf(stdout, "pactree v" PACKAGE_VERSION "\n");
}

static int parse_options(int argc, char *argv[])
{
	int opt, option_index = 0;
	char *endptr = NULL;

	static const struct option opts[] = {
		{"ascii",    no_argument,          0, 'a'},
		{"dbpath",   required_argument,    0, 'b'},
		{"color",    no_argument,          0, 'c'},
		{"depth",    required_argument,    0, 'd'},
		{"graph",    no_argument,          0, 'g'},
		{"help",     no_argument,          0, 'h'},
		{"linear",   no_argument,          0, 'l'},
		{"optional", optional_argument,    0, 'o'},
		{"reverse",  no_argument,          0, 'r'},
		{"sync",     no_argument,          0, 's'},
		{"unique",   no_argument,          0, 'u'},
		{"version",  no_argument,          0, 'V'},

		{"config",  required_argument,    0, OP_CONFIG},
		{"debug",   no_argument,          0, OP_DEBUG},
		{"gpgdir",  required_argument,    0, OP_GPGDIR},

		{0, 0, 0, 0}
	};

	setlocale(LC_ALL, "");
	if(strcmp(nl_langinfo(CODESET), "UTF-8") == 0) {
		style = &graph_utf8;
	}

	while((opt = getopt_long(argc, argv, "ab:cd:ghlrsuo::V", opts, &option_index))) {
		if(opt < 0) {
			break;
		}

		switch(opt) {
			case OP_CONFIG:
				configfile = optarg;
				break;
			case OP_DEBUG:
				debug = 1;
				break;
			case OP_GPGDIR:
				gpgdir = optarg;
				break;
			case 'a':
				style = &graph_default;
				break;
			case 'b':
				dbpath = optarg;
				break;
			case 'c':
				color = &use_color;
				break;
			case 'd':
				/* validate depth */
				max_depth = (int)strtol(optarg, &endptr, 10);
				if(*endptr != '\0') {
					fprintf(stderr, "error: invalid depth -- %s\n", optarg);
					return 1;
				}
				break;
			case 'g':
				graphviz = 1;
				break;
			case 'l':
				style = &graph_linear;
				break;
			case 'o':
				if(optarg) {
					opt_level = (int)strtol(optarg, &endptr, 10);
					if(*endptr != '\0') {
						fprintf(stderr, "error: invalid optional depth -- %s\n", optarg);
						return 1;
					}
				} else {
					opt_level = 1;
				}
				break;
			case 'r':
				reverse = 1;
				break;
			case 's':
				searchsyncs = 1;
				break;
			case 'u':
				unique = 1;
				style = &graph_linear;
				break;
			case 'h':
				usage();
				cleanup(0);
			case 'V':
				version();
				cleanup(0);
			default:
				usage();
				return 1;
		}
	}

	if(!argv[optind] || argv[optind + 1]) {
		usage();
		return 1;
	}

	return 0;
}

static int should_show_satisfier(const char *pkg, const char *depstring)
{
	int result;
	alpm_depend_t *dep = alpm_dep_from_string(depstring);
	if(!dep) return 1;
	result = strcmp(pkg, dep->name) != 0;
	free(dep);
	return result;
}

/* pkg provides provision */
static void print_text(const char *pkg, const char *provision,
		tdepth *depth, int last, int opt_dep)
{
	const char *tip = "";
	const char *opt_str = opt_dep ? style->optional : "";
	int level = 1;
	if(!pkg && !provision) {
		/* not much we can do */
		return;
	}

	if(depth->level > 0) {
		tip = last ? style->last : style->tip;

		/* print limbs */
		while(depth->prev) {
			depth = depth->prev;
		}
		printf("%s", color->branch1);
		while(depth->next) {
			printf("%*s%-*s", style->indent * (depth->level - level), "",
					style->indent, style->limb);
			level = depth->level + 1;
			depth = depth->next;
		}
		printf("%*s", style->indent * (depth->level - level), "");
	}

	/* print tip */
	/* If style->provides is empty (e.g. when using linear style), we do not
	 * want to print the provided package. This makes output easier to parse and
	 * to reuse. */
	if(!pkg && provision) {
		printf("%s%s%s%s [unresolvable]%s%s\n", tip, color->leaf1,
				provision, color->branch1, opt_str, color->off);
	} else if(provision && *(style->provides) != '\0' && should_show_satisfier(pkg, provision)) {
		printf("%s%s%s%s%s %s%s%s%s\n", tip, color->leaf1, pkg,
				color->leaf2, style->provides, color->leaf1, provision, opt_str,
				color->off);
	} else {
		printf("%s%s%s%s%s\n", tip, color->leaf1, provision ? provision : pkg, opt_str, color->off);
	}
}

static void print_graph(const char *parentname, const char *pkgname, const char *depname, int opt_dep)
{
	const char *style = opt_dep ? ", style=dotted" : "";
	if(depname) {
		printf("\"%s\" -> \"%s\" [color=chocolate4%s];\n", parentname, depname, style);
		if(pkgname && strcmp(depname, pkgname) != 0 && !alpm_list_find_str(provisions, depname)) {
			printf("\"%s\" -> \"%s\" [arrowhead=none, color=grey%s];\n", depname, pkgname, style);
			provisions = alpm_list_add(provisions, strdup(depname));
		}
	} else if(pkgname) {
		printf("\"%s\" -> \"%s\" [color=chocolate4%s];\n", parentname, pkgname, style);
	}
}

/* parent depends on dep which is satisfied by pkg */
static void print(const char *parentname, const char *pkgname,
		const char *depname, tdepth *depth, int last, int opt_dep)
{
	if(graphviz) {
		print_graph(parentname, pkgname, depname, opt_dep);
	} else {
		print_text(pkgname, depname, depth, last, opt_dep);
	}
}

static void print_start(const char *pkgname, const char *provname)
{
	if(graphviz) {
		printf("digraph G { START [color=red, style=filled];\n"
				"node [style=filled, color=green];\n"
				" \"START\" -> \"%s\";\n", pkgname);
	} else {
		tdepth d = {
			NULL,
			NULL,
			0
		};
		print_text(pkgname, provname, &d, 0, 0);
	}
}

static void print_end(void)
{
	if(graphviz) {
		/* close graph output */
		printf("}\n");
	}
}

static alpm_list_t *get_pkg_deps(alpm_pkg_t *pkg)
{
	alpm_list_t *i, *dep_strings = NULL;
	for(i = alpm_pkg_get_depends(pkg); i; i = alpm_list_next(i)) {
		alpm_depend_t *dep = i->data;
		char *ds = alpm_dep_compute_string(dep);
		dep_strings = alpm_list_add(dep_strings, ds);
	}
	return dep_strings;
}

static alpm_list_t *get_pkg_optdeps(alpm_pkg_t *pkg)
{
	alpm_list_t *i, *dep_strings = NULL;
	for(i = alpm_pkg_get_optdepends(pkg); i; i = alpm_list_next(i)) {
		alpm_depend_t *dep = i->data;
		char *ds = alpm_dep_compute_string(dep);
		dep_strings = alpm_list_add(dep_strings, ds);
	}
	return dep_strings;
}

/* forward declaration */
static void walk_deps(alpm_list_t *dblist, alpm_pkg_t *pkg, tdepth *depth, int rev, int optional);

/**
 * print the dependency list given, passing the optional parameter when required
 */
static void print_dep_list(alpm_list_t *deps, alpm_list_t *dblist, alpm_pkg_t *pkg, tdepth *depth, int rev, int optional, int opt_dep, int final)
{
	alpm_list_t *i;
	int new_optional;

	if(optional > 0) {
		/* decrease the depth by 1 */
		new_optional = optional - 1;
	} else {
		/* preserve 0 (ran out of depth) and negative numbers (infinite depth) */
		new_optional = optional;
	}

	for(i = deps; i; i = alpm_list_next(i)) {
		const char *pkgname = i->data;
		int last = final && (alpm_list_next(i) ? 0 : 1);

		alpm_pkg_t *dep_pkg = alpm_find_dbs_satisfier(handle, dblist, pkgname);

		if(alpm_list_find_str(walked, dep_pkg ? alpm_pkg_get_name(dep_pkg) : pkgname)) {
			/* if we've already seen this package, don't print in "unique" output
			 * and don't recurse */
			if(!unique) {
				print(alpm_pkg_get_name(pkg), alpm_pkg_get_name(dep_pkg), pkgname, depth, last, opt_dep);
			}
		} else {
			tdepth d;
			print(alpm_pkg_get_name(pkg), alpm_pkg_get_name(dep_pkg), pkgname, depth, last, opt_dep);
			if(dep_pkg) {
				d = (tdepth){
					depth,
					NULL,
					depth->level + 1
				};
				depth->next = &d;
				/* last dep, cut off the limb here */
				if(last) {
					if(depth->prev) {
						depth->prev->next = &d;
						d.prev = depth->prev;
						depth = &d;
					} else {
						d.prev = NULL;
					}
				}
				walk_deps(dblist, dep_pkg, &d, rev, new_optional);
				depth->next = NULL;
			}
		}
	}
}

/**
 * walk dependencies, showing dependencies of the target
 */
static void walk_deps(alpm_list_t *dblist, alpm_pkg_t *pkg, tdepth *depth, int rev, int optional)
{
	alpm_list_t *deps, *optdeps;

	if(!pkg || ((max_depth >= 0) && (depth->level > max_depth))) {
		return;
	}

	walked = alpm_list_add(walked, (void *)alpm_pkg_get_name(pkg));

	if(rev) {
		deps = alpm_pkg_compute_requiredby(pkg);
	} else {
		deps = get_pkg_deps(pkg);
	}

	optdeps = NULL;
	if(optional){
		if(rev) {
			optdeps = alpm_pkg_compute_optionalfor(pkg);
		} else {
			optdeps = get_pkg_optdeps(pkg);
		}
	}

	print_dep_list(deps, dblist, pkg, depth, rev, optional, 0, !optdeps);
	FREELIST(deps);

	print_dep_list(optdeps, dblist, pkg, depth, rev, optional, 1, 1);
	FREELIST(optdeps);
}

int main(int argc, char *argv[])
{
	int freelist = 0, ret;
	alpm_errno_t err;
	const char *target_name;
	alpm_pkg_t *pkg;
	alpm_list_t *dblist = NULL;

	if((ret = parse_options(argc, argv)) != 0) {
		cleanup(ret);
	}

	handle = alpm_initialize(ROOTDIR, dbpath, &err);
	if(!handle) {
		fprintf(stderr, "error: cannot initialize alpm: %s\n",
				alpm_strerror(err));
		cleanup(1);
	}

	if(debug) {
		alpm_option_set_logcb(handle, cb_log, NULL);
	}

	/* no need to fail on error here */
	alpm_option_set_gpgdir(handle, gpgdir);

	if(searchsyncs) {
		if(register_syncs() != 0) {
			cleanup(1);
		}
		dblist = alpm_get_syncdbs(handle);
	} else {
		dblist = alpm_list_add(dblist, alpm_get_localdb(handle));
		freelist = 1;
	}

	/* we only care about the first non option arg for walking */
	target_name = argv[optind];

	pkg = alpm_find_dbs_satisfier(handle, dblist, target_name);
	if(!pkg) {
		fprintf(stderr, "error: package '%s' not found\n", target_name);
		cleanup(1);
	}

	print_start(alpm_pkg_get_name(pkg), target_name);

	tdepth d = {
		NULL,
		NULL,
		1
	};
	walk_deps(dblist, pkg, &d, reverse, opt_level);

	print_end();

	if(freelist) {
		alpm_list_free(dblist);
	}

	cleanup(0);
}
