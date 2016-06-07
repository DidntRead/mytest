
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/limits.h>

#include "util.h"

#include "log.h"


// -----------------------------------------------------------------------------
// Function definitions
// -----------------------------------------------------------------------------
int util_read_int(const char *path, const char *dst_file, int *val)
{
	int ret = 0;
	FILE *fptr;
	char filename[PATH_MAX];
	size_t num_bytes;

	char temp_str[100]; // todo
	char *dst;
	int tempint;

	if ((strlen(path) == 0) || (strlen(dst_file) == 0))
	{
		return -EINVAL;
	}

	strcpy(filename, path);

	if(filename[strlen(filename) - 1] != '/')
	{
		strcat(filename, "/");
	}

	strcat(filename, dst_file);

	fptr = fopen(filename, "r");

	if(fptr == NULL)
	{
		ret = -errno;
		LOGE("Unable to open %s for read (%d)\n", filename, ret);
	}

	else
	{
		num_bytes = 0;
		dst = temp_str;
	
		while (!feof(fptr) && (num_bytes < sizeof(temp_str)))
		{
			num_bytes += fread(dst, 1, sizeof(char), fptr);
			dst++;
		}
		*dst = '\0';

		fclose(fptr);

		if(num_bytes > 0)
		{
			ret = num_bytes;
			tempint = atoi(temp_str);
			*val = tempint;
			LOGD("Read %d from %s\n", tempint, filename);
		}
		else
		{
			ret = -ENOENT;
		}
	}
	return ret;
}


// -----------------------------------------------------------------------------
int util_write_int(const char *path, const char *dst_file, int val)
{
	int ret = 0;
	FILE *fptr;

	char filename[PATH_MAX];
	char output[sizeof("123456789012345")];

	if ((strlen(path) == 0) || (strlen(dst_file) == 0))
	{
		return -EINVAL;
	}

	strcpy(filename, path);

	if(filename[strlen(filename) - 1] != '/')
	{
		strcat(filename, "/");
	}

	strcat(filename, dst_file);

	fptr = fopen(filename, "w");

	if(fptr == NULL)
	{
		ret = -errno;
		LOGE("Unable to open %s for write (%d)\n", filename, ret);
	}
	else
	{
		sprintf(output, "%d", val);

		ret = fwrite(output, 1, strlen(output), fptr);
		fflush(fptr);
		fclose(fptr);

		if (ret == strlen(output))
		{
			LOGD("Wrote %s to %s\n", output, filename);
			ret = 0;
		}
		else
		{
			LOGE("Error writing %s (%d)\n", filename, ret);
		}
	}

	return ret;
}

// -----------------------------------------------------------------------------


