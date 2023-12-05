#ifndef AMBIORIX_HANDLER
#define AMBIORIX_HANDLER

#include <iostream>
#include <string>

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
    std::string name;
    actions_callback callback;
} sReadActionsCallback;

typedef struct sEventsCallback {
    std::string name;
    events_callback callback;
} sEventsCallback;

typedef struct sRPCFunctions { //the path should be the name, the name should be ID
    std::string path_id;
    std::string name;
    ambiorix_func_ptr callback;
} sRPCFunctions;

//Events
void odl_exec_env_added(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void odl_exec_env_changed(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void odl_exec_env_uninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void odl_deployment_unit_added(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void odl_deployment_unit_changed(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void odl_deployment_unit_uninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv);
//RPCs
amxd_status_t odl_function_exec_env_add(amxd_object_t *object, amxd_function_t *func, amxc_var_t *args, amxc_var_t *ret);
//RO params
amxd_status_t odl_action_read_ee_status(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv);
amxd_status_t odl_action_read_ee_Reset(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv);
amxd_status_t odl_action_read_ee_CurrentRunLevel(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv);

amxd_status_t odl_action_read_ee_AvailableDiskSpace(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv);

amxd_status_t odl_action_read_ee_AvailableMemory(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv);

amxd_status_t odl_action_read_ee_ActiveExecutionUnits(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv);
#endif
