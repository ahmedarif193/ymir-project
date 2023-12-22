#!/usr/bin/lua

local json = require 'cjson'

local function set_exec_env_name(instance_number, container_name)
    local command = string.format('ubus-cli "SoftwareModules.ExecEnv.%d.Name=%s"', instance_number, container_name)
    print("Setting ExecEnv name:", command)
    os.execute(command)
end

local function get_execenv_instances()
    local start_time = os.time()
    local handle = io.popen('ubus-cli "SoftwareModules.ExecEnv.?" -j')
    local json_line

    for line in handle:lines() do
        if os.difftime(os.time(), start_time) > 2 then
            print("Command took too long, continuing loop")
            break
        end

        if line:match("^%s*{%s*") or line:match("^%s*%[%s*") then
            json_line = line
            break
        end
    end

    handle:close()
    return json_line
end

local function file_exists(file_path)
    local file = io.open(file_path, "r")
    if file then
        file:close()
        return true
    else
        return false
    end
end

local function start_container(container_name)
    print("Starting container:", container_name)
    os.execute('lxc-start "' .. container_name .. '"')
end

local function stop_container(container_name)
    print("Stopping container:", container_name)
    os.execute('lxc-stop "' .. container_name .. '"')
end

local function reset_container_enable(exec_env_key)
    print("Resetting Enable state for:", exec_env_key)
    local disable_command = 'ubus-cli "' .. exec_env_key .. 'Enable=0"'
    local enable_command = 'ubus-cli "' .. exec_env_key .. 'Enable=1"'
    print("Executing:", disable_command)
    os.execute(disable_command)
    print("Executing:", enable_command)
    os.execute(enable_command)
end

local function is_container_running(container_name)
    print("is_container_running")
    local command = 'lxc-info -n "' .. container_name .. '" | grep -q "RUNNING"'
    local result = os.execute(command)
    return result == true or result == 0  -- os.execute returns true or 0 on success
end

local function extract_instance_number(exec_env_key)
    print("extract_instance_number")
    return exec_env_key:match("%d+")
end

local function check_and_set_name_if_missing(exec_env_key, value)
    print("check_and_set_name_if_missing")
    if not value.Name or value.Name == '' then
        local instance_number = extract_instance_number(exec_env_key)
        local new_name = "Container-" .. instance_number
        set_exec_env_name(instance_number, new_name)
        value.Name = new_name
    end
end

local function handle_execenv_instances(exec_env_key, value)
    -- Check and set the container name if it's missing
    check_and_set_name_if_missing(exec_env_key, value)
    local container_config_path = '/srv/lxc/' .. value.Name .. '/config'

    if value.Enable == 1 then
        local isRunning = is_container_running(value.Name)
        if not isRunning then
            reset_container_enable(exec_env_key)
        end
    elseif value.Enable == 0 then
        local isRunning = is_container_running(value.Name)
        if isRunning then
            stop_container(value.Name)
        end
    end
end

local function handle_lxc(exec_envs)
    print("tick tack")
    for _, exec_env in ipairs(exec_envs) do
        for key, value in pairs(exec_env) do
            handle_execenv_instances(key, value)
        end
    end
end

local function main_execution_loop()
    while true do
        local ubus_output = get_execenv_instances()
        if ubus_output then
            local exec_envs = json.decode(ubus_output)
            handle_lxc(exec_envs)
        else
            print("No JSON data found or command took too long")
        end
        os.execute("sleep 1")
    end
end

main_execution_loop()
