#
# -*- makefile -*-
#
#     Copyright (C) 2004-2005 Davide Viti <zinosat@tiscali.it>
# 
#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.
# 
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public
#     License along with this program; if not, write to the Free
#     Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA 02111-1307 USA
#

bash_scripts = build_index.sh  check_all.sh  check_dit.sh  diff_stats.sh  l10n-spellcheck.sh
awk_scripts =  msgid_extract.awk  msgstr_extract.awk
perl_scripts = check_var.pl


.PHONY: all install clean

all:
	@echo "Making all (...nothing really!)"

install:
	mkdir -p ${DESTDIR}/usr/bin
	cp ${bash_scripts} ${awk_scripts} ${perl_scripts} ${DESTDIR}/usr/bin/
	cd ${DESTDIR}/usr/bin && chmod 644 ${bash_scripts} ${awk_scripts} ${perl_scripts}

uninstall:
	-cd ${DESTDIR}/usr/bin && rm ${bash_scripts} ${awk_scripts} ${perl_scripts}

clean:
	@echo "Making clean (...nothing really!)"
