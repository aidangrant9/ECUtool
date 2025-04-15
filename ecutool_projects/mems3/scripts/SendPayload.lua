function entry(arguments)
    local toSend = hexStringToTable(arguments)

   local request, response = ApplicationLayer.requestSingleResponse(toSend)

   if response and request then
        local decoded, resString, reqString = ApplicationLayer.decodeResponse(request, response)
        log:info(reqString .. "\n" .. resString)
   end
end


function hexStringToTable(hexString)
    local result = {}
    hexString = hexString:gsub("[^0-9A-Fa-f]", "")
    
    for i = 1, #hexString, 2 do
        local hex = hexString:sub(i, i+1)
        if #hex == 2 then
            table.insert(result, tonumber(hex, 16))
        end
    end
    
    return result
end