/* SPDX-License-Identifier: GPL-2.0 */
#include <syscall.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

static int mlock2_(void *start, size_t len, int flags)
{
	int ret = syscall(__NR_mlock2, start, len, flags);

	if (ret) {
		errno = ret;
		return -1;
	}
	return 0;
}

static FILE *seek_to_smaps_entry(unsigned long addr)
{
	FILE *file;
	char *line = NULL;
	size_t size = 0;
	unsigned long start, end;
	char perms[5];
	unsigned long offset;
	char dev[32];
	unsigned long inode;
	char path[BUFSIZ];

	file = fopen("/proc/self/smaps", "r");
	if (!file)
		ksft_exit_fail_msg("fopen smaps: %s\n", strerror(errno));

	while (getline(&line, &size, file) > 0) {
		if (sscanf(line, "%lx-%lx %s %lx %s %lu %s\n",
			   &start, &end, perms, &offset, dev, &inode, path) < 6)
			goto next;

		if (start <= addr && addr < end)
			goto out;

next:
		free(line);
		line = NULL;
		size = 0;
	}

	fclose(file);
	file = NULL;

out:
	free(line);
	return file;
}
