/*
 * applywim.c - A program to extract the first image from a WIM file.
 *
 * The following copying information applies to this specific source code file:
 *
 * Written in 2013-2014 by Eric Biggers <ebiggers3@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all copyright
 * and related and neighboring rights to this software to the public domain
 * worldwide via the Creative Commons Zero 1.0 Universal Public Domain
 * Dedication (the "CC0").
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the CC0 for more details.
 *
 * You should have received a copy of the CC0 along with this software; if not
 * see <http://creativecommons.org/publicdomain/zero/1.0/>.
 */

#include <wimlib.h>
#include <stdio.h>

#define TO_PERCENT(numerator, denominator) \
	((float)(((denominator) == 0) ? 0 : ((numerator) * 100 / (float)(denominator))))

static enum wimlib_progress_status
extract_progress(enum wimlib_progress_msg msg,
		 union wimlib_progress_info *info, void *progctx)
{
	switch (msg) {
	case WIMLIB_PROGRESS_MSG_EXTRACT_STREAMS:
		printf("Extracting files: %.2f%% complete\n",
		       TO_PERCENT(info->extract.completed_bytes,
				  info->extract.total_bytes));
		break;
	default:
		break;
	}
	return WIMLIB_PROGRESS_STATUS_CONTINUE;
}

int main(int argc, char **argv)
{
	int ret;
	WIMStruct *wim = NULL;
	const char *wimpath;
	const char *destdir;

	/* Check for the correct number of arguments.  */
	if (argc != 3) {
		fprintf(stderr, "Usage: applywim WIM DIR\n");
		return 2;
	}

	wimpath = argv[1];
	destdir = argv[2];

	/* Open the WIM file as a WIMStruct.  */
	ret = wimlib_open_wim(wimpath,  /* Path of WIM file to open  */
			      0,        /* WIMLIB_OPEN_FLAG_* flags (0 means all defaults)  */
			      &wim);    /* Return the WIMStruct pointer in this location  */
	if (ret != 0) /* Always should check the error codes.  */
		goto out;

	/* Register our progress function.  */
	wimlib_register_progress_function(wim, extract_progress, NULL);

	/* Extract the first image.  */
	ret = wimlib_extract_image(wim,     /* WIMStruct from which to extract the image  */
				   1,       /* Image to extract  */
				   destdir, /* Directory to extract the image to  */
				   0);      /* WIMLIB_EXTRACT_FLAG_* flags (0 means all defaults)  */

out:
	/* Free the WIMStruct.  Has no effect if the pointer to it is NULL.  */
	wimlib_free(wim);

	/* Check for error status.  */
	if (ret != 0) {
		fprintf(stderr, "wimlib error %d: %s\n",
			ret, wimlib_get_error_string(ret));
	}

	/* Free global memory (optional).  */
	wimlib_global_cleanup();

	return ret;
}
