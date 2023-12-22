#ifndef AMBIORIX_HANDLER
#define AMBIORIX_HANDLER

#include "common-amx.h"

//Events deployment_unit_manage
void ExecutionUnitManage(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void deploymentUnitManage(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void ExecEnvManage(const char *const sig_name, const amxc_var_t *const data, void *const priv);

void odlPrintEvent(const char *const sig_name, const amxc_var_t *const data, void *const priv);

void odlExecEnvChanged(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void odlExecEnvUninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv);

void odlDeploymentUnitChanged(const char *const sig_name, const amxc_var_t *const data, void *const priv);
void odlDeploymentUnitUninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv);

void odlExecutionUnitChanged(const char *const sig_name, const amxc_var_t *const data, void *const priv);

//RPCs
amxd_status_t odlFunctionExecEnvAdd(amxd_object_t *object, amxd_function_t *func, amxc_var_t *args, amxc_var_t *ret);
amxd_status_t odlFunctionDeploymentUnitAdd(amxd_object_t *object, amxd_function_t *func, amxc_var_t *args, amxc_var_t *ret);
//RO params
amxd_status_t odlActionReadEeStatus(amxd_object_t *object, amxd_param_t *param,
                                    amxd_action_t reason, const amxc_var_t *const args,
                                    amxc_var_t *const retval, void *priv);
amxd_status_t odlActionReadEeReset(amxd_object_t *object, amxd_param_t *param,
                                   amxd_action_t reason, const amxc_var_t *const args,
                                   amxc_var_t *const retval, void *priv);
amxd_status_t odlActionReadEeCurrentrunlevel(amxd_object_t *object, amxd_param_t *param,
                                             amxd_action_t reason, const amxc_var_t *const args,
                                             amxc_var_t *const retval, void *priv);

amxd_status_t odlActionReadEeAvailablediskspace(amxd_object_t *object, amxd_param_t *param,
                                                amxd_action_t reason, const amxc_var_t *const args,
                                                amxc_var_t *const retval, void *priv);

amxd_status_t odlActionReadEeAvailablememory(amxd_object_t *object, amxd_param_t *param,
                                             amxd_action_t reason, const amxc_var_t *const args,
                                             amxc_var_t *const retval, void *priv);

amxd_status_t odlActionReadEeActiveexecutionunits(amxd_object_t *object, amxd_param_t *param,
                                                  amxd_action_t reason, const amxc_var_t *const args,
                                                  amxc_var_t *const retval, void *priv);

#endif
