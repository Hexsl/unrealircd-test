/* spamfilter.* RPC calls
 * (C) Copyright 2022-.. Bram Matthys (Syzop) and the UnrealIRCd team
 * License: GPLv2 or later
 */

#include "unrealircd.h"

ModuleHeader MOD_HEADER
= {
	"rpc/spamfilter",
	"1.0.0",
	"spamfilter.* RPC calls",
	"UnrealIRCd Team",
	"unrealircd-6",
};

/* Forward declarations */
RPC_CALL_FUNC(rpc_spamfilter_list);
RPC_CALL_FUNC(rpc_spamfilter_get);
RPC_CALL_FUNC(rpc_spamfilter_del);
RPC_CALL_FUNC(rpc_spamfilter_add);

MOD_INIT()
{
	RPCHandlerInfo r;

	MARK_AS_OFFICIAL_MODULE(modinfo);

	memset(&r, 0, sizeof(r));
	r.method = "spamfilter.list";
	r.call = rpc_spamfilter_list;
	if (!RPCHandlerAdd(modinfo->handle, &r))
	{
		config_error("[rpc/spamfilter] Could not register RPC handler");
		return MOD_FAILED;
	}
	r.method = "spamfilter.get";
	r.call = rpc_spamfilter_get;
	if (!RPCHandlerAdd(modinfo->handle, &r))
	{
		config_error("[rpc/spamfilter] Could not register RPC handler");
		return MOD_FAILED;
	}
	r.method = "spamfilter.del";
	r.call = rpc_spamfilter_del;
	if (!RPCHandlerAdd(modinfo->handle, &r))
	{
		config_error("[rpc/spamfilter] Could not register RPC handler");
		return MOD_FAILED;
	}
	r.method = "spamfilter.add";
	r.call = rpc_spamfilter_add;
	if (!RPCHandlerAdd(modinfo->handle, &r))
	{
		config_error("[rpc/spamfilter] Could not register RPC handler");
		return MOD_FAILED;
	}

	return MOD_SUCCESS;
}

MOD_LOAD()
{
	return MOD_SUCCESS;
}

MOD_UNLOAD()
{
	return MOD_SUCCESS;
}

RPC_CALL_FUNC(rpc_spamfilter_list)
{
	json_t *result, *list, *item;
	int index, index2;
	TKL *tkl;

	result = json_object();
	list = json_array();
	json_object_set_new(result, "list", list);

	for (index = 0; index < TKLISTLEN; index++)
	{
		for (tkl = tklines[index]; tkl; tkl = tkl->next)
		{
			if (TKLIsSpamfilter(tkl))
			{
				item = json_object();
				json_expand_tkl(item, NULL, tkl, 1);
				json_array_append_new(list, item);
			}
		}
	}

	rpc_response(client, request, result);
	json_decref(result);
}

RPC_CALL_FUNC(rpc_spamfilter_get)
{
}

RPC_CALL_FUNC(rpc_spamfilter_del)
{
}

RPC_CALL_FUNC(rpc_spamfilter_add)
{
	json_t *result, *list, *item;
	int type = TKL_SPAMF|TKL_GLOBAL;
	const char *error;
	const char *str;
	const char *name, *reason;
	time_t ban_duration = 0;
	TKL *tkl;
	Match *m;
	BanAction action;
	int match_type = 0;
	int targets = 0;
	char targetbuf[64];
	char actionbuf[2];
	char *err = NULL;

	name = json_object_get_string(params, "name");
	if (!name)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Missing parameter: 'name'");
		return;
	}

	str = json_object_get_string(params, "match_type");
	if (!str)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Missing parameter: 'match_type'");
		return;
	}
	match_type = unreal_match_method_strtoval(str);
	if (!match_type)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Invalid value for parameter 'match_type'");
		return;
	}

	str = json_object_get_string(params, "spamfilter_targets");
	if (!str)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Missing parameter: 'spamfilter_targets'");
		return;
	}
	targets = spamfilter_gettargets(str, NULL);
	if (!targets)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Invalid value(s) for parameter 'spamfilter_targets'");
		return;
	}
	strlcpy(targetbuf, spamfilter_target_inttostring(targets), sizeof(targetbuf));

	str = json_object_get_string(params, "ban_action");
	if (!str)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Missing parameter: 'ban_action'");
		return;
	}
	action = banact_stringtoval(str);
	if (!action)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Invalid value for parameter 'ban_action'");
		return;
	}
	actionbuf[0] = banact_valtochar(action);
	actionbuf[1] = '\0';

	reason = json_object_get_string(params, "reason");
	if (!reason)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Missing parameter: 'reason'");
		return;
	}

	/* Ban duration */
	if ((str = json_object_get_string(params, "ban_duration")))
	{
		ban_duration = config_checkval(str, CFG_TIME);
		if (ban_duration < 0)
		{
			rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Invalid value for parameter 'ban_duration'");
			return;
		}
	}

	if (find_tkl_spamfilter(type, name, action, targets))
	{
		rpc_error(client, request, JSON_RPC_ERROR_ALREADY_EXISTS, "A spamfilter with that regex+action+target already exists");
		return;
	}

	/* now check the regex / match field (only when adding) */
	m = unreal_create_match(match_type, name, &err);
	if (!m)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INVALID_PARAMS, "Invalid regex or match string specified");
		return;
	}

	tkl = tkl_add_spamfilter(type, targets, action, m, client->name, 0, TStime(),
	                         ban_duration, reason, 0);

	if (!tkl)
	{
		rpc_error(client, request, JSON_RPC_ERROR_INTERNAL_ERROR, "Unable to add item");
		return;
	}

	tkl_added(client, tkl);

	result = json_object();
	json_expand_tkl(result, "tkl", tkl, 1);
	rpc_response(client, request, result);
	json_decref(result);
}
