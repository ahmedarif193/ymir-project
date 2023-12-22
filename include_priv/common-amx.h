#ifndef COMMON_AMX_HANDLER
#define COMMON_AMX_HANDLER


#include "utils/string.h"

#include <amxc/amxc.h>
#include <amxp/amxp.h>

#include <amxc/amxc.h>
#include <amxd/amxd_action.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxd/amxd_object_event.h>
#include <amxd/amxd_transaction.h>

#include <amxb/amxb.h>
#include <amxb/amxb_register.h>

#include <amxo/amxo.h>
#include <amxo/amxo_save.h>

using actions_callback = amxd_status_t (*)(amxd_object_t *object, amxd_param_t *param,
                                           amxd_action_t reason, const amxc_var_t *const args,
                                           amxc_var_t *const retval, void *priv);

using events_callback = void (*)(const char *const sig_name, const amxc_var_t *const data,
                                 void *const priv);

using ambiorix_func_ptr = amxd_status_t (*)(amxd_object_t *object, amxd_function_t *func,
                                            amxc_var_t *args, amxc_var_t *ret);

typedef struct sReadActionsCallback {
	lxcd::string name;
	actions_callback callback;
} sReadActionsCallback;

typedef struct sEventsCallback {
	lxcd::string name;
	events_callback callback;
} sEventsCallback;

typedef struct sRPCFunctions { //the path should be the name, the name should be ID
	lxcd::string path_id;
	lxcd::string name;
	ambiorix_func_ptr callback;
} sRPCFunctions;

#endif
