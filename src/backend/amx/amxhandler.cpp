#include "amxhandler.h"
#include <iostream>

void odl_exec_env_added(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_exec_env_added" << std::endl;
    // Function implementation
}

void odl_exec_env_changed(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_exec_env_changed" << std::endl;
    // Function implementation
}

void odl_exec_env_uninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_exec_env_uninstalled" << std::endl;
    // Function implementation
}

void odl_deployment_unit_added(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_deployment_unit_added" << std::endl;
    // Function implementation
}

void odl_deployment_unit_changed(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_deployment_unit_changed" << std::endl;
    // Function implementation
}

void odl_deployment_unit_uninstalled(const char *const sig_name, const amxc_var_t *const data, void *const priv){
    std::cout << "Entered function: odl_deployment_unit_uninstalled" << std::endl;
    // Function implementation
}

amxd_status_t odl_function_exec_env_add(amxd_object_t *object, amxd_function_t *func, amxc_var_t *args, amxc_var_t *ret){ 
    std::cout << "Entered function: odl_function_exec_env_add" << std::endl;
    // Function implementation
};

amxd_status_t action_read_ee_status(amxd_object_t *object, amxd_param_t *param, amxd_action_t reason, const amxc_var_t *const args, amxc_var_t *const retval, void *priv){
    std::cout << "Entered function: action_read_ee_status" << std::endl;
    // Function implementation
}
