function entry(args)
    local seclevel = 0x01

    local seed, msg = Services.requestSeed(seclevel)

    if seed then
        local key = securityAlgorithm(seed)
        local granted, msg2 = Services.sendKey(seclevel+1, key, 2)
        if granted then
            log:info(msg .. "\n\n" .. msg2 .. "\n" .. "Access Granted")
            return
        end
        log:info(msg .. "\n\n" .. msg2)
        return
    end

    if msg then
        log:info(msg)
        return
    end
end

function securityAlgorithm(seed)
    -- Removed due to IP
end
