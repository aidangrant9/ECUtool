function entry()
    local success, msg = Services.stopDiagnosticSession()

    if msg then
        log:info(msg)
    end
end
