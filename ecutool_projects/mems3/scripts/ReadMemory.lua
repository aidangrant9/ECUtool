function entry(args)
    local first, second = args:match("(%S+)%s+(%S+)")
    local address = tonumber(first, 16)
    local size = tonumber(second, 16)

    local data, msgs = Services.readDataByMemoryAddress(address, size)

    if data then
        log:info(Helpers.dataToHexString(data))
    elseif msgs then
        log:info(msgs)
    end

end
