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

/**
 * @defgroup eap_simaka_rest_provider eap_simaka_rest_provider
 * @{ @ingroup eap_simaka_rest
 */

#ifndef eap_simaka_rest_CLIENT_H_
#define eap_simaka_rest_CLIENT_H_


#include <library.h>
#include <json.h>

typedef struct eap_simaka_rest_client_t eap_simaka_rest_client_t;

/**
 * Public REST interface
 */
struct eap_simaka_rest_client_t {
	/**
	 * Send an HTTP POST request including a JSON object
	 *
	 * @param jreq		JSON object in HTTP request
	 * @param jresp		JSON object in HTTP response
	 * @return			Status (SUCCESS or FAILED)
	 */
	status_t (*post)(eap_simaka_rest_client_t *this, char *command, json_object *jreq,
					 json_object **jresp);

	/**
	 * Destroy eap_simaka_rest_client_t object
	 */
	void (*destroy)(eap_simaka_rest_client_t *this);

};

/**
 * Create an eap_simaka_rest_client_t instance
 *
 * @param uri			REST URI (http://username:password@hostname[:port]/api/)
 * @param timeout		Timeout of the REST connection
 */
eap_simaka_rest_client_t* eap_simaka_rest_client_create(char *uri, u_int timeout);

#endif /** eap_simaka_rest_CLIENT_H_ @}*/