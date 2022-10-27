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
#include "eap_simaka_rest_plugin.h"
#include "eap_simaka_rest_provider.h"

#include <daemon.h>

typedef struct private_eap_simaka_rest_t private_eap_simaka_rest_t;

/**
 * Private data of an eap_simaka_rest_t object.
 */
struct private_eap_simaka_rest_t {

	/**
	 * Public eap_simaka_rest_plugin_t interface.
	 */
	eap_simaka_rest_plugin_t public;

	/**
	 * (U)SIM provider
	 */
	eap_simaka_rest_provider_t *provider;

	/**
	 * REST client for quintuplets
	 */
	eap_simaka_rest_client_t *client;
};

METHOD(plugin_t, get_name, char*,
	private_eap_simaka_rest_t *this)
{
	return "eap-simaka-rest";
}

/**
 * Initialize things
 */
static bool init(private_eap_simaka_rest_t *this,
					plugin_feature_t *feature, bool reg, void *data)
{
	if (reg)
	{
		char *uri;
		int timeout;

		uri = lib->settings->get_str(lib->settings,
									 "%s.plugins.eap-simaka-rest.uri", NULL,
									 lib->ns);
		if (!uri)
		{
			DBG1(DBG_CFG, "eap-simaka-rest database URI missing");
			return FALSE;
		}
		timeout = lib->settings->get_int(lib->settings,
								"%s.plugins.eap-simaka-rest.timeout", 5,
								lib->ns);
		this->client = eap_simaka_rest_client_create(uri, timeout);
		this->provider = eap_simaka_rest_provider_create(this->client);
		return TRUE;
	}
	this->provider->destroy(this->provider);
	this->client->destroy(this->client);
	this->provider = NULL;
	this->client = NULL;
	return TRUE;
}

/**
 * Callback providing our provider to register
 */
static simaka_provider_t* get_provider(private_eap_simaka_rest_t *this)
{
	return &this->provider->provider;
}

METHOD(plugin_t, get_features, int,
	private_eap_simaka_rest_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_CALLBACK((void*)init, NULL),
			PLUGIN_PROVIDE(CUSTOM, "eap-simaka-rest-init"),
		PLUGIN_CALLBACK(simaka_manager_register, get_provider),
			PLUGIN_PROVIDE(CUSTOM, "aka-provider"),
				PLUGIN_DEPENDS(CUSTOM, "aka-manager"),
				PLUGIN_DEPENDS(CUSTOM, "eap-simaka-rest-init"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_eap_simaka_rest_t *this)
{
	free(this);
}

/**
 * See header
 */
plugin_t *eap_simaka_rest_plugin_create()
{
	private_eap_simaka_rest_t *this;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	return &this->public.plugin;
}
