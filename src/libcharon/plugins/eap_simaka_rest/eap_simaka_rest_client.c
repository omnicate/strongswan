/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */


#define _GNU_SOURCE
#include <stdio.h>

#include "eap_simaka_rest_client.h"

typedef struct private_eap_simaka_rest_client_t private_eap_simaka_rest_client_t;

/**
 * Private data of an rest_t object.
 */
struct private_eap_simaka_rest_client_t {

	/**
	 * Public members of rest_t
	 */
	eap_simaka_rest_client_t public;

	/**
	 * URI of REST API
	 */
	char *uri;

	/**
	 * Timeout of REST API connection
	 */
	u_int timeout;

};


#define HTTP_STATUS_CODE_NOT_FOUND				404
#define HTTP_STATUS_CODE_PRECONDITION_FAILED	412

METHOD(eap_simaka_rest_client_t, post, status_t,
	private_eap_simaka_rest_client_t *this, char *command, json_object *jrequest,
	json_object **jresponse)
{
	struct json_tokener *tokener;
	chunk_t data, response = chunk_empty;
	status_t status;
	char *uri;
	int code;

	if (asprintf(&uri, "%s%s",this->uri, command) < 0)
	{
		return FAILED;
	}
	data = chunk_from_str((char*)json_object_to_json_string(jrequest));

	status = lib->fetcher->fetch(lib->fetcher, uri, &response,
				FETCH_TIMEOUT, this->timeout,
				FETCH_REQUEST_DATA, data,
				FETCH_REQUEST_TYPE, "application/json; charset=utf-8",
				FETCH_REQUEST_HEADER, "Accept: application/json",
				FETCH_REQUEST_HEADER, "Expect:",
				FETCH_RESPONSE_CODE, &code,
				FETCH_END);
	free(uri);

	if (status != SUCCESS)
	{
		switch (code)
		{
			case HTTP_STATUS_CODE_NOT_FOUND:
				status = NOT_FOUND;
				break;
			default:
				DBG2(DBG_IMV, "REST http request failed with status code: %d",
							   code);
				status = FAILED;
				break;
		}
	}
    else
    {
        if (jresponse)
        {
            /* Parse HTTP response into a JSON object */
            tokener = json_tokener_new();
            *jresponse = json_tokener_parse_ex(tokener, response.ptr,
                                                        response.len);
            json_tokener_free(tokener);
        }
    }
	free(response.ptr);

	return status;
}

METHOD(eap_simaka_rest_client_t, destroy, void,
	private_eap_simaka_rest_client_t *this)
{
	free(this->uri);
	free(this);
}

/**
 * Described in header.
 */
eap_simaka_rest_client_t *eap_simaka_rest_client_create(char *uri, u_int timeout)
{
	private_eap_simaka_rest_client_t *this;

	INIT(this,
		.public = {
			.post = _post,
			.destroy = _destroy,
		},
		.uri = strdup(uri),
		.timeout = timeout,
	);

	return &this->public;
}