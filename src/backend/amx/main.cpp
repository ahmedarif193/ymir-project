#include "amxhandler.h"
#include <amxrt/amxrt.h>
#include <vector>
#include <iostream>
#include <memory>

// Custom wrapper for amxrt resources.
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

    std::unique_ptr<amxc_var_t> syssigs;
    std::unique_ptr<amxc_var_t> config(amxrt_get_config());
    std::unique_ptr<amxd_dm_t> dm(amxrt_get_dm());
    std::unique_ptr<amxo_parser_t> parser(amxrt_get_parser());
    retval = amxrt_config_init(argc, argv, &index, NULL);
    if (retval != 0) {
        std::cerr << "Configuration initialization failed\n";
        return retval;
    }

    // Define the callbacks list
    const std::vector<sReadActionsCallback> actions_list = {
        {"action_read_ee_status", odl_action_read_ee_status},
        {"action_read_ee_Reset", odl_action_read_ee_Reset},
        {"action_read_ee_CurrentRunLevel", odl_action_read_ee_CurrentRunLevel},
        {"action_read_ee_AvailableDiskSpace", odl_action_read_ee_AvailableDiskSpace},
        {"action_read_ee_AvailableMemory", odl_action_read_ee_AvailableMemory},
        {"action_read_ee_ActiveExecutionUnits", odl_action_read_ee_ActiveExecutionUnits}
    };
    const std::vector<sEventsCallback> events_list = {
        {"exec_env_added", odl_exec_env_added},
        {"exec_env_changed", odl_exec_env_changed},
        {"exec_env_uninstalled", odl_exec_env_uninstalled},
        {"deployment_unit_added", odl_deployment_unit_added},
        {"deployment_unit_changed", odl_deployment_unit_changed},
        {"deployment_unit_uninstalled", odl_deployment_unit_uninstalled}
    };
    const std::vector<sRPCFunctions> functions_list = {
        {"odl_function_exec_env_add", "SoftwareModules.addExecEnv", odl_function_exec_env_add}
    };

    auto add_callbacks = [&retval](const auto& list, auto adder, auto& parser) {
        for (const auto& item : list) {
            retval = adder(parser.get(), item.name.c_str(), reinterpret_cast<amxo_fn_ptr_t>(item.callback));
            if (retval != 0) {
                std::cerr << "Error: Unable to add callback for '" << item.name << "'. Function returned: " << retval << "\n";
                return;
            }
            std::cout << "Success: Callback for '" << item.name << "' has been successfully added to the function table.\n";
        }
    };

    // Add callbacks
    add_callbacks(actions_list, amxo_resolver_ftab_add, parser);
    add_callbacks(events_list, amxo_resolver_ftab_add, parser);
    add_callbacks(functions_list, amxo_resolver_ftab_add, parser);

    // for (const auto &action : actions_list) {
    //     auto ret = amxo_resolver_ftab_add(parser.get(), action.name.c_str(),
    //                                       reinterpret_cast<amxo_fn_ptr_t>(action.callback));
    //     if (ret != 0) {
    //         std::cout << "Failed to add " << action.name;
    //         continue;
    //     }
    //     std::cout << "Added " << action.name << " to the functions table.";
    // }
    // for (const auto &event : events_list) {
    //     auto ret = amxo_resolver_ftab_add(parser.get(), event.name.c_str(),
    //                                       reinterpret_cast<amxo_fn_ptr_t>(event.callback));
    //     if (ret != 0) {
    //         std::cout << "Failed to add " << event.name;
    //         continue;
    //     }
    //     std::cout << "Added " << event.name << " to the functions table.";
    // }
    // for (const auto &func : functions_list) {
    //     auto ret = amxo_resolver_ftab_add(parser.get(), func.name.c_str(), AMXO_FUNC(func.callback));
    //     if (ret != 0) {
    //         std::cout << "Failed to add " << func.name;
    //         continue;
    //     }
    //     std::cout << "Added " << func.name << " to the functions table.";
    // }

    // Load ODL files
    retval = amxrt_load_odl_files(argc, argv, index);
    if (retval != 0) {
        std::cerr << "Loading ODL files failed\n";
        return retval;
    }

    // Add entry point to the parser
    amxo_parser_add_entry_point(parser.get(), amxrt_dm_save_load_main);

    // Connect
    retval = amxrt_connect();
    if (retval != 0) {
        std::cerr << "Connection failed\n";
        return retval;
    }

    // Enable system signals if available
    syssigs.reset(GET_ARG(config.get(), "system-signals"));
    if (syssigs) {
        amxrt_enable_syssigs(syssigs.get());
    }

    // Create event loop
    retval = amxrt_el_create();
    if (retval != 0) {
        std::cerr << "Event loop creation failed\n";
        return retval;
    }

    // Register or wait
    retval = amxrt_register_or_wait();
    if (retval != 0) {
        std::cerr << "Register or wait failed\n";
        return retval;
    }
    //amxo_parser_parse_string(parser.get(), "?include '${odl.directory}/${name}.odl':'${odl.dm-defaults}';", amxd_dm_get_root(dm.get()));
    // Start event loop
    amxrt_el_start();

    return retval;
}
