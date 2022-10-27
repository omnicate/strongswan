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

#include "eap_simaka_rest_client.h"
#include "eap_simaka_rest_provider.h"

#include <time.h>

#include <daemon.h>

#include <json.h>

typedef struct private_eap_simaka_rest_provider_t private_eap_simaka_rest_provider_t;

/**
 * Private data of an eap_simaka_rest_provider_t object.
 */
struct private_eap_simaka_rest_provider_t {

	/**
	 * Public eap_simaka_rest_provider_t interface.
	 */
	eap_simaka_rest_provider_t public;

	/**
	 * REST client for quintuplets
	 */
	eap_simaka_rest_client_t *client;
};

size_t bytes_to_hex(char *buf, int len, char **out)
{
	const size_t outlen = len * 2;
	*out = malloc(outlen + 1);
	for (int i = 0; i < len; i++)
	{
		sprintf(*out + i * 2, "%02x", buf[i]);
	}
	return outlen;
}

void log_bytes(char *buf, int len)
{
	char *out;
	bytes_to_hex(buf, len, &out);
	DBG1(DBG_CFG, out);
	free(out);
}

bool json_field_to_bytes(json_object *obj, const char* key, char* out, int* out_len)
{
	json_object *val;
	if(!json_object_object_get_ex(obj, key, &val) || !json_object_is_type(val, json_type_string))
	{
		return FALSE;
	}
	const char* strval = json_object_get_string(val);
	int byte_len = strlen(strval) / 2;

	if (byte_len == 0)
	{
		return FALSE;
	}

	if (out_len) {
		*out_len = byte_len;
	}

	for (int i = 0; i < byte_len; i++)
	{
		if (sscanf(strval + i * 2, "%02x", &out[i]) != 1) {
			return FALSE;
		}
	}
	return TRUE;
}


METHOD(simaka_provider_t, get_quintuplet, bool,
	private_eap_simaka_rest_provider_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char xres[AKA_RES_MAX], int *xres_len,
	char ck[AKA_CK_LEN], char ik[AKA_IK_LEN], char autn[AKA_AUTN_LEN])
{
	char id_buf[128];

	snprintf(id_buf, sizeof(id_buf), "%Y", id);

	json_object *jreq, *jresp;

	jreq = json_object_new_object();
	json_object_object_add(jreq, "id", json_object_new_string(id_buf));
	
	DBG1(DBG_CFG, "requesting a quintuplet via REST: %s", (char*)json_object_to_json_string(jreq));
	status_t post_res = this->client->post(this->client, "vectors", jreq, &jresp);

	DBG1(DBG_CFG, "status: %d", (int)post_res);

	if (post_res != SUCCESS)
	{
		return FALSE;
	}
	DBG1(DBG_CFG, "quintuplet: %s", (char*)json_object_to_json_string(jresp));

	if (!json_field_to_bytes(jresp, "rand", rand, NULL))
	{
		DBG1(DBG_CFG, "rand is missing/malformed!");
		return FALSE;
	}
	if (!json_field_to_bytes(jresp, "xres", xres, xres_len))
	{
		DBG1(DBG_CFG, "xres is missing/malformed!");
		return FALSE;
	}
	if (!json_field_to_bytes(jresp, "ck", ck, NULL))
	{
		DBG1(DBG_CFG, "ck is missing/malformed!");
		return FALSE;
	}
	if (!json_field_to_bytes(jresp, "ik", ik, NULL))
	{
		DBG1(DBG_CFG, "ik is missing/malformed!");
		return FALSE;
	}
	if (!json_field_to_bytes(jresp, "autn", autn, NULL))
	{
		DBG1(DBG_CFG, "autn is missing/malformed!");
		return FALSE;
	}
	return TRUE;
}

METHOD(eap_simaka_rest_provider_t, destroy, void,
	private_eap_simaka_rest_provider_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_simaka_rest_provider_t *eap_simaka_rest_provider_create(eap_simaka_rest_client_t *client)
{
	private_eap_simaka_rest_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.get_triplet = (void*)return_false,
				.get_quintuplet = _get_quintuplet,
				.resync = (void*)return_false,
				.is_pseudonym = (void*)return_null,
				.gen_pseudonym = (void*)return_null,
				.is_reauth = (void*)return_null,
				.gen_reauth = (void*)return_null,
			},
			.destroy = _destroy,
		},
		.client = client,
	);

	return &this->public;
}
