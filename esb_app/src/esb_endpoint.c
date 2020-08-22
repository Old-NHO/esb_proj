/*
 * Copyright (c) 2016-2018 Joris Vink <joris@coders.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This example demonstrates how to properly deal with file uploads
 * coming from a multipart form.
 *
 * The basics are quite trivial:
 *	1) call http_populate_multipart_form()
 *	2) find the file using http_file_lookup().
 *	3) read the file data using http_file_read().
 *
 * In this example the contents is written to a newly created file
 * on the server that matches the naming given by the uploader.
 *
 * Note that the above is probably not what you want to do in real life.
 */

#include <kore/kore.h>
#include <kore/http.h>

#include <fcntl.h>
#include <unistd.h>
#include "esb.h"

/**
 * Define a suitable struct for holding the endpoint request handling result.
 */
typedef struct
{
	int status;
	char *bmd_path;
} endpoint_result;

int esb_endpoint(struct http_request *);
endpoint_result save_bmd(struct http_request *);

/**
 * HTTP request handler function mapped in the conf file.
 */
int esb_endpoint(struct http_request *req)
{
	printf("Received the BMD request.\n");
	endpoint_result epr = save_bmd(req);
	if (epr.status < 0)
	{
		printf("Failed to handle the BMD request.\n");
		return (KORE_RESULT_ERROR);
	}
	else
	{
		/* Invoke the ESB's main processing logic. */
		int esb_status = process_esb_request(epr.bmd_path);
		if (0 == esb_status)
		{
			//TODO: Take suitable action
			return (KORE_RESULT_OK);
		}
		else
		{
			//TODO: Take suitable action
			printf("ESB failed to process the BMD.\n");
			return (KORE_RESULT_ERROR);
		}
	}
}

endpoint_result
save_bmd(struct http_request *req)
{
	endpoint_result ep_res;
	/* Default to error */
	ep_res.status = -1;

	int fd;
	struct http_file *file;
	u_int8_t buf[BUFSIZ];
	ssize_t ret, written;

	/* Only deal with POSTs. */
	if (req->method != HTTP_METHOD_POST)
	{
		kore_log(LOG_ERR, "Rejecting non POST request.");
		http_response(req, 405, NULL, 0);
		return ep_res;
	}

	/* Parse the multipart data that was present. */
	http_populate_multipart_form(req);

	/* Find our file. It is expected to be under parameter named bmd_file */
	if ((file = http_file_lookup(req, "bmd_file")) == NULL)
	{
		http_response(req, 400, NULL, 0);
		return ep_res;
	}

	/* Open dump file where we will write file contents. */
	fd = open(file->filename, O_CREAT | O_TRUNC | O_WRONLY, 0700);
	if (fd == -1)
	{
		http_response(req, 500, NULL, 0);
		return ep_res;
	}

	/* While we have data from http_file_read(), write it. */
	/* Alternatively you could look at file->offset and file->length. */
	/**
	 * TODO: The BMD should be saved at a proper unique file path.
	 * That path then should be returned to the caller for allowing
	 * further processing by the ESB.
	 */
	for (;;)
	{
		ret = http_file_read(file, buf, sizeof(buf));
		if (ret == -1)
		{
			kore_log(LOG_ERR, "failed to read from file");
			http_response(req, 500, NULL, 0);
			goto cleanup;
		}

		if (ret == 0)
			break;

		written = write(fd, buf, ret);
		if (written == -1)
		{
			kore_log(LOG_ERR, "write(%s): %s",
					 file->filename, errno_s);
			http_response(req, 500, NULL, 0);
			goto cleanup;
		}

		if (written != ret)
		{
			kore_log(LOG_ERR, "partial write on %s",
					 file->filename);
			http_response(req, 500, NULL, 0);
			goto cleanup;
		}
	}

	http_response(req, 200, NULL, 0);
	kore_log(LOG_INFO, "file '%s' successfully received",
			 file->filename);
	ep_res.bmd_path = malloc(strlen(file->filename) * sizeof(char) + 1);
	strcpy(ep_res.bmd_path, file->filename);

cleanup:
	if (close(fd) == -1)
		kore_log(LOG_WARNING, "close(%s): %s", file->filename, errno_s);

	if (ep_res.status < 0)
	{
		if (unlink(file->filename) == -1)
		{
			kore_log(LOG_WARNING, "unlink(%s): %s",
					 file->filename, errno_s);
		}
		ep_res.status = 0;
	}
	printf("BMD is saved\n");
	return ep_res;
}