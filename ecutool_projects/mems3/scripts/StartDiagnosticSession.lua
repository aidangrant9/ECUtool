function entry(args)
    local mode = 0xA0

    if args == "programming" then
        mode = {0x85, 0x01, 0x00, 0x25, 0x80, 0x6B}
    end

    local succ, msg = Services.startDiagnosticSession(mode)

    if msg then
        log:info(msg)
    end
end
