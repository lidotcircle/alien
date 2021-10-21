local _M = {}

function _M.TableMergeInto(dest, src)
    for k, v in pairs(src) do
        dest[k] = v
    end
    return dest
end

return _M