ServiceIds = {
    START_COMMUNICATION = 0x81,
    STOP_COMMUNICATION = 0x82,

    START_DIAGNOSTIC_SESSION = 0x10,
    ECU_RESET = 0x11,
    READ_FREEZE_FRAME = 0x12,
    READ_DTC = 0x13,
    CLEAR_DIAGNOSTIC_INFORMATION = 0x14,
    READ_DTC_STATUS = 0x17,
    READ_DTC_BY_STATUS = 0x18,
    READ_ECU_ID = 0x1A,
    STOP_DIAGNOSTIC_SESSION = 0x20,
    READ_DATA_BY_LOCAL_ID = 0x21,
    READ_DATA_BY_COMMON_ID = 0x22,
    READ_MEMORY_BY_ADDRESS = 0x23,
    SET_DATA_RATES = 0x26,
    SECURITY_ACCESS = 0x27,
    DYNAMICALLY_DEFINE_LOCAL_ID = 0x2C,
    WRITE_DATA_BY_COMMON_ID = 0x2E,
    IO_CONTROL_BY_COMMON_ID = 0x2F,
    IO_CONTROL_BY_LOCAL_ID = 0x30,
    START_ROUTINE_BY_LOCAL_ID = 0x31,
    STOP_ROUTINE_BY_LOCAL_ID = 0x32,
    REQUEST_ROUTINE_RESULTS_BY_LOC_ID = 0x33,
    REQUEST_DOWNLOAD = 0x34,
    REQUEST_UPLOAD = 0x35,
    TRANSFER_DATA = 0x36,
    REQUEST_TRANSFER_EXIT = 0x37,
    START_ROUTINE_BY_ADDRESS = 0x38,
    STOP_ROUTINE_BY_ADDRESS = 0x39,
    REQUEST_ROUTINE_RESULT_BY_ADDRESS = 0x3A,
    WRITE_DATA_BY_LOCAL_ID = 0x3B,
    WRITE_MEMORY_BY_ADDRESS = 0x3D,
    TESTER_PRESENT = 0x3E,
    ESC_CODE = 0x80
}

-- Helper to construct payloads in nice syntax
function buildPayload(serviceId, ...)
    local payload = {serviceId}
    for _, param in ipairs({...}) do
        if type(param) == "table" then
            for _, value in ipairs(param) do
                table.insert(payload, value)
            end
        else
            table.insert(payload, param)
        end
    end
    return payload
end
    
-- For services with no side effects or data response
function handleStandardResponse(request, response)
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        return decoded.responseType == "positive", (reqString .. "\n" .. repString)
    end
    return false
end

Services = {}

-- Returns (success:bool, message:string)
function Services.startCommunication(fastInit)
    local retString = ""
    if fastInit == true then
        connection:wakeUpPattern()

        local request, response = ApplicationLayer.requestSingleResponse({ServiceIds.START_COMMUNICATION}, 0xE9)

        if not response then
            return false
        end

        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)

        if decoded.responseType == "positive" then
            retString = reqString .. "\n" .. repString .. string.format("\nKey bytes %02X %02X", decoded.payload[1], decoded.payload[2])
            connection:setGlobalState("keyByte", string.format("%02X", decoded.payload[1]))
            return true, retString
        end

        retString = reqString .. "\n" .. repString
        return false, retString
    end

    -- 5 Baud Init

    local fiveBaudFrameMatch = function(data)
        if #data < 3 then
            return 0
        end
        
        if data[1] == 0x55 then
            return 3
        end

        return 0
    end

    
    local invAddrFrameMatch = function(data)
        if data[1] == ~connection:targetAddress() & 0xFF then
            return 1
        end
        return 0
    end

    connection:write({})
    connection:sendFiveBaudAddress(connection:targetAddress())
    
    local response = connection:readFrameMatch(timeout, fiveBaudFrameMatch)

    if not response or #response == 0 then
        return false, "Recieved no sync byte"
    end


    retString = string.format("Key bytes %02X %02X", response[2], response[3])
    connection:setGlobalState("keyByte", string.format("%02X", response[2]))

    connection:sleep(25)
    connection:write({~response[3] & 0xFF})
    response = connection:readFrameMatch(timeout, invAddrFrameMatch)

    if #response == 0 then
        retString = retString .. "\nRecieved no inverted address"
        return false, retString
    end

    return true, retString .. "\nSuccessfully intitialised"
end


function Services.primitiveOperation(payload)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    return handleStandardResponse(request, response)
end

-- Returns (success:bool, message:string)
function Services.stopCommunication()
    return Services.primitiveOperation(buildPayload(ServiceIds.STOP_COMMUNICATION))
end

function Services.startDiagnosticSession(mode)
    local request, response = ApplicationLayer.requestSingleResponse(buildPayload(ServiceIds.START_DIAGNOSTIC_SESSION, mode))
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            if decoded.payload[1] == mode then
                return true, (reqString .. "\n" .. repString)
            else
                return false, (reqString .. "\n" .. repString .. "\nRequested mode does not match")
            end
        else
            return false, (reqString .. "\n" .. repString)
        end
    end
end

function Services.stopDiagnosticSession()
    return Services.primitiveOperation(buildPayload(ServiceIds.STOP_DIAGNOSTIC_SESSION))
end

-- Returns seed (opt), message (opt)
function Services.requestSeed(level)
    local request, response = ApplicationLayer.requestSingleResponse(buildPayload(ServiceIds.SECURITY_ACCESS, level))
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local seed = 0
            for i = 2, #decoded.payload do
                seed = (seed << 8) | decoded.payload[i]
            end
            return seed, reqString .. "\n" .. repString .. "\n" .. string.format("Got seed %0X", seed) 
        end
        return nil, reqString .. "\n" .. repString
    end
end

-- returns accessgranted:bool
function Services.sendKey(mode, key, keyLen)
    local payload = buildPayload(ServiceIds.SECURITY_ACCESS, mode)
    for i = 1, keyLen do
        table.insert(payload, (key >> (8 * (keyLen - i))) & 0xFF)
    end
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            if decoded.payload[1] == mode then
                return true, reqString .. "\n" .. repString
            end
        end
        return false, reqString .. "\n" .. repString
    end
end


function Services.testerPresent(responseRequired)
    if responseRequired == true then
        return Services.primitiveOperation(buildPayload(ServiceIds.TESTER_PRESENT, 0x01))
    else
        ApplicationLayer.sendRequest(buildPayload(ServiceIds.TESTER_PRESENT, 0x02))
        return true
    end
end


function Services.ecuReset(mode)
    return Services.primitiveOperation(buildPayload(ServiceIds.ECU_RESET, mode))
end

-- returns id or nill, message
function Services.readECUIdentification(idx)
    local request, response = ApplicationLayer.requestSingleResponse(buildPayload(ServiceIds.READ_ECU_ID, idx))
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local data = {}
            for i = 2, #decoded.payload do
                data[#data+1] = decoded.payload[i]
            end
            
            return true, data, (reqString .. "\n" .. repString)
        else 
            return false, nil, (reqString .. "\n" .. repString)
        end
    end
    return false, nil
end

-- Helper for following functions
function handleRecordReading(request, response, startIndex, maxresponse)
    local records = {}
    local msgs = nil
    local hasReq = false
    local hasData = false

    while response do
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if maxresponse then maxresponse = maxresponse - 1 end
        
        if not hasReq then
            if not msgs then
                msgs = ""
            end
            msgs = msgs .. reqString .. "\n"
            hasReq = true
        end
        msgs = msgs .. repString .. "\n"

        if decoded.responseType == "positive" then
            for i = startIndex, #decoded.payload do
                table.insert(records, decoded.payload[i])
                hasData = true
            end
        end

        if not maxresponse or maxresponse <= 0 or decoded.responseType ~= "positive" then
            if not hasData and decoded.responseType ~= "positive" then
                return nil, msgs
            end
            return records, msgs
        end

        response = ApplicationLayer.receiveResponse()
    end
    
    if not hasData then
        return nil, msgs
    end
    return records, msgs
end

-- Read data by local identifier with optional transmission mode
function Services.readDataByLocalIdentifier(identifier, transmode, maxresponse)
    local payload = buildPayload(ServiceIds.READ_DATA_BY_LOCAL_ID, identifier)
    
    if transmode then
        table.insert(payload, transmode)
        if maxresponse then
            table.insert(payload, maxresponse)
        end
    end
    
    local request, response = ApplicationLayer.requestSingleResponse(payload)

    if not response then
        return nil
    end

    return handleRecordReading(request, response, 2, maxresponse)
end


-- returns records or nil, message
function Services.readDataByCommonIdentifier(identifier, transmode, maxresponse)
    local payload = buildPayload(ServiceIds.READ_DATA_BY_COMMON_ID, 
                            (identifier >> 8) & 0xFF,
                            identifier & 0xFF)  

    if transmode then
        payload = buildPayload(payload, transmode, maxresponse)
    end

    local request, response = ApplicationLayer.requestSingleResponse(payload)

    if not response then
        return nil
    end

    return handleRecordReading(request, response, 3, maxresponse)
end

-- returns records or nil, message
function Services.readDataByMemoryAddress(address, size, transmode, maxresponse)
    local payload = buildPayload(ServiceIds.READ_MEMORY_BY_ADDRESS, 
                            (address >> 16) & 0xFF,
                            (address >> 8) & 0xFF,  
                            address & 0xFF, size)

    if transmode then
        payload = buildPayload(payload, transmode, maxresponse)
    end

    local request, response = ApplicationLayer.requestSingleResponse(payload)

    if not response then
        return nil
    end

    local records, msgs = handleRecordReading(request, response, 1, maxresponse)

    if records then
        for i, record in ipairs(records) do
            local recordLength = #record
            record[recordLength] = nil
            record[recordLength-1] = nil
            record[recordLength-2] = nil
        end
        return records, msgs


    else
        return nil, msgs
    end
end

local DEFINE_MODE = {
    BY_LOCAL_ID = 0x01,
    BY_COMMON_ID = 0x02,
    BY_MEMORY_ADDRESS = 0x03,
    CLEAR = 0x04
}

function Services.dynamicallyDefineLocalIdentifier(mode, identifier_to, position_to, size, identifier_from, position_from)
    if mode == DEFINE_MODE.BY_LOCAL_ID then
        return Services.dynamicallyDefineLocalIdentifierByLocalIdentifier(identifier_to, position_to, size, identifier_from, position_from)
    elseif mode == DEFINE_MODE.BY_COMMON_ID then
        return Services.dynamicallyDefineLocalIdentifierByCommonIdentifier(identifier_to, position_to, size, identifier_from, position_from)
    elseif mode == DEFINE_MODE.BY_MEMORY_ADDRESS then
        return Services.dynamicallyDefineLocalIdentifierByMemoryAddress(identifier_to, position_to, size, identifier_from, position_from)
    elseif mode == DEFINE_MODE.CLEAR then
        return Services.primitiveOperation(buildPayload(ServiceIds.DYNAMICALLY_DEFINE_LOCAL_ID, identifier_to, DEFINE_MODE.CLEAR))
    else
        return false, "Mode not implemented"
    end
end

function Services.dynamicallyDefineLocalIdentifierByLocalIdentifier(identifier, position_to, size, existing, position_from)
    local payload = buildPayload(ServiceIds.DYNAMICALLY_DEFINE_LOCAL_ID, identifier, 
                            DEFINE_MODE.BY_LOCAL_ID, position_to, size, existing, position_from)
    return Services.primitiveOperation(payload)
end

function Services.dynamicallyDefineLocalIdentifierByCommonIdentifier(identifier, position_to, size, existing, position_from)
    local payload = buildPayload(ServiceIds.DYNAMICALLY_DEFINE_LOCAL_ID, identifier, 
                            DEFINE_MODE.BY_COMMON_ID, position_to, size, 
                            (existing >> 8) & 0xFF, existing & 0xFF, position_from)
    return Services.primitiveOperation(payload)
end

function Services.dynamicallyDefineLocalIdentifierByMemoryAddress(identifier, position_to, size, existing, position_from)
    local payload = buildPayload(ServiceIds.DYNAMICALLY_DEFINE_LOCAL_ID, identifier, 
                            DEFINE_MODE.BY_MEMORY_ADDRESS, position_to, size, 
                            (existing >> 16) & 0xFF, (existing >> 8) & 0xFF, existing & 0xFF)
    return Services.primitiveOperation(payload)
end

function Services.writeDataByLocalIdentifier(identifier, records)
    local payload = buildPayload(ServiceIds.WRITE_DATA_BY_LOCAL_ID, identifier & 0xFF, records)
    return Services.primitiveOperation(payload)
end

function Services.writeDataByCommonIdentifier(identifier, records)
    local payload = buildPayload(ServiceIds.WRITE_DATA_BY_COMMON_ID, 
                            (identifier >> 8) & 0xFF, identifier & 0xFF, records)
    return Services.primitiveOperation(payload)
end

function Services.writeDataByMemoryAddress(address, records)
    local payload = buildPayload(ServiceIds.WRITE_MEMORY_BY_ADDRESS, 
                            (address >> 16) & 0xFF, 
                            (address >> 8) & 0xFF, 
                            address & 0xFF, records)
    return Services.primitiveOperation(payload)
end

function Services.setDataRates(slow, med, fast)
    return Services.primitiveOperation(buildPayload(ServiceIds.SET_DATA_RATES, slow, med, fast))
end

-- returns numCodes, Codes, msg
function Services.readDiagnosticTroubleCodes(DTCgroups)
    local payload = buildPayload(ServiceIds.READ_DTC, DTCgroups)
    local request, response = ApplicationLayer.requestSingleResponse(payload)

    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local nCodes = decoded.payload[1]
            local codes = {}
            for i = 2, #decoded.payload do
                table.insert(codes, decoded.payload[i])
            end
            
            if nCodes == 0 or #codes == 0 then
                return 0, nil, reqString .. "\n" .. repString
            end
            
            return nCodes, codes, reqString .. "\n" .. repString
        else
            return 0, nil, reqString .. "\n" .. repString
        end
    end
    return 0, nil
end

-- returns numDtc, dtcAndStatuses, msg
function Services.readDiagnosticTroubleCodesByStatus(DTCStatuses, DTCgroups)
    local payload = buildPayload(ServiceIds.READ_DTC_BY_STATUS)
    
    for _, status in ipairs(DTCStatuses) do
        table.insert(payload, status)
    end
    
    for _, group in ipairs(DTCgroups) do
        table.insert(payload, group)
    end

    local request, response = ApplicationLayer.requestSingleResponse(payload)

    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local nCodes = decoded.payload[1]
            local codes = {}
            for i = 2, #decoded.payload do
                table.insert(codes, decoded.payload[i])
            end
            return nCodes, codes, reqString .. "\n" .. repString
        else
            return 0, {}, reqString .. "\n" .. repString
        end
    end
    return 0, {}
end


function Services.readStatusOfDiagnosticTroubleCodes(DTCgroups)
    local payload = buildPayload(ServiceIds.READ_DTC_STATUS)
    
    for _, group in ipairs(DTCgroups) do
        table.insert(payload, group)
    end

    local request, response = ApplicationLayer.requestSingleResponse(payload)

    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local nCodes = decoded.payload[1]
            local codes = {}
            for i = 2, #decoded.payload do
                table.insert(codes, decoded.payload[i])
            end
            return nCodes, codes, reqString .. "\n" .. repString
        else
            return 0, {}, reqString .. "\n" .. repString
        end
    end
    return 0, {}
end

function Services.readFreezeFrameData(mode, frameNum, identifier)
    local payload = buildPayload(ServiceIds.READ_FREEZE_FRAME, frameNum, mode)
    
    if mode == 0x01 then
        table.insert(payload, identifier)
    elseif mode == 0x02 then
        table.insert(payload, (identifier >> 8) & 0xFF)
        table.insert(payload, identifier & 0xFF)
    elseif mode == 0x03 then
        table.insert(payload, (identifier >> 16) & 0xFF)
        table.insert(payload, (identifier >> 8) & 0xFF)
        table.insert(payload, identifier & 0xFF)
    elseif mode == 0x04 then
        if type(identifier) == "table" then
            for _, v in ipairs(identifier) do
                table.insert(payload, v)
            end
        else
            table.insert(payload, identifier)
        end
    end
    
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            return decoded.payload, reqString .. "\n" .. repString
        else
            return nil, reqString .. "\n" .. repString
        end
    end
    return nil
end

function Services.clearDiagnosticInformation(DTCgroups)
    local payload = buildPayload(ServiceIds.CLEAR_DIAGNOSTIC_INFORMATION, DTCgroups)
    return Services.primitiveOperation(payload)
end

function Services.inputOutputControlByLocalIdentifier(identifier, controlOptions)
    local payload = buildPayload(ServiceIds.IO_CONTROL_BY_LOCAL_ID, identifier, controlOptions)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local controlStatus = {}
            for i = 2, #decoded.payload do
                table.insert(controlStatus, decoded.payload[i])
            end
            
            return true, controlStatus, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.inputOutputControlByCommonIdentifier(identifier, controlOptions)
    local payload = buildPayload(ServiceIds.IO_CONTROL_BY_COMMON_ID, 
                            (identifier >> 8) & 0xFF, identifier & 0xFF, controlOptions)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local controlStatus = {}
            for i = 3, #decoded.payload do
                table.insert(controlStatus, decoded.payload[i])
            end

            return true, controlStatus, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.startRoutineByLocalIdentifier(identifier, routineEntryOptions)
    local payload = buildPayload(ServiceIds.START_ROUTINE_BY_LOCAL_ID, identifier, routineEntryOptions)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local status = {}
            for i = 2, #decoded.payload do
                table.insert(status, decoded.payload[i])
            end
            return true, status, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.stopRoutineByLocalIdentifier(identifier, routineExitOptions)
    local payload = buildPayload(ServiceIds.STOP_ROUTINE_BY_LOCAL_ID, identifier, routineExitOptions)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local status = {}
            for i = 2, #decoded.payload do
                table.insert(status, decoded.payload[i])
            end
            return true, status, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.requestRoutineResultsByLocalIdentifier(identifier)
    local payload = buildPayload(ServiceIds.REQUEST_ROUTINE_RESULTS_BY_LOC_ID, identifier)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local results = {}
            for i = 2, #decoded.payload do
                table.insert(results, decoded.payload[i])
            end
            return true, results, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.startRoutineByAddress(address, routineEntryOptions)
    local payload = buildPayload(ServiceIds.START_ROUTINE_BY_ADDRESS, 
                            (address >> 16) & 0xFF, 
                            (address >> 8) & 0xFF, 
                            address & 0xFF, 
                            routineEntryOptions)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local status = {}
            for i = 4, #decoded.payload do
                table.insert(status, decoded.payload[i])
            end
            return true, status, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.stopRoutineByAddress(address, routineExitOptions)
    local payload = buildPayload(ServiceIds.STOP_ROUTINE_BY_ADDRESS, 
                            (address >> 16) & 0xFF, 
                            (address >> 8) & 0xFF, 
                            address & 0xFF, 
                            routineExitOptions)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local status = {}
            for i = 4, #decoded.payload do
                table.insert(status, decoded.payload[i])
            end
            return true, status, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.requestRoutineResultsByAddress(address)
    local payload = buildPayload(ServiceIds.REQUEST_ROUTINE_RESULT_BY_ADDRESS, 
                            (address >> 16) & 0xFF, 
                            (address >> 8) & 0xFF, 
                            address & 0xFF)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local results = {}
            for i = 4, #decoded.payload do
                table.insert(results, decoded.payload[i])
            end
            return true, results, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.requestDownload(transferRequestParams)
    local payload = buildPayload(ServiceIds.REQUEST_DOWNLOAD, transferRequestParams)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local params = {}
            for i = 1, #decoded.payload do
                table.insert(params, decoded.payload[i])
            end
            return true, params, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.requestUpload(transferRequestParams)
    local payload = buildPayload(ServiceIds.REQUEST_UPLOAD, transferRequestParams)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local params = {}
            for i = 1, #decoded.payload do
                table.insert(params, decoded.payload[i])
            end
            return true, params, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.transferData(transferRequestParams)
    local payload = buildPayload(ServiceIds.TRANSFER_DATA, transferRequestParams)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local params = {}
            for i = 1, #decoded.payload do
                table.insert(params, decoded.payload[i])
            end
            return true, params, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.requestTransferExit(transferRequestParams)
    local payload = buildPayload(ServiceIds.REQUEST_TRANSFER_EXIT, transferRequestParams)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local params = {}
            for i = 1, #decoded.payload do
                table.insert(params, decoded.payload[i])
            end
            return true, params, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

function Services.escCode(manufacturerServiceId, recordValues)
    local payload = buildPayload(ServiceIds.ESC_CODE, manufacturerServiceId, recordValues)
    local request, response = ApplicationLayer.requestSingleResponse(payload)
    
    if response then
        local decoded, repString, reqString = ApplicationLayer.decodeResponse(request, response)
        if decoded.responseType == "positive" then
            local values = {}
            for i = 2, #decoded.payload do
                table.insert(values, decoded.payload[i])
            end
            return true, values, reqString .. "\n" .. repString
        else
            return false, nil, reqString .. "\n" .. repString
        end
    end
    return false, nil
end

