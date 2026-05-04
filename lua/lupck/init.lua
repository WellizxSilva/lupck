---@class LupckConfig
---@field entry string The main script to execute (e.g., "main.lua")
---@field version? string The version of your application
---@field author? string The author's name
---@field description? string A brief description of the app
-- @field assets? string[] List of additional folders to include


local lupck  = {
    _VERSION = "0.1.0"
}
local config = {
    ---Defines and validates the configuration for the Lupck package.
    ---@param cfg LupckConfig
    ---@return LupckConfig
    define = function(cfg)
        if type(cfg) ~= "table" then
            error("[Lupck] define_config: expected a table, got " .. type(cfg), 2)
        end
        if not cfg.entry or type(cfg.entry) ~= "string" then
            error("[Lupck] config error: 'entry' field is required and must be a string", 2)
        end
        return cfg
    end
}
lupck.config = config


-- ---Check if the code is currently running inside an embedded package.
-- ---@return boolean
-- function lupck.is_embedded()
--     -- This global will be injected by the C++ Loader
--     return _G.__LUPCK_EMBEDDED__ == true
-- end

return lupck
