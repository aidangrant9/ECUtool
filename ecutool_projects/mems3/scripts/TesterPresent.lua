function entry()
    local succ, msg = Services.testerPresent(true)

    if succ then
        log:info(msg)
    else
        log:error(defaultCol .. msg .. errorCol .. "\nNo response from TesterPresent: Session dead")
        return false
    end
end
