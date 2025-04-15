function entry()
    local kb1, kb2 = Datalink.initialise()

    if not kb1 then
        return
    end

    local messages = Datalink.accumulateResponses()

    if #messages == 0 then
        return
    end

    for i = 1, #messages do
        local no, type, data, msg = Datalink.decodeMessage(messages[i])
        log:info(msg)
    end
end