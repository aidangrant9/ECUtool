function entry(args)
    local mode = tonumber(args, 16)

    if not mode then
        mode = 0x1
    end

    local succ, msg = Services.ecuReset(mode)

    if msg then
        log:info(msg)
    end
end
