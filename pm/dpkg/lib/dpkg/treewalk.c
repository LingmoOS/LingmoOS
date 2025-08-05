/*
 * libdpkg - Debian packaging suite library routines
 * treewalk.c - directory tree walk support
 *
 * Copyright © 2013-2016 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <dpkg/dpkg.h>
#include <dpkg/i18n.h>
#include <dpkg/treewalk.h>

/* treenode functions. */

typedef int treewalk_stat_func(const char *pathname, struct stat *st);

struct treenode {
	struct treenode *up;	/* Parent dir. */
	struct treenode *next;	/* Next node in dir. */
	struct treenode **down;	/* Dir contents. */

	char *pathname;		/* Node pathname. */
	char *virtname;		/* Node virtname. */
	char *name;		/* Node name. */

	struct stat *stat;	/* Node metadata. */
	mode_t mode;

	size_t pathname_len;	/* Node pathname length. */

	size_t down_used;	/* Number of used nodes in dir. */
	size_t down_size;	/* Number of allocated nodes in dir. */
};

static inline struct treenode *
treenode_alloc(void)
{
	return m_calloc(1, sizeof(struct treenode));
}

static struct treenode *
treenode_root_new(const char *rootdir)
{
	struct treenode *node;

	node = treenode_alloc();
	node->up = NULL;
	node->pathname = m_strdup(rootdir);
	node->pathname_len = strlen(node->pathname);
	node->virtname = node->pathname + node->pathname_len;
	node->name = strrchr(node->pathname, '/');
	if (node->name == NULL)
		node->name = node->pathname;
	else
		node->name++;

	return node;
}

static struct treenode *
treenode_node_new(struct treenode *root, struct treenode *dir, const char *name)
{
	struct treenode *node;

	node = treenode_alloc();
	node->up = dir;

	node->pathname = str_fmt("%s/%s", node->up->pathname, name);
	node->pathname_len = strlen(node->pathname);
	node->virtname = node->pathname + root->pathname_len + 1;
	node->name = node->pathname + node->up->pathname_len + 1;

	return node;
}

static void
treenode_stat(struct treenode *node, treewalk_stat_func *stat_func)
{
	if (node->stat)
		return;

	node->stat = m_malloc(sizeof(*node->stat));

	if (stat_func(node->pathname, node->stat) < 0)
		ohshite(_("cannot stat pathname '%s'"), node->pathname);

	node->mode = node->stat->st_mode;
}

static mode_t
dirent_to_mode_type(struct dirent *e)
{
#ifdef _DIRENT_HAVE_D_TYPE
	switch (e->d_type) {
	case DT_REG:
		return S_IFREG;
	case DT_DIR:
		return S_IFDIR;
	case DT_LNK:
		return S_IFLNK;
	case DT_CHR:
		return S_IFCHR;
	case DT_BLK:
		return S_IFBLK;
	case DT_FIFO:
		return S_IFIFO;
	case DT_SOCK:
		return S_IFSOCK;
	case DT_UNKNOWN:
	default:
		return 0;
	}
#else
	return 0;
#endif
}

static void
treenode_stat_shallow(struct treenode *node, struct dirent *e,
                      treewalk_stat_func *stat_func)
{
	mode_t mode;

	mode = dirent_to_mode_type(e);
	if (mode == 0 || S_ISDIR(mode) || S_ISLNK(mode))
		treenode_stat(node, stat_func);
	else
		node->mode |= mode;
}

const char *
treenode_get_name(struct treenode *node)
{
	return node->name;
}

const char *
treenode_get_pathname(struct treenode *node)
{
	return node->pathname;
}

const char *
treenode_get_virtname(struct treenode *node)
{
	return node->virtname;
}

mode_t
treenode_get_mode(struct treenode *node)
{
	return node->mode;
}

struct stat *
treenode_get_stat(struct treenode *node)
{
	treenode_stat(node, lstat);
	return node->stat;
}

struct treenode *
treenode_get_parent(struct treenode *node)
{
	return node->up;
}

static inline bool
treenode_is_dir(struct treenode *node)
{
	return node && S_ISDIR(node->mode);
}

static void
treenode_resize_down(struct treenode *node)
{
	size_t new_size;

	if (node->down_size)
		node->down_size *= 2;
	else if (node->stat->st_nlink > 4)
		node->down_size = node->stat->st_nlink * 2;
	else
		node->down_size = 8;

	new_size = node->down_size * sizeof(*node);
	node->down = m_realloc(node->down, new_size);
}

static int
treenode_cmp(const void *a, const void *b)
{
	return strcmp((*(const struct treenode **)a)->name,
	              (*(const struct treenode **)b)->name);
}

static void
treenode_sort_down(struct treenode *dir)
{
	size_t i;

	qsort(dir->down, dir->down_used, sizeof(struct treenode *), treenode_cmp);

	/* Relink the nodes. */
	for (i = 0; i < dir->down_used - 1; i++)
		dir->down[i]->next = dir->down[i + 1];
	dir->down[i]->next = NULL;
}

static void
treenode_fill_down(struct treenode *root, struct treenode *dir,
                   enum treewalk_options options)
{
	DIR *d;
	struct dirent *e;
	treewalk_stat_func *stat_func;

	if (options & TREEWALK_FOLLOW_LINKS)
		stat_func = stat;
	else
		stat_func = lstat;

	d = opendir(dir->pathname);
	if (!d)
		ohshite(_("cannot open directory '%s'"), dir->pathname);

	while ((e = readdir(d)) != NULL) {
		struct treenode *node;

		if (strcmp(e->d_name, ".") == 0 ||
		    strcmp(e->d_name, "..") == 0)
			continue;

		if (dir->down_used >= dir->down_size)
			treenode_resize_down(dir);

		node = treenode_node_new(root, dir, e->d_name);
		if (options & TREEWALK_FORCE_STAT)
			treenode_stat(node, stat_func);
		else
			treenode_stat_shallow(node, e, stat_func);

		dir->down[dir->down_used] = node;
		dir->down_used++;
	}

	closedir(d);
}

static void
treenode_free_node(struct treenode *node)
{
	free(node->pathname);
	free(node->stat);
	free(node);
}

static void
treenode_free_down(struct treenode *node)
{
	size_t i;

	if (!node->down_size)
		return;

	for (i = 0; i < node->down_used; i++)
		treenode_free_node(node->down[i]);
	free(node->down);
}


/* treeroot functions. */

struct treeroot {
	struct treenode *rootnode;

	struct treenode *curdir;
	struct treenode *curnode;

	enum treewalk_options options;
	struct treewalk_funcs func;
};

static inline void
treeroot_set_curdir(struct treeroot *tree, struct treenode *node)
{
	tree->curdir = node;
}

static inline void
treeroot_set_curnode(struct treeroot *tree, struct treenode *node)
{
	tree->curnode = node;
}

static bool
treeroot_skip_node(struct treeroot *tree, struct treenode *node)
{
	if (tree->func.skip)
		return tree->func.skip(node);

	return false;
}

static void
treeroot_fill_node(struct treeroot *tree, struct treenode *dir)
{
	treenode_fill_down(tree->rootnode, dir, tree->options);
}

static void
treeroot_sort_node(struct treeroot *tree, struct treenode *dir)
{
	static struct treenode *down_empty[] = { NULL, NULL };

	if (dir->down_used == 0)
		dir->down = down_empty;
	else if (tree->func.sort)
		tree->func.sort(dir);
	else
		treenode_sort_down(dir);
}

static void
treeroot_visit_node(struct treeroot *tree, struct treenode *node)
{
	if (tree->func.visit == NULL)
		return;

	if (!treeroot_skip_node(tree, node))
		tree->func.visit(node);
}

/**
 * Open a new tree to be walked.
 *
 * @param rootdir The root directory to start walking the tree.
 * @param options The options specifying how to walk the tree.
 * @param func    The functions callbacks.
 */
struct treeroot *
treewalk_open(const char *rootdir, enum treewalk_options options,
              const struct treewalk_funcs *func)
{
	struct treeroot *tree;
	struct treenode *root;

	tree = m_malloc(sizeof(*tree));

	tree->options = options;
	if (func)
		tree->func = *func;
	else
		tree->func = TREEWALK_OBJECT;

	root = treenode_root_new(rootdir);
	treenode_stat(root, lstat);

	if (!treenode_is_dir(root))
		ohshit(_("treewalk root %s is not a directory"), rootdir);

	treeroot_set_curnode(tree, root);
	tree->rootnode = tree->curdir = root;

	return tree;
}

/**
 * Return the current node.
 *
 * This function is only needed if you want to start walking the tree from
 * the root node. With something like:
 *
 * @code
 * struct treeroot *tree;
 * struct treenode *node;
 *
 * tree = treewalk_open(...);
 * for (node = treewalk_node(tree); node; node = treewalk_next(tree))
 *         visitor(node);
 * treewalk_close(tree);
 * @endcode
 *
 * @param tree The tree structure.
 */
struct treenode *
treewalk_node(struct treeroot *tree)
{
	return tree->curnode;
}

/**
 * Return the next node.
 *
 * This function works basically as an iterator. And will return NULL when
 * the whole tree has been traversed. When starting it will skip the root
 * node, so you might want to use treewalk_node() to get that, otherwise
 * you could use it like this:
 *
 * @code
 * struct treeroot *tree;
 * struct treenode *node;
 *
 * tree = treewalk_open(...);
 * while ((node = treewalk_next(tree))
 *         visitor(node);
 * treewalk_close(tree);
 * @endcode
 *
 * @param tree The tree structure.
 */
struct treenode *
treewalk_next(struct treeroot *tree)
{
	struct treenode *node;

	/* Handle rootless trees, such as uninitialized or fully traversed. */
	if (tree->rootnode == NULL)
		return NULL;

	node = tree->curnode;

	/* Handle end of tree. */
	if (node == NULL)
		return NULL;

	/* Get next node, descending or sidewide. */
	if (treenode_is_dir(node) && !treeroot_skip_node(tree, node)) {
		struct treenode *dir;

		treeroot_fill_node(tree, node);
		treeroot_sort_node(tree, node);
		treeroot_set_curdir(tree, node);

		/* Check for directory loops. */
		for (dir = node->up; dir; dir = dir->up) {
			if (dir->stat->st_dev == node->stat->st_dev &&
			    dir->stat->st_ino == node->stat->st_ino)
				break;
		}

		/* Skip directory loops. */
		if (dir)
			node = node->next;
		else
			node = node->down[0];
	} else {
		node = node->next;
	}

	/* Back track node, ascending. */
	while (node == NULL) {
		struct treenode *olddir = tree->curdir;

		if (tree->curdir->next) {
			/* Next entry in the parent directory. */
			node = tree->curdir->next;
			treeroot_set_curdir(tree, olddir->up);
			treenode_free_down(olddir);
		} else if (tree->curdir->up) {
			/* Next entry in the grand-parent directory. */
			node = tree->curdir->up->next;
			treeroot_set_curdir(tree, olddir->up->up);
			treenode_free_down(olddir);
			treenode_free_down(olddir->up);
		} else {
			/* Otherwise, we're in the rootnode. */
			treenode_free_down(olddir);
			treenode_free_node(olddir);
			break;
		}

		if (tree->curdir == NULL) {
			treenode_free_node(tree->rootnode);
			tree->rootnode = NULL;
			break;
		}
	}

	treeroot_set_curnode(tree, node);

	return node;
}

/**
 * Closes the tree being walked.
 *
 * It will free any resources previously allocated.
 */
void
treewalk_close(struct treeroot *tree)
{
	free(tree);
}

/**
 * Tree walker.
 *
 * @param rootdir The root directory to start walking the tree.
 * @param options The options specifying how to walk the tree.
 * @param func    The function callbacks.
 */
int
treewalk(const char *rootdir, enum treewalk_options options,
         struct treewalk_funcs *func)
{
	struct treeroot *tree;
	struct treenode *node;

	tree = treewalk_open(rootdir, options, func);

	/* Breath first visit. */
	for (node = treewalk_node(tree); node; node = treewalk_next(tree))
		treeroot_visit_node(tree, node);

	treewalk_close(tree);

	return 0;
}
