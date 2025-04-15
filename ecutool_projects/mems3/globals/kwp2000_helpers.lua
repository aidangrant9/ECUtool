Helpers = {}

function Helpers.parsedFrameToHexString(parsed)
    local responseString = ""
    for i = 1, #parsed.header do
        responseString = responseString .. string.format("%02X ",parsed.header[i])
    end

    responseString = responseString .. highlightCol

    for i = 1, #parsed.payload do
        responseString = responseString .. string.format("%02X ",parsed.payload[i])
    end

    responseString = responseString .. defaultCol
    responseString = responseString .. string.format("%02X",parsed.checksum)

    return responseString
end

function Helpers.dataToHexString(msg)
    local hexString = ""
    for i = 1, #msg-1 do
        hexString = hexString .. string.format("%02X ", msg[i])
    end
    hexString = hexString .. string.format("%02X", msg[#msg])
    return hexString
end

function dump(o)
    if type(o) == 'table' then
       local s = '{ '
       for k,v in pairs(o) do
          if type(k) ~= 'number' then k = '"'..k..'"' end
          s = s .. '['..k..'] = ' .. dump(v) .. ','
       end
       return s .. '} '
    else
       return tostring(o)
    end
 end