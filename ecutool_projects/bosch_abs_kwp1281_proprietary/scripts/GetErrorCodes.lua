function entry()
    local succ = Datalink.sendMessage(getCounter(), {Datalink.blockTitles.GET_ERRORS})

    if not succ then
        log:error("Failed to send GetErrors")
        return false
    end

    local messages = Datalink.accumulateResponses()

    if #messages == 0 then
        log:error("GetErrors failed")
        return false
    end

    for i = 1, #messages do
        local no, type, data, msg = Datalink.decodeMessage(messages[i])
        log:info(msg)
    end
end