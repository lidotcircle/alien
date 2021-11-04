local _M = require "alien_c"
local alien = _M

require("alien.def_load")

local alias = alien.alias
local atype = alien.atype

function _M.alias(new, old)
    if (type(old) == "string") then
        old = atype(old)
    end

    alias(new, old)
end

return _M
