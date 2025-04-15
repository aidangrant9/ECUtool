



function setCounter(n)
    connection:setGlobalState("counter", string.format("%02X", n & 0xFF))
end

function getCounter() 
    local counter = tonumber(connection:getGlobalState("counter"), 16)
    if counter then
        return counter
    else
        return 1
    end
end

function dataToHexString(msg)
    local hexString = ""
    if not msg or #msg == 0 then
        return ""
    end
    for i = 1, #msg-1 do
        hexString = hexString .. string.format("%02X ", msg[i])
    end
    hexString = hexString .. string.format("%02X", msg[#msg])
    return hexString
end

