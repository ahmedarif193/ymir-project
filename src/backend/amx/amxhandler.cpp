#include "lxc-container.h"
#include "utils/uuid.h"

#include "amxhandler.h"
#include <amxc/amxc.h>
#include <amxd/amxd_object.h>
#include <amxrt/amxrt.h>

lxcd::map<lxcd::string, lxcd::SharedPtr<lxcd::CProcess>> processMap;
lxcd::map<lxcd::string, lxcd::SharedPtr<lxcd::CProcess>>::Iterator retrieveOrInitializeProcess(
        const lxcd::string& serviceName,
        const lxcd::string& execEnvName) {

    auto processEntry = processMap.find(serviceName);
    if (processEntry == processMap.end()) {
        processEntry = processMap.insert(lxcd::make_pair(serviceName, lxcd::makeShared<lxcd::CProcess>())).key;
        processEntry->value->setContainer(execEnvName);
    }
    return processEntry;
}

enum class EventType {
    Unknown,
    AppStart,
    ContainerStop,
    ContainerStart
};

void odlPrintEvent(const char* const sig_name,
                   const amxc_var_t* const data,
                   UNUSED void* const priv) {
    printf("Signal received - %s\n", sig_name);
    printf("Signal data = \n");
    fflush(stdout);
    if(!amxc_var_is_null(data)) {
        amxc_var_dump(data, STDOUT_FILENO);
    }
}

void dump_amxd_object_parameters(amxd_object_t* object) {
    if (object == NULL) {
        printf("Object is NULL\n");
        return;
    }

    amxc_var_t plist;
    amxc_var_init(&plist);

    // Assuming this function lists all parameters in 'plist'
    if (amxd_object_list_params(object, &plist, amxd_dm_access_private) != 0) {
        printf("Failed to list parameters\n");
        return;
    }

    // Iterate through the list of parameters
    amxc_var_t* param_name;
    amxc_llist_t* it;
    amxc_llist_for_each(it, amxc_var_constcast(amxc_llist_t, &plist)) {
        param_name = amxc_var_from_llist_it(it);
        if (param_name != NULL && amxc_var_type_of(param_name) == AMXC_VAR_ID_CSTRING) {
            const char* name = amxc_var_constcast(cstring_t, param_name);

            // Hypothetical function to get the value of a parameter
            const char* value = amxc_var_constcast(cstring_t, amxd_object_get_param_value(object, name));
            printf("-- -- -- -- Parameter: %s, Value: %s\n", name, value);
        }
    }

    amxc_var_clean(&plist);
}
//TODO transactions unused, use them later
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
        amxd_trans_set_value(cstring_t, &transaction, "Status", "Disabled");
    }
    amxd_trans_apply(&transaction, dm);
    amxd_trans_clean(&transaction);
}

//----------------------managing EU
//TODO : Add number of failed starts here- maybe (5 times before stop auto starts)
auto serviceExitCallback = [](int exitcode, const lxcd::string& output, void* priv){
    amxd_object_t* executionUnit = (amxd_object_t*)priv;
    printf("service stopped\n");
    if(executionUnit){
        lxcd::string serviceName = amxd_object_get_cstring_t(executionUnit, "Name", nullptr);
        printf("[Info] Process named '%s' is stopped.\n", serviceName.c_str());
        amxc_var_t params;
        amxc_var_init(&params);
        amxc_var_set_type(&params, AMXC_VAR_ID_HTABLE);
        amxc_var_add_key(cstring_t, &params, "Status", "Idle");
        amxd_object_emit_changed(executionUnit, &params);
        amxc_var_clean(&params);

    }
};

void processExecutionUnitObject(amxd_object_t* executionUnit) {
    if (!executionUnit) {
        printf("[Error] Execution unit object is null.\n");
        return;
    }

    lxcd::string serviceName = amxd_object_get_cstring_t(executionUnit, "Name", nullptr);
    lxcd::string execEnvName = amxd_object_get_cstring_t(executionUnit, "ExecEnvLabel", 0);
    lxcd::string requestedState = amxd_object_get_cstring_t(executionUnit, "RequestedState", 0);
    lxcd::string executionCommand = amxd_object_get_cstring_t(executionUnit, "X_LXCD-COM_exec", 0);

    if (serviceName.empty() || execEnvName.empty()) {
        printf("[Error] Service name or execution environment name is empty.\n");
        return;
    }

    printf("[Info] Processing Execution Unit. Service Name: '%s', Execution Environment: '%s', Requested State: '%s'\n",
           serviceName.c_str(), execEnvName.c_str(), requestedState.c_str());

    auto processEntry = retrieveOrInitializeProcess(serviceName, execEnvName);
    printf("[Info] Process entry retrieved or initialized for service '%s'.\n", serviceName.c_str());

    if (requestedState == "Active") {
        if (!processEntry->value->isRunning()) {
            printf("[Action] Initiating startup of service '%s'.\n", serviceName.c_str());
            processEntry->value->runAsync(executionCommand, serviceExitCallback, executionUnit);
        } else {
            printf("[Warning] Attempt to start service '%s' which is already active.\n", serviceName.c_str());
        }
    } else if (requestedState == "Idle") {
        if (processEntry->value->isRunning()) {
            printf("[Action] Initiating shutdown of service '%s'.\n", serviceName.c_str());
            processEntry->value->stop();
        } else {
            printf("[Warning] Attempt to stop service '%s' which is already idle.\n", serviceName.c_str());
        }
    }

    bool isRunning = processEntry->value->isRunning();
    printf("[Info] Service '%s' status after processing: %s.\n", serviceName.c_str(), isRunning ? "Running" : "Not Running");
    amxd_object_set_cstring_t(executionUnit, "Status", isRunning ? "Active" : "Idle");
}

void helperEuLoopEnable(amxd_object_t* const executionUnit, int32_t depth, void* priv) {
    if (!executionUnit || !priv) {
        printf("[Error] Invalid arguments in helperEuLoopEnable.\n");
        return;
    }

    amxc_var_t* args = static_cast<amxc_var_t*>(priv);
    EventType eventType = static_cast<EventType>(GET_INT32(args, "EventType"));
    printf("[Info] helperEuLoopEnable called for event.\n");

    switch (eventType) {
        case EventType::AppStart: {
            bool autoStart = amxd_object_get_bool(executionUnit, "AutoStart", nullptr);
            if (autoStart) {
                lxcd::string requestedState = "Active";
                amxd_object_set_cstring_t(executionUnit, "RequestedState", requestedState.c_str());
                printf("[Action] Auto-starting service for APPSTART event.\n");
            }
            break;
        }
        case EventType::ContainerStop:
        case EventType::ContainerStart: {
            lxcd::string eventContainerName = GET_CHAR(args, "Name");
            lxcd::string execEnvName = amxd_object_get_cstring_t(executionUnit, "ExecEnvLabel", nullptr);
            if (eventContainerName != execEnvName) {
                printf("[Info] '%s' is not the target service, continue looping\n", execEnvName.c_str());
                return;
            }
            lxcd::string requestedState = (eventType == EventType::ContainerStop) ? "Idle" : "Active";
            amxd_object_set_cstring_t(executionUnit, "RequestedState", requestedState.c_str());
            printf("[Action] %s service for event.\n", requestedState.c_str());
            break;
        }
        default:
            printf("[Warning] Unknown event type.\n");
            break;
    }

    processExecutionUnitObject(executionUnit);
}

void ExecutionUnitManage(const char* const signalName, const amxc_var_t* const data, void* priv) {
    if (!signalName || !data) {
        printf("[Error] Invalid arguments in ExecutionUnitManage.\n");
        return;
    }
    amxd_object_t* deploymentUnitObject = amxd_dm_findf(amxrt_get_dm(), "%s", "SoftwareModules.ExecutionUnit.");
    if (!deploymentUnitObject) {
        printf("[Error] Failed to find Deployment Unit Object.\n");
        return;
    }
    amxd_object_hierarchy_walk(deploymentUnitObject, amxd_direction_down, NULL, helperEuLoopEnable, 5, (void*)data);
}

//----------------------managing DU

void processDeploymentUnitObject(amxd_object_t* deploymentUnitObject) {
    if (!deploymentUnitObject) {
        printf("[Error] Deployment unit object is null.\n");
        return;
    }

    lxcd::string deploymentUnitUrl = amxd_object_get_cstring_t(deploymentUnitObject, "URL", nullptr);
    lxcd::string deploymentUnitUuid = amxd_object_get_cstring_t(deploymentUnitObject, "UUID", nullptr);
    lxcd::string executionEnvironmentName = amxd_object_get_cstring_t(deploymentUnitObject, "ExecutionEnvRef", nullptr);

    if (deploymentUnitUrl.empty() || executionEnvironmentName.empty()) {
        printf("[Error] URL or Execution Environment Name is empty in deployment unit object.\n");
        return;
    }

    printf("[Info] Processing Deployment Unit. UUID: %s, Execution Environment: %s\n",
           deploymentUnitUuid.c_str(), executionEnvironmentName.c_str());

    if (deploymentUnitUuid.empty()) {
        amxd_object_set_cstring_t(deploymentUnitObject, "Status", "Installing");

        DeploymentUnitHelper helper;
        if (deploymentUnitUuid.empty()) {
            deploymentUnitUuid = lxcd::UUIDGenerator::generate();
        }

        lxcd::SharedPtr<DeploymentUnit> deploymentUnit = helper.addDeploymentUnit(executionEnvironmentName, deploymentUnitUrl, deploymentUnitUuid);
        amxd_object_set_cstring_t(deploymentUnitObject, "UUID", deploymentUnitUuid.c_str());

        lxcd::string deploymentUnitId = executionEnvironmentName + "-" + deploymentUnitUuid;
        amxd_object_set_cstring_t(deploymentUnitObject, "DUID", deploymentUnitId.c_str());
        amxd_object_set_cstring_t(deploymentUnitObject, "Name", deploymentUnit->name.c_str());
        amxd_object_set_bool(deploymentUnitObject, "Resolved", true);
        amxd_object_set_cstring_t(deploymentUnitObject, "URL", deploymentUnitUrl.c_str());
        amxd_object_set_cstring_t(deploymentUnitObject, "Description", deploymentUnit->description.c_str());
        amxd_object_set_cstring_t(deploymentUnitObject, "Vendor", deploymentUnit->vendor.c_str());
        amxd_object_set_int32_t(deploymentUnitObject, "Version", deploymentUnit->version);
        amxd_object_set_cstring_t(deploymentUnitObject, "ExecutionEnvRef", executionEnvironmentName.c_str());
        amxd_object_set_cstring_t(deploymentUnitObject, "Status", deploymentUnit->name.empty() ? "Uninstalled" : "Installed");
    }
}

void helperDuLoopEnable(amxd_object_t* const duObj, int32_t depth, void* priv) {
    processDeploymentUnitObject(duObj);
}

void deploymentUnitManage(const char* const signalName, const amxc_var_t* const data, void* priv) {
    if (!signalName) {
        printf("[Error] Signal name is null in deploymentUnitManage.\n");
        return;
    }

    printf("[Info] Managing Deployment Units at app start. Signal: %s\n", signalName);

    amxd_object_t* deploymentUnitObject = amxd_dm_findf(amxrt_get_dm(), "%s", "SoftwareModules.DeploymentUnit.");
    if (!deploymentUnitObject) {
        printf("[Error] Failed to find Deployment Unit Object.\n");
        return;
    }
    amxd_object_hierarchy_walk(deploymentUnitObject, amxd_direction_down, NULL, helperDuLoopEnable, 5, nullptr);

    amxc_var_t args;
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);

    printf("[Info] SIGNAL: sending 'deploymentunit-managed' with 'EventType' as 'APPSTART'\n");
    amxc_var_add_key(int8_t, &args, "EventType", static_cast<int>(EventType::AppStart));
    amxd_object_send_signal(amxd_dm_get_root(amxrt_get_dm()), "lxcd:deploymentunit-managed", &args, true);

    amxc_var_clean(&args);
}

//----------------------managing EE
void processExecEnvObject(amxd_object_t* executionEnvironmentObject) {
    if (!executionEnvironmentObject) {
        printf("[Error] Execution environment object is null.\n");
        return;
    }

    printf("[Info] Processing Execution Environment Object.\n");

    // Extracting Execution Environment parameters
    lxcd::string execEnvName = amxd_object_get_cstring_t(executionEnvironmentObject, "Name", nullptr);
    bool execEnvEnabled = amxd_object_get_bool(executionEnvironmentObject, "Enable", nullptr);
    lxcd::string execEnvTemplate = amxd_object_get_cstring_t(executionEnvironmentObject, "Type", nullptr);
    lxcd::string allocatedStorage = amxd_object_get_cstring_t(executionEnvironmentObject, "AllocatedDiskSpace", nullptr);

    printf("[Info] Execution Environment: Name: %s, Enable: %d, Template: %s\n",
           execEnvName.c_str(), execEnvEnabled, execEnvTemplate.c_str());

    if (execEnvName.empty() || execEnvTemplate.empty()) {
        printf("[Error] Missing name or template for Execution Environment.\n");
        return;
    }

    // Setting status to 'Working'
    amxd_object_set_cstring_t(executionEnvironmentObject, "Status", "Working");

    // Configuring and running container
    LxcContainer container(execEnvName);
    container.setTemplate(execEnvTemplate);
    container.setStorageSpace(lxcd::string::stoi(allocatedStorage));
    container.setAction(execEnvEnabled ? Method::ENABLE : Method::DISABLE);

    int operationResult = container.run();
    lxcd::string status = operationResult ? "Error" : execEnvEnabled ? "Up" : "Disabled";
    amxd_object_set_cstring_t(executionEnvironmentObject, "Status", status.c_str());
}

void helperEELoopEnable(amxd_object_t* const executionEnvironmentObject, int32_t depth, void* priv) {
    if (!executionEnvironmentObject) {
        printf("[Error] Execution environment object is null in helperEELoopEnable.\n");
        return;
    }

    char* execEnvPath = amxd_object_get_path(executionEnvironmentObject, AMXD_OBJECT_TERMINATE | AMXD_OBJECT_INDEXED);
    if (execEnvPath) {
        printf("[Info] Processing Execution Environment at path: %s\n", execEnvPath);
        free(execEnvPath);
    }

    processExecEnvObject(executionEnvironmentObject);
}

void ExecEnvManage(const char* const signalName, const amxc_var_t* const data, void* priv) {
    printf("[Info] ExecEnvManage triggered. Signal: %s\n", signalName ? signalName : "null");

    amxd_object_t* execEnvObject = amxd_dm_findf(amxrt_get_dm(), "%s", "SoftwareModules.ExecEnv.");
    if (!execEnvObject) {
        printf("[Error] Failed to find Execution Environment Object.\n");
        return;
    }
    amxd_object_hierarchy_walk(execEnvObject, amxd_direction_down, NULL, helperEELoopEnable, 5, NULL);

    amxc_var_t args;
    amxc_var_init(&args);
    amxc_var_set_type(&args, AMXC_VAR_ID_HTABLE);
    amxc_var_add_key(int8_t, &args, "EventType", static_cast<int>(EventType::AppStart));
    amxd_object_send_signal(amxd_dm_get_root(amxrt_get_dm()), "lxcd:execenv-managed", &args, true);
    amxc_var_clean(&args);
}

void odlExecEnvChanged(const char* const signalName, const amxc_var_t* const data, void* priv) {
    printf("[Info] Entered function: odlExecEnvChanged. Signal: %s\n", signalName ? signalName : "null");

    amxd_object_t* execEnvObject = amxd_dm_signal_get_object(amxrt_get_dm(), data);
    if (!execEnvObject) {
        printf("[Error] Failed to get Execution Environment object.\n");
        return;
    }

    processExecEnvObject(execEnvObject);
}

//TODO need to check if there is any associated DU and remove them
void odlExecEnvUninstalled(const char* const signalName, const amxc_var_t* const data, void* priv) {
    printf("[Info] Entered function: odlExecEnvUninstalled. Signal: %s\n", signalName ? signalName : "null");

    const char* execEnvName = GETP_CHAR(data, "parameters.Name");
    if (!execEnvName) {
        printf("[Error] Execution Environment name is missing.\n");
        return;
    }

    LxcContainer container(execEnvName);
    container.setAction(Method::DESTROY);
    int operationResult = container.run();
    if (operationResult != 0) {
        printf("[Error] Failed to destroy Execution Environment: %s\n", execEnvName);
    } else {
        printf("[Info] Successfully destroyed Execution Environment: %s\n", execEnvName);
    }
}

void odlDeploymentUnitChanged(const char* const signalName, const amxc_var_t* const data, void* priv) {
    printf("[Info] Entered function: odlDeploymentUnitChanged. Signal: %s\n", signalName ? signalName : "null");
}

//TODO : if du deleted remove all recurrent EU
void odlDeploymentUnitUninstalled(const char* const signalName, const amxc_var_t* const data, void* priv) {
    printf("[Info] Entered function: odlDeploymentUnitUninstalled. Signal: %s\n", signalName ? signalName : "null");

    lxcd::string deploymentUnitUuid = GETP_CHAR(data, "parameters.UUID");
    if (deploymentUnitUuid.empty()) {
        printf("[Error] Deployment Unit UUID is missing.\n");
        return;
    }

    DeploymentUnitHelper helper;
    helper.removeDeploymentUnit(deploymentUnitUuid);
    printf("[Info] Deployment Unit with UUID: %s removed.\n", deploymentUnitUuid.c_str());
}

void odlExecutionUnitChanged(const char* const signalName, const amxc_var_t* const data, void* priv) {
    printf("[Info] odlExecutionUnitChanged triggered. Signal: %s\n", signalName ? signalName : "null");

    amxd_object_t* execUnitObject = amxd_dm_signal_get_object(amxrt_get_dm(), data);
    if (!execUnitObject) {
        printf("[Error] Failed to get Execution Unit object.\n");
        return;
    }

    processExecutionUnitObject(execUnitObject);
}

amxd_status_t odlFunctionExecEnvAdd(amxd_object_t* object, amxd_function_t* func, amxc_var_t* args, amxc_var_t* ret) {
    printf("Starting Execution Environment addition process.\n");

    // Extracting values from arguments
    lxcd::string execEnvName = GET_CHAR(args, "name");
    lxcd::string execEnvType = GET_CHAR(args, "type");
    int allocatedStorage = GET_INT32(args, "storage");

    // Validating input parameters
    if (execEnvName.empty() || execEnvType.empty()) {
        printf("Error: Missing Execution Environment name or type.\n");
        return amxd_status_parameter_not_found;
    }

    if (allocatedStorage <= 10) {
        printf("Error: Allocated storage must be at least 10 MB.\n");
        return amxd_status_parameter_not_found;
    }

    // Retrieving Execution Environment object from data model
    amxd_object_t* execEnvObject = amxd_object_get_child(object, "ExecEnv");
    if (!execEnvObject) {
        printf("Error: Execution Environment object not found in the data model.\n");
        return amxd_status_object_not_found;
    }

    // Creating a new Execution Environment instance
    amxd_object_t* execEnvInstance = nullptr;
    amxc_var_clean(ret);
    amxd_object_add_instance(&execEnvInstance, execEnvObject, nullptr, 0, nullptr);

    if (!execEnvInstance) {
        printf("Error: Failed to create a new Execution Environment instance.\n");
        return amxd_status_unknown_error;
    }

    // Setting up the Execution Environment instance
    printf("Configuring Execution Environment: %s, Type: %s, Allocated Storage: %d MB\n", execEnvName.c_str(), execEnvType.c_str(), allocatedStorage);
    amxd_object_set_cstring_t(execEnvInstance, "Name", execEnvName.c_str());
    amxd_object_set_cstring_t(execEnvInstance, "Type", execEnvType.c_str());
    amxd_object_set_int32_t(execEnvInstance, "AllocatedDiskSpace", allocatedStorage);
    amxd_object_set_bool(execEnvInstance, "Enable", true);
    amxd_object_emit_add_inst(execEnvInstance);

    // Processing the Execution Environment
    processExecEnvObject(execEnvInstance);

    return amxd_status_ok;
}

amxd_status_t odlFunctionDeploymentUnitAdd(amxd_object_t* object, amxd_function_t* func, amxc_var_t* args, amxc_var_t* ret) {
    if (!object || !args) {
        printf("[Error] Invalid arguments in odlFunctionDeploymentUnitAdd.\n");
        return amxd_status_parameter_not_found;
    }

    lxcd::string deploymentUnitUrl = GET_CHAR(args, "du_url");
    lxcd::string executionEnvironmentName = GET_CHAR(args, "ee_name");
    lxcd::string deploymentUnitUuid = lxcd::UUIDGenerator::generate();
    lxcd::string deploymentUnitId = executionEnvironmentName + "-" + deploymentUnitUuid;

    if (deploymentUnitUrl.empty() || executionEnvironmentName.empty()) {
        printf("[Error] Missing DeploymentUnit URL or Execution Environment name.\n");
        return amxd_status_parameter_not_found;
    }

    DeploymentUnitHelper helper;
    lxcd::SharedPtr<DeploymentUnit> deploymentUnit = helper.addDeploymentUnit(executionEnvironmentName, deploymentUnitUrl, deploymentUnitUuid);

    if (deploymentUnit->name.empty()) {
        printf("[Error] Failed to install DeploymentUnit.\n");
        return amxd_status_unknown_error;
    }

    amxd_object_t* deploymentUnitInstance = nullptr;
    amxd_object_t* deploymentUnitObject = amxd_object_get_child(object, "DeploymentUnit");

    if (!deploymentUnitObject) {
        printf("[Error] DeploymentUnit object not found in data model.\n");
        return amxd_status_object_not_found;
    }

    amxc_var_clean(ret);
    amxd_object_add_instance(&deploymentUnitInstance, deploymentUnitObject, nullptr, 0, nullptr);

    if (!deploymentUnitInstance) {
        printf("[Error] Could not create DeploymentUnit instance.\n");
        return amxd_status_unknown_error;
    }

    amxd_object_set_cstring_t(deploymentUnitInstance, "UUID", deploymentUnitUuid.c_str());
    amxd_object_set_cstring_t(deploymentUnitInstance, "DUID", deploymentUnitId.c_str());
    amxd_object_set_cstring_t(deploymentUnitInstance, "Name", deploymentUnit->name.c_str());
    amxd_object_set_cstring_t(deploymentUnitInstance, "Status", "Installed");
    amxd_object_set_bool(deploymentUnitInstance, "Resolved", true);
    amxd_object_set_cstring_t(deploymentUnitInstance, "URL", deploymentUnitUrl.c_str());
    amxd_object_set_cstring_t(deploymentUnitInstance, "Description", deploymentUnit->description.c_str());
    amxd_object_set_cstring_t(deploymentUnitInstance, "Vendor", deploymentUnit->vendor.c_str());
    amxd_object_set_int32_t(deploymentUnitInstance, "Version", deploymentUnit->version);
    amxd_object_set_cstring_t(deploymentUnitInstance, "ExecutionEnvRef", executionEnvironmentName.c_str());
    amxd_object_emit_add_inst(deploymentUnitInstance);

    dump_amxd_object_parameters(deploymentUnitInstance);

    printf("[Info] DeploymentUnit added successfully.\n");

    for (const auto& service : deploymentUnit->executionunits) {
        amxd_object_t* executionUnitInstance = nullptr;
        amxd_object_t* executionUnitObject = amxd_object_get_child(object, "ExecutionUnit");

        if (!executionUnitObject) {
            printf("Error: executionUnit object not found in data model\n");
            return amxd_status_object_not_found;
        }

        amxc_var_clean(ret);
        amxd_object_add_instance(&executionUnitInstance, executionUnitObject, nullptr, 0, nullptr);

        if (!executionUnitInstance) {
            printf("Error: Could not create executionUnit instance\n");
            return amxd_status_unknown_error;
        }
        printf("installing service with COM_exec %s\n", service.exec.c_str());
        amxd_object_set_cstring_t(executionUnitInstance, "Name", service.name);
        amxd_object_set_cstring_t(executionUnitInstance, "ExecEnvLabel", executionEnvironmentName);
        amxd_object_set_cstring_t(executionUnitInstance, "EUID", executionEnvironmentName +"-"+ deploymentUnit->uuid);
        amxd_object_set_cstring_t(executionUnitInstance, "X_LXCD-COM_exec", service.exec);
        amxd_object_set_bool(executionUnitInstance, "AutoStart", service.autostart);
        lxcd::string RequestedState = service.autostart ?"Active":"Idle";
        amxd_object_set_cstring_t(executionUnitInstance, "RequestedState", RequestedState);
        amxd_object_emit_add_inst(executionUnitInstance);
    }

    return amxd_status_ok;
}

amxd_status_t odlActionReadEeStatus(amxd_object_t* object, amxd_param_t* param, amxd_action_t reason, const amxc_var_t* const args, amxc_var_t* const retval, void* priv) {
    printf("Entering odlActionReadEeStatus\n");

    if (reason != action_param_read) {
        printf("Action not implemented for non-read operations\n");
        return amxd_status_function_not_implemented;
    }

    if (!param) {
        printf("Parameter not found\n");
        return amxd_status_parameter_not_found;
    }

    amxd_status_t readStatus = amxd_action_param_read(object, param, reason, args, retval, priv);
    if (readStatus != amxd_status_ok) {
        printf("Failed to read parameter value\n");
        return readStatus;
    }

    amxd_param_t* nameParam = amxd_object_get_param_def(object, "Name");
    if (!nameParam) {
        printf("Failed to read 'Name' parameter in Execution Environment data model\n");
        return amxd_status_parameter_not_found;
    }
    const char* containerName = amxc_var_constcast(cstring_t, &nameParam->value);

    LxcContainer container(containerName);
    amxc_var_set(cstring_t, retval,container.isRunning()? "UP" : "Disabled");

    return amxd_status_ok;
}

amxd_status_t odlActionReadEeReset(amxd_object_t *object, amxd_param_t *param,
                                   amxd_action_t reason, const amxc_var_t *const args,
                                   amxc_var_t *const retval, void *priv){
    return amxd_status_ok;
}

amxd_status_t odlActionReadEeCurrentrunlevel(amxd_object_t *object, amxd_param_t *param,
                                             amxd_action_t reason, const amxc_var_t *const args,
                                             amxc_var_t *const retval, void *priv){
    //printf("Entered function: action_read_ee_CurrentRunLevel\n");
    return amxd_status_ok;
}

amxd_status_t odlActionReadEeAvailablediskspace(amxd_object_t* object, amxd_param_t* param,
                                                amxd_action_t reason, const amxc_var_t* const args,
                                                amxc_var_t* const retval, void* priv) {
    if (reason != action_param_read) {
        printf("[Error] odlActionReadEeAvailablediskspace called with invalid reason.\n");
        return amxd_status_function_not_implemented;
    }
    if (!object || !param) {
        printf("[Error] Object or parameter is null in odlActionReadEeAvailablediskspace.\n");
        return amxd_status_parameter_not_found;
    }

    amxd_status_t status = amxd_action_param_read(object, param, reason, args, retval, priv);
    if (status != amxd_status_ok) {
        printf("[Error] amxd_action_param_read failed in odlActionReadEeAvailablediskspace. Status: %d\n", status);
        return status;
    }

    const char* execEnvName = GETP_CHAR(args, "parameters.Name");
    if (!execEnvName) {
        printf("[Error] Execution environment name not found in arguments.\n");
        return amxd_status_parameter_not_found;
    }
    printf("[Info] Execution environment name retrieved: %s\n", execEnvName);

    //lxcd::string execEnvNameStr = get_param_string(object, "Name");
    // Additional processing can be done here if needed

    return amxd_status_ok;
}

amxd_status_t odlActionReadEeAvailablememory(amxd_object_t *object, amxd_param_t *param,
                                             amxd_action_t reason, const amxc_var_t *const args,
                                             amxc_var_t *const retval, void *priv){
    return amxd_status_ok;
}

amxd_status_t odlActionReadEeActiveexecutionunits(amxd_object_t *object, amxd_param_t *param,
                                                  amxd_action_t reason, const amxc_var_t *const args,
                                                  amxc_var_t *const retval, void *priv){
    return amxd_status_ok;
}
