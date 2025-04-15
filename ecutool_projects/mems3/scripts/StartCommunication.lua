function entry(mode)
    local fastinit = true
    if mode == "fivebaud" then
        fastinit = false
    end

    local success, msg = Services.startCommunication(fastinit)
    if msg then
        log:info(msg)
    end
end
