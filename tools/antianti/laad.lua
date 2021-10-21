local ntdll = require("alien.ntdll")
local alien = require("alien")

local tag = alien.tag("alien_objecthandleinfo")
ntdll.raw.NtClose:hook(function (handle)
    local buf = alien.wrap("alien_objecthandleinfo", 0, 0)
    local status = ntdll.NtQueryObject(handle, 4, buf, 8)
    local a, b = alien.unwrap("alien_objecthandleinfo", buf)

    if (status >= 0) then
        return ntdll.raw.NtClose:horigin()(handle)
    else
        return -1073741816
    end
end)
