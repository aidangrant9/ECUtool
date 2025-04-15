function entry(args)
    local tried = {}
    local succeeded = {}
    local failed = {}
    
    log:info("Testing KWP2000 Services, please wait...\n")
    
    for k,v in pairs(ServiceIds) do
        if v == 0x81 or v == 0x82 or v == 0x10 or v == 0x80 or v == 0x20 or v == 0x11 then
            goto continue
        end

        connection:sleep(5)
        
        table.insert(tried, {id = v, name = k})
        
        local request, response = ApplicationLayer.requestSingleResponse({v})
        
        if response then
            local decoded = ApplicationLayer.decodeResponse(request, response)
            if decoded.responseType == "positive" or decoded.responseType == "unknown" then
                table.insert(succeeded, {id = v, name = k})
            elseif decoded.responseType == "negative" then
                if decoded.payload[2] ~= 0x11 then
                    table.insert(succeeded, {id = v, name = k})
                else
                    table.insert(failed, {id = v, name = k, reason = "Negative response"})
                end
            end
        else
            table.insert(failed, {id = v, name = k, reason = "No response"})
        end
        
        ::continue::
    end
    
    local msg = "--KWP2000 Service Check Results--\n"
    msg = msg .. string.format("%d/%d services supported\n\n", #succeeded, #tried)
    
    msg = msg .. "Services supported:\n"
    for _, v in ipairs(succeeded) do
        msg = msg .. string.format("0x%02X %s\n", v.id, v.name)
    end
    
    msg = msg .. "\nServices not supported:\n"
    for _, v in ipairs(failed) do
        msg = msg .. string.format("0x%02X %s (%s)\n", v.id, v.name, v.reason)
    end
    
    log:info(msg)
end