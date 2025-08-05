/*
 * libdpkg - Debian packaging suite library routines
 * pkg-queue.c - primitives for pkg queue handling
 *
 * Copyright © 2010 Guillem Jover <guillem@debian.org>
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

#include <stdlib.h>

#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-queue.h>

/**
 * Initialize a package queue.
 *
 * @param queue The queue to initialize.
 */
void
pkg_queue_init(struct pkg_queue *queue)
{
	queue->head = NULL;
	queue->tail = NULL;
	queue->length = 0;
}

/**
 * Destroy a package queue.
 *
 * It frees the contained package list and resets the queue members.
 *
 * @param queue The queue to destroy.
 */
void
pkg_queue_destroy(struct pkg_queue *queue)
{
	pkg_list_free(queue->head);
	pkg_queue_init(queue);
}

/**
 * Check if a package queue is empty.
 *
 * @param queue The queue to check.
 *
 * @return A boolean value.
 */
int
pkg_queue_is_empty(struct pkg_queue *queue)
{
	return (queue->head == NULL);
}

/**
 * Push a new node containing pkginfo to the tail of the queue.
 *
 * @param queue The queue to insert to.
 * @param pkg The package to use fo the new node.
 *
 * @return The newly inserted pkg_list node.
 */
struct pkg_list *
pkg_queue_push(struct pkg_queue *queue, struct pkginfo *pkg)
{
	struct pkg_list *node;

	node = pkg_list_new(pkg, NULL);

	if (queue->tail == NULL)
		queue->head = node;
	else
		queue->tail->next = node;

	queue->tail = node;

	queue->length++;

	return node;
}

/**
 * Pop a node containing pkginfo from the head of the queue.
 *
 * This removes and frees the node from the queue, effectively reducing its
 * size.
 *
 * @param queue The queue to remove from.
 *
 * @return The pkginfo from the removed node, or NULL if the queue was empty.
 */
struct pkginfo *
pkg_queue_pop(struct pkg_queue *queue)
{
	struct pkg_list *node;
	struct pkginfo *pkg;

	if (pkg_queue_is_empty(queue))
		return NULL;

	node = queue->head;
	pkg = node->pkg;

	queue->head = node->next;
	if (queue->head == NULL)
		queue->tail = NULL;

	free(node);
	queue->length--;

	return pkg;
}
