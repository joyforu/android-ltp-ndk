#include "libcontrollers.h"

/*
 * Function: scan_shares_file()
 * This function scans all the shares files under the mountpoint
 * of the controller and returns the total added shares of all
 * the groups (currently excludes default group) ??
 */
int scan_shares_files ()
{
	struct stat 	statbuffer;
	DIR		*dp;
	char		*path_pointer;

	/*
	 * Check if we can get stat of the file
	 */
	if (lstat (fullpath, &statbuffer) < 0)
		error_function("Can not read stat for file ", fullpath);

	if (S_ISDIR (statbuffer.st_mode) == 0) /* not a directory */
	{
	/*
	 * We run all user tasks in the created groups and not default groups. So
	 * exclude the shares of default group. FLAG to ensure dir_pointer is non NULL
	 */
		if ((FLAG == 1) && (strcmp(fullpath, "/dev/cpuctl/cpu.shares") != 0) &&\
			 (strcmp (dir_pointer->d_name, "cpu.shares") == 0))
		{
			*shares_pointer += read_shares_file (fullpath);
		}
		return 0;
	}

	/*
	 * Now it's a directory. let the path_pointer point to the end
	 * of fullpath to append new files names
	 */

	path_pointer = fullpath + strlen(fullpath);
	*path_pointer++ = '/';
	*path_pointer	= 0;

	if ((dp = opendir(fullpath)) == NULL) /* Error in opening directory */
		error_function("Can't open ", fullpath);
	/*
	 * search all groups recursively and get total shares
	 */

	while ((dir_pointer = readdir(dp)) != NULL) /* Error in reading directory */
	{
		if ((strcmp(dir_pointer->d_name, ".") == 0) || (strcmp(dir_pointer->d_name, "..") == 0))
			continue;	/* ignore current and parent directory */

		FLAG = 1;
		strcpy (path_pointer, dir_pointer->d_name ); /* append name to fullpath */

		if ((retval = scan_shares_files()) != 0)
			break;
	}

	/*
	 * This directory is searched fully. let us go back to parent directory again
	 */

	path_pointer[-1] = 0;

	if (closedir (dp) < 0)
		error_function("Could not close dir ", fullpath);
	return 0;
}

/*
 * Function: read_file ()
 * This function is written keeping in mind the support
 * to read other files also if required in future.
 * Each file under a group contains some diff parameter/s
 */

int read_file(char *filepath, int action, unsigned int *value)
{
	int num_line = 0;
	FILE *fp;
	switch (action)
	{
		case GET_SHARES:
			*value = read_shares_file(filepath);
			if (*value == -1)
				return -1;
			break;

		case GET_TASKS:
			fp = fopen (filepath, "r");
			if (fp == NULL)
				error_function("Could not open file", filepath);
			while(fgets(target, LINE_MAX, fp) != NULL)
				num_line++;
			*value = (unsigned int)num_line;
			if (fclose (fp))
				error_function ("Could not close file", filepath);
			break;

		default:
			error_function("Wrong action type passed to fun read_file for ", filepath);
			break;
	}
	return 0;
}

/*
 * Function: errir_function()
 * Prints error message and returns -1
 */

inline int error_function(char *msg1, char *msg2)
{
	fprintf (stdout,"ERROR: %s ", msg1);
	fprintf (stdout,"%s\n", msg2);
	return -1;

}

/* Function: read_shares_file()
 * Reads shares value from a given shares file and writes them to
 * the given pointer location. Returns 0 if success
 */

unsigned
int read_shares_file (char *filepath)
{
	FILE *fp;
	unsigned int shares;
	fp = fopen (filepath, "r");
	if (fp == NULL)
		error_function("Could not open file", filepath);
	fscanf (fp, "%u", &shares);
	if (fclose (fp))
		error_function ("Could not close file", filepath);
	return shares;
}

/* Function: write_to_file()
 * writes value to shares file or pid to tasks file
 */

int write_to_file (char *file, const char *mode, unsigned int value)
{
	FILE *fp;
	fp = fopen (file, mode);
	if (fp == NULL)
		error_function ("in opening file for writing:", file);
	fprintf (fp, "%u\n", value);
	fclose (fp);
	return 0;
}
/* Function: signal_handler_alarm()
 * signal handler for the new action
 */

void signal_handler_alarm (int signal)
{
	timer_expired = 1;
}
