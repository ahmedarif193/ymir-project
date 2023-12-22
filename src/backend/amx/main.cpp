#include "amxhandler.h"
#include <amxrt/amxrt.h>

#include "utils/vector.h"
#include "utils/sharedptr.h"
class AmxrtResource {
public:
    AmxrtResource() {
        amxrt_new();
    }

    ~AmxrtResource() {
        amxrt_stop();
        amxrt_delete();
    }
};

int main(int argc, char* argv[]) {
    AmxrtResource amxrtResource; // RAII for amxrt resources
    int retval = 0;
    int index = 0;

    lxcd::SharedPtr<amxc_var_t> config(amxrt_get_config());
    lxcd::SharedPtr<amxd_dm_t> dm(amxrt_get_dm());
    lxcd::SharedPtr<amxo_parser_t> parser(amxrt_get_parser());
    retval = amxrt_config_init(argc, argv, &index, NULL);
    if (retval != 0) {
        printf("Configuration initialization failed\n");
        return retval;
    }

    lxcd::vector<sReadActionsCallback> actions_list;
    actions_list.push_back({"action_read_ee_status", odlActionReadEeStatus});
    actions_list.push_back({"action_read_ee_Reset", odlActionReadEeReset});
    actions_list.push_back({"action_read_ee_CurrentRunLevel", odlActionReadEeCurrentrunlevel});
    actions_list.push_back({"action_read_ee_AvailableDiskSpace", odlActionReadEeAvailablediskspace});
    actions_list.push_back({"action_read_ee_AvailableMemory", odlActionReadEeAvailablememory});
    actions_list.push_back({"action_read_ee_ActiveExecutionUnits", odlActionReadEeActiveexecutionunits});

    lxcd::vector<sEventsCallback> events_list;
    events_list.push_back({"execution_unit_manage", ExecutionUnitManage});
    events_list.push_back({"deployment_unit_manage", deploymentUnitManage});
    events_list.push_back({"app_start", ExecEnvManage});
    events_list.push_back({"print_event", odlPrintEvent});
    events_list.push_back({"exec_env_changed", odlExecEnvChanged});
    events_list.push_back({"exec_env_uninstalled", odlExecEnvUninstalled});
    events_list.push_back({"deployment_unit_changed", odlDeploymentUnitChanged});
    events_list.push_back({"deployment_unit_uninstalled", odlDeploymentUnitUninstalled});
    events_list.push_back({"exec_unit_changed", odlExecutionUnitChanged});

    lxcd::vector<sRPCFunctions> functions_list;
    functions_list.push_back({"odl_function_exec_env_add", "SoftwareModules.addExecEnv", odlFunctionExecEnvAdd});
    functions_list.push_back({"odl_function_deployment_unit_add", "SoftwareModules.addDeploymentUnit", odlFunctionDeploymentUnitAdd});

    auto add_callbacks = [&retval](const auto& list, auto adder, auto& parser) {
        for (const auto& item : list) {
            retval = adder(parser.get(), item.name.c_str(), reinterpret_cast<amxo_fn_ptr_t>(item.callback));
            if (retval != 0) {
                printf("Error: Unable to add callback for '%s'. Function returned: %d\n", item.name.c_str(), retval);
                return;
            }
            printf("Success: Callback for '%s' has been successfully added to the function table.\n", item.name.c_str());
        }
    };

    add_callbacks(actions_list, amxo_resolver_ftab_add, parser);
    add_callbacks(events_list, amxo_resolver_ftab_add, parser);
    add_callbacks(functions_list, amxo_resolver_ftab_add, parser);

    retval = amxrt_load_odl_files(argc, argv, index);
    if (retval != 0) {
        printf("Loading ODL files failed\n");
        return retval;
    }

    // Add entry point to the parser
    amxo_parser_add_entry_point(parser.get(), amxrt_dm_save_load_main);

    // Connect
    retval = amxrt_connect();
    if (retval != 0) {
        printf("Connection failed\n");
        return retval;
    }

    // Enable system signals if available
    auto syssigs = GET_ARG(config.get(), "system-signals");
    if (syssigs) {
        amxrt_enable_syssigs(syssigs);
    }

    // Create event loop
    retval = amxrt_el_create();
    if (retval != 0) {
        printf("Event loop creation failed\n");
        return retval;
    }

    // Register or wait
    retval = amxrt_register_or_wait();
    if (retval != 0) {
        printf("Register or wait failed\n");
        return retval;
    }
    amxrt_el_start();

    return retval;
}
