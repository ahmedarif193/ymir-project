#include "amxhandler.h"
#include <iostream>
#include "lxc-container.h"
#include <amxrt/amxrt.h>

static void helper_exec_env_set_status(amxd_object_t* obj, bool enable) {
    amxd_trans_t transaction;
    const char* alias = NULL;
    amxd_dm_t* dm = amxrt_get_dm();
    amxd_trans_init(&transaction);
    amxd_trans_set_attr(&transaction, amxd_tattr_change_ro, true);

    alias = amxc_var_constcast(cstring_t, amxd_object_get_param_value(obj, "Alias"));
    amxd_trans_select_object(&transaction, obj);
    if(enable) {
        amxd_trans_set_value(cstring_t, &transaction, "Status", "Up");
    } else {
        amxd_trans_set_value(cstring_t, &transaction, "Status", "Down");
    }
    amxd_trans_apply(&transaction, dm);
    amxd_trans_clean(&transaction);
}

static void helper_exec_env_set_name(amxd_object_t* obj, const char* name_str) {
    amxd_trans_t transaction;
    const char* alias = NULL;
    amxd_dm_t* dm = amxrt_get_dm();
    amxd_trans_init(&transaction);
    amxd_trans_set_attr(&transaction, amxd_tattr_change_ro, true);

    alias = amxc_var_constcast(cstring_t, amxd_object_get_param_value(obj, "Alias"));
    amxd_trans_select_object(&transaction, obj);
    amxd_trans_set_value(cstring_t, &transaction, "Name", name_str);
    amxd_trans_apply(&transaction, dm);
    amxd_trans_clean(&transaction);
}


//if defined, but Enable = 0 just create the instance,
void odl_exec_env_added(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    amxd_object_t *ee_obj = amxd_dm_signal_get_object(amxrt_get_dm(), data);
    if (!ee_obj) {
        printf("Failed to get object exec_env\n");
        return;
    }

    int index = GET_UINT32(data, "index");
    const char* name_str = amxd_object_get_cstring_t(ee_obj, "Name", nullptr);
    bool enable_bool = amxd_object_get_bool(ee_obj, "Enable", NULL);
    auto template_str = amxd_object_get_cstring_t(ee_obj, "Type", nullptr);
    char formatted_name[256];
    if (strlen(name_str) > 0) {
        snprintf(formatted_name, sizeof(formatted_name), "%s", name_str);
    } else {
        snprintf(formatted_name, sizeof(formatted_name), "Container-%d", index);
    }
    printf("Name: %s\n", formatted_name);
    printf("odl_exec_env_added Name len -: %ld\n", strlen(name_str));
    
    std::cout << "amxd_object_get_bool exec_env value " << enable_bool<< std::endl;
    LxcContainer container(formatted_name);
    if(enable_bool){
        container.setAction(Method::ENABLE);
    }
    container.setTemplate(template_str);
    int retcode = container.run();
    // helper_exec_env_set_status(ee_obj, );
    // helper_exec_env_set_name(ee_obj, formatted_name);
    amxd_object_set_cstring_t(ee_obj, "Status", "Up");
    amxd_object_set_cstring_t(ee_obj, "Name", formatted_name);
}

void odl_exec_env_changed(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_exec_env_changed" << std::endl;
    amxd_object_t *ee_obj = amxd_dm_signal_get_object(amxrt_get_dm(), data);
    if (!ee_obj) {
        std::cout << "Failed to get object exec_env" << std::endl;
        return;
    }
    auto name_str = amxd_object_get_cstring_t(ee_obj, "Name", nullptr);
    auto template_str = amxd_object_get_cstring_t(ee_obj, "Type", nullptr);

    bool enable_bool = amxd_object_get_bool(ee_obj, "Enable", 0);
    std::cout << "amxd_object_get_bool exec_env value " << enable_bool<< std::endl;
    LxcContainer container(name_str);
    if(enable_bool){
        container.setAction(Method::ENABLE);
    }else{
        container.setAction(Method::DISABLE);
    }
    container.setTemplate(template_str);
    container.run();
}

void odl_exec_env_uninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_exec_env_uninstalled" << std::endl;

    amxd_object_t *ee_obj = amxd_dm_signal_get_object(amxrt_get_dm(), data);
    if (!ee_obj) {
        std::cout << "Failed to get object exec_env" << std::endl;
        return;
    }
    auto name_str = amxd_object_get_cstring_t(ee_obj, "Name", nullptr);

    LxcContainer container(name_str);
    container.setAction(Method::DESTROY);
    container.run();
}

void odl_deployment_unit_added(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_deployment_unit_added" << std::endl;
}

void odl_deployment_unit_changed(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_deployment_unit_changed" << std::endl;
}

void odl_deployment_unit_uninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_deployment_unit_uninstalled" << std::endl;
}

amxd_status_t odl_function_exec_env_add(amxd_object_t *object, amxd_function_t *func, amxc_var_t *args, amxc_var_t *ret){ 
    std::cout << "Entered function: odl_function_exec_env_add" << std::endl;
    return amxd_status_ok;
};

amxd_status_t odl_action_read_ee_status(amxd_object_t *object, amxd_param_t *param, amxd_action_t reason, const amxc_var_t *const args, amxc_var_t *const retval, void *priv) {
    //std::cout << "Entered function: action_read_ee_status" << std::endl;
    if (reason != action_param_read) {
        return amxd_status_function_not_implemented;
    }
    if (!param) {
        return amxd_status_parameter_not_found;
    }
    auto status = amxd_action_param_read(object, param, reason, args, retval, priv);
    if (status != amxd_status_ok) {
        return status;
    }

    amxd_param_t *name_param = amxd_object_get_param_def(object, "Name");
    if (name_param == nullptr) {
        std::cout << "Name cannot be read in EE datamodel";
        return amxd_status_parameter_not_found;
    }
    auto name_param_str = amxc_var_constcast(cstring_t, &name_param->value);

    struct lxc_container *cont = lxc_container_new(name_param_str, NULL);
    if (!cont) {
        std::cout << "Failed to create container object for: " << name_param_str << std::endl;
        amxc_var_set(cstring_t, retval, "Error");
        return amxd_status_unknown_error;
    }

    if (cont->is_defined(cont) && cont->is_running(cont)) {
        amxc_var_set(cstring_t, retval, "UP");
    } else {
        amxc_var_set(cstring_t, retval, "Disabled");
    }

    lxc_container_put(cont);

    return amxd_status_ok;
}


amxd_status_t odl_action_read_ee_Reset(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv){
std::cout << "Entered function: action_read_ee_Reset" << std::endl;
return amxd_status_ok;
};
amxd_status_t odl_action_read_ee_CurrentRunLevel(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv){
//std::cout << "Entered function: action_read_ee_CurrentRunLevel" << std::endl;
return amxd_status_ok;
};

amxd_status_t odl_action_read_ee_AvailableDiskSpace(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv){
//std::cout << "Entered function: action_read_ee_AvailableDiskSpace" << std::endl;
return amxd_status_ok;
};

amxd_status_t odl_action_read_ee_AvailableMemory(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv){
//std::cout << "Entered function: action_read_ee_AvailableMemory" << std::endl;
return amxd_status_ok;
};

amxd_status_t odl_action_read_ee_ActiveExecutionUnits(amxd_object_t *object, amxd_param_t *param,
                                            amxd_action_t reason, const amxc_var_t *const args,
                                            amxc_var_t *const retval, void *priv){
//std::cout << "Entered function: action_read_ee_ActiveExecutionUnits" << std::endl;
return amxd_status_ok;
};
