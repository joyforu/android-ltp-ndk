/*
 *
 *   Copyright (c) Red Hat Inc., 2008
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Author: Masatake YAMATO <yamato@redhat.com> */

#ifndef __SETGROUPS_COMPAT_16_H__
#define __SETGROUPS_COMPAT_16_H__

#include <asm/posix_types.h>
#include "linux_syscall_numbers.h"

/* For avoiding circular dependency. */
extern void cleanup(void);

#ifdef TST_USE_COMPAT16_SYSCALL
typedef __kernel_old_gid_t GID_T;

int 
COMPAT_SIZE_CHECK(gid_t gid)
{
	/* See high2lowgid in linux/highuid.h
	   Return 0 if gid is too large to store
	   it to __kernel_old_gid_t. */
	return ((gid) & ~0xFFFF)? 0: 1;
}

long
SETGROUPS(int gidsetsize, GID_T *list)
{
	return syscall(__NR_setgroups, gidsetsize, list);
}

int
GETGROUPS(size_t size16, GID_T *list16)
{
	int r;
	int i;

	gid_t *list32;

	list32 = malloc(size16 * sizeof(gid_t));
	if (list32 == NULL)
	  tst_brkm(TBROK, NULL, "malloc failed to alloc %d errno "
		   " %d ", size16 * sizeof(gid_t), errno);

	r = getgroups(size16, list32);
	if (r < 0)
	  goto out;
  
	for (i = 0; i < size16; i++) {
		if (!COMPAT_SIZE_CHECK(list32[i]))
		  tst_brkm(TBROK,
			   cleanup,
			   "gid returned from getgroups is too large for testing setgroups32");
		list16[i] = (GID_T)list32[i];
	}

 out:
	free(list32);
	return r;
}

#else
typedef gid_t GID_T;

int
SETGROUPS(size_t size, const GID_T *list)
{
	return setgroups(size, list);
}

int
GETGROUPS(size_t size, GID_T *list)
{
	return getgroups(size, list);
}

int 
COMPAT_SIZE_CHECK(gid_t gid)
{
  return 1;
}
#endif	/* TST_USE_COMPAT16_SYSCALL */

#endif /* __SETGROUPS_COMPAT_16_H__ */