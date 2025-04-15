ApplicationLayer = {}

function ApplicationLayer.sendRequest(payload, keyByte)
    local request, error = Datalink.constructFrame(
        payload, 
        connection:sourceAddress(), 
        connection:targetAddress(), 
        connection:functionalAddressing(),
        keyByte
    )

    if not request then
        log:error("Failed to construct frame " .. error)
        return nil, error
    end

    connection:writeWithDelay(request, innerByteTiming)
    return Datalink.parseFrame(request)
end

function ApplicationLayer.receiveResponse()
    local response = connection:readFrameMatch(timeout, Datalink.frameMatch)
    
    if not response or #response == 0 then 
        log:error("Failed to receive response from ECU")
        return nil, "NO_RESPONSE"
    end
    
    local parsed, error = Datalink.parseFrame(response)
    
    if not parsed then
        log:error("Failed to parse response " .. error)
        return nil, "PARSE_FAILED"
    end

    if parsed.addressIncluded then
        if not (parsed.source == connection:targetAddress()) or
        not (parsed.target == connection:sourceAddress()) then
            log:error("Address mismatch in response from ECU")
            return nil, "ADDRESS_MISMATCH"
        end
    end

    return parsed
end

-- Request and recieve single response from the server (either negative or positive)
function ApplicationLayer.requestSingleResponse(payload, keyByte)
    local request = ApplicationLayer.sendRequest(payload, keyByte)
    local response = ApplicationLayer.receiveResponse()

    while response do
        local decoded, resString, reqString = ApplicationLayer.decodeResponse(request, response)

        if decoded.responseType == "positive" or decoded.responseType == "unknown" then
            return request, response
        end

        if decoded.responseType == "negative" then
            if decoded.responseCode == 0x23 or decoded.responseCode == 0x21 then
                log:info(reqString .. "\n" .. resString .. "\nWaiting for ECU to finish...")
                connection:sleep(math.floor(p2+p3))
                ApplicationLayer.sendRequest(payload)

            elseif decoded.responseCode == 0x78 then
                log:info(reqString .. "\n" .. resString .. "\nWaiting for ECU to finish...")
            else
                return request, response
            end
        end

        response = ApplicationLayer.receiveResponse()
    end

    return nil
end

-- Returns the decoding result and string representation
function ApplicationLayer.decodeResponse(request, response)

    local responseString = Helpers.parsedFrameToHexString(response)
    local requestString = Helpers.parsedFrameToHexString(request)

    local responseTypeString = ""
    local responseCodeString = nil

    local retDecode = { responseType = "unknown", responseCode = nil, payload={} }

    if response.payload[1] == (request.payload[1] | 0x40) then
        retDecode.responseType = "positive"
        for i = 2, #response.payload do
            retDecode.payload[#retDecode.payload+1] = response.payload[i]
        end
        responseTypeString = successCol .. "PositiveResponse" .. defaultCol

    elseif response.payload[1] == 0x7F then
        retDecode.responseType = "negative"
        responseTypeString = errorCol .. "NegativeResponse" .. defaultCol

        if #response.payload == 3 and response.payload[2] == request.payload[1] then
            retDecode.responseCode = response.payload[3]
            responseCodeString = ApplicationLayer.getResponseCodeString(response.payload[3])
        end
        for i = 2, #response.payload do
            retDecode.payload[#retDecode.payload+1] = response.payload[i]
        end
    else
        retDecode.responseType = "unknown"
        retDecode.payload = response.payload
    end 

    local reqString = "[ " .. requestString .. " ] <Request>"
    local retString = "[ " .. responseString .. " ] <" .. responseTypeString .. ">"

    if responseCodeString then
        retString = retString .. " <" .. responseCodeString .. ">"
    end

    return retDecode, retString, reqString
end

function ApplicationLayer.getResponseCodeString(code)
    local string = ResponseCodes[code]

    if string then
        return string
    end

    if code >= 0x80 then
        return "ManufacturerDefined"
    end

    return "UnknownResponseCode"
end

ResponseCodes = {
    [0x10] = "GeneralReject",
    [0x11] = "ServiceNotSupported",
    [0x12] = "SubFunctionNotSupported-InvalidFormat",
    [0x21] = "Busy-RepeatRequest",
    [0x22] = "ConditionsNotCorrect or RequestSequenceError",
    [0x23] = "RoutineNotComplete",
    [0x31] = "RequestOutOfRange",
    [0x33] = "SecurityAccessDenied",
    [0x35] = "InvalidKey",
    [0x36] = "ExceedNumberOfAttempts",
    [0x37] = "RequiredTimeDelayNotExpired",
    [0x40] = "DownloadNotAccepted",
    [0x41] = "ImproperDownloadType",
    [0x42] = "CantDownloadToSpecifiedAddress",
    [0x43] = "CantDownloadNumberOfBytesRequested",
    [0x50] = "UploadNotAccepted",
    [0x51] = "ImproperUploadType",
    [0x52] = "CantUploadFromSpecifiedAddress",
    [0x53] = "CantUploadNumberOfBytesRequested",
    [0x71] = "TransferSuspended",
    [0x72] = "TransferAborted",
    [0x74] = "IllegalAddressInBlockTransfer",
    [0x75] = "IllegalByteCountInBlockTransfer",
    [0x76] = "IllegalBlockTransferType",
    [0x77] = "BlockTransferDataChecksumError",
    [0x78] = "ReqCorrectlyReceived-PendingResponse",
    [0x79] = "IncorrectByteCountDuringBlockTransfer",

    -- Add manufacturer response codes here
}
