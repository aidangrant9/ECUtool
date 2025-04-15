function entry(args)
    local block = 0x51
    local logBuffer = ""
    
    if args == "2" then
        block = 0x52
        logBuffer = logBuffer .. "Reading Configuration Block 2\n"
    else
        logBuffer = logBuffer .. "Reading Configuration Block 1\n"
    end
    
    local data, msg = Services.readDataByLocalIdentifier(block)
    
    if data then
        logBuffer = logBuffer .. (msg or "") .. "\n"
        
        if block == 0x51 then
            logBuffer = logBuffer .. interpretConfigBlock1(data)
        else
            logBuffer = logBuffer .. interpretConfigBlock2(data)
        end
        
        log:info(logBuffer)
        return true
    else
        log:error(defaultCol .. logBuffer .. (msg or ""))
        return false
    end
end

function interpretConfigBlock1(data)
    local result = "Configuration Block 1 Interpretation:\n"
    
    if #data >= 6 then
        local autoBeforeInch = (data[3] == 0x88) and "Yes" or "No"
        local seatbeltWarningLamp = (data[4] == 0xE8) and "Yes" or "No"
        local volumetricSensor = (data[5] == 0x70) and "Yes" or "No"
        local hazardFlashOnInertia = (data[6] == 0x19) and "Yes" or "No"
        
        result = result .. "Auto Before Inch: " .. autoBeforeInch .. "\n"
        result = result .. "Seatbelt Warning Lamp: " .. seatbeltWarningLamp .. "\n"
        result = result .. "Volumetric Sensor: " .. volumetricSensor .. "\n"
        result = result .. "Hazard Flash on Inertia: " .. hazardFlashOnInertia .. "\n"
    else
        result = result .. highlightCol .. "Insufficient data to interpret Block 1" .. defaultCol .. "\n"
    end
    
    return result
end

function interpretConfigBlock2(data)
    local result = "Configuration Block 2 Interpretation:\n"
    
    if #data >= 15 then
        local microwaveGainLevel = data[15]
        result = result .. "Microwave Gain Level: " .. string.format("0x%02X", microwaveGainLevel) .. "\n"
    else
        result = result .. highlightCol .. "Insufficient data to interpret Block 2" .. defaultCol .. "\n"
    end
    
    return result
end