/*  Copyright (C) 2007 O.S. Systems */
/*  Copyright (C) 2007 Otavio Salvador <otavio@debian.org> */

/*  This program is free software; you can redistribute it and/or modify */
/*  it under the terms of the GNU General Public License as published by */
/*  the Free Software Foundation; either version 2 of the License, or */
/*  (at your option) any later version. */

/*  This program is distributed in the hope that it will be useful, */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*  GNU General Public License for more details. */

/*  You should have received a copy of the GNU General Public License along */
/*  with this program; if not, write to the Free Software Foundation, Inc., */
/*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include <config.h>

#include <stdio.h>
#include <sys/statfs.h>

int main(int argc, char** argv)
{
  struct statfs stat;

  if (argc != 2) {
    printf("Wrong number of arguments, use: print-inodes mount-point\n");
    return 1;
  }

  if (statfs(argv[1], &stat) != 0) {
    perror("statfs has failed! ");
    return 1;
  }

  printf("%ld\n", stat.f_files);

  return 0;
}
