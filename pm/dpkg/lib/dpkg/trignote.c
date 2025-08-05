/*
 * libdpkg - Debian packaging suite library routines
 * trignote.c - trigger note handling
 *
 * Copyright © 2007 Canonical Ltd
 * Written by Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
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

#include <string.h>

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/pkg-list.h>
#include <dpkg/dlist.h>
#include <dpkg/triglib.h>

/*
 * Note: This is also called from fields.c where *pend is a temporary!
 *
 * trig is not copied!
 */
bool
trig_note_pend_core(struct pkginfo *pend, const char *trig)
{
	struct trigpend *tp;

	for (tp = pend->trigpend_head; tp; tp = tp->next)
		if (strcmp(tp->name, trig) == 0)
			return false;

	tp = nfmalloc(sizeof(*tp));
	tp->name = trig;
	tp->next = pend->trigpend_head;
	pend->trigpend_head = tp;

	return true;
}

/*
 * trig is not copied!
 *
 * @retval true  For done.
 * @retval false For already noted.
 */
bool
trig_note_pend(struct pkginfo *pend, const char *trig)
{
	if (!trig_note_pend_core(pend, trig))
		return false;

	if (pend->trigaw.head)
		pkg_set_status(pend, PKG_STAT_TRIGGERSAWAITED);
	else
		pkg_set_status(pend, PKG_STAT_TRIGGERSPENDING);

	return true;
}

/*
 * Note: This is called also from fields.c where *aw is a temporary
 * but pend is from pkg_hash_find()!
 *
 * @retval true  For done.
 * @retval false For already noted.
 */
bool
trig_note_aw(struct pkginfo *pend, struct pkginfo *aw)
{
	struct trigaw *ta;

	/* We search through aw's list because that's probably shorter. */
	for (ta = aw->trigaw.head; ta; ta = ta->sameaw.next)
		if (ta->pend == pend)
			return false;

	ta = nfmalloc(sizeof(*ta));
	ta->aw = aw;
	ta->pend = pend;
	ta->samepend_next = pend->othertrigaw_head;
	pend->othertrigaw_head = ta;
	LIST_LINK_TAIL_PART(aw->trigaw, ta, sameaw);

	return true;
}

static struct pkg_list *trig_awaited_pend_head;

void
trig_awaited_pend_enqueue(struct pkginfo *pend)
{
	struct pkg_list *tp;

	for (tp = trig_awaited_pend_head; tp; tp = tp->next)
		if (tp->pkg == pend)
			return;

	pkg_list_prepend(&trig_awaited_pend_head, pend);
}

void
trig_awaited_pend_foreach(trig_awaited_pend_foreach_func *func)
{
	struct pkg_list *tp;

	for (tp = trig_awaited_pend_head; tp; tp = tp->next)
		if (!tp->pkg->trigpend_head)
			func(tp->pkg);
}

void
trig_awaited_pend_free(void)
{
	pkg_list_free(trig_awaited_pend_head);
	trig_awaited_pend_head = NULL;
}
