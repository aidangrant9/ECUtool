Datalink = {}

-- Time to wait for the sync and key bytes during init 
SYNC_INFO_TIMEOUT = 600

-- Time to wait before sending inverted key byte
KB2_INVERSION_DELAY = 100

-- Communication Params
CLIENT_SEND_AFTER_RECV = 1 -- Delay to wait before sending new byte after receiving one
INBETWEEN_BYTES_MAX = 55 -- Maximum we or the ECU wait for next byte in message
ECHO_TIMEOUT = 55
NA_RETRIES = 5 -- Number of no ack retries before err
TO_RETRIES = 3 -- Nuber of timeout retries while sending a message before discarding
INBETWEEN_BYTES_MIN = 1 -- Min time to wait between messages
INBETWEEN_MESSAGES_MAX = 1100 -- Max time inbetween messages
INBETWEEN_MESSAGES_MIN = 1

-- Performs KWP1281 Bosch proprietary 5-baud initialisation
-- Returns KB1, KB2 if successful
function Datalink.initialise()
    connection:sendFiveBaudAddress(connection:targetAddress())
    connection:write({}) -- Clear line

    local keyByteMatch = function(data)
        if data[1] == 0x55 and #data > 2 then
            return 3
        end
        return 0
    end

    -- Get sync and key bytes
    local keyBytes = connection:readFrameMatch(SYNC_INFO_TIMEOUT, keyByteMatch)

    if #keyBytes > 0 then
        log:info(string.format("Sync: %02X\nKeyword: %02X%02X", keyBytes[1], keyBytes[2], keyBytes[3]))
        connection:sleep(KB2_INVERSION_DELAY)
        connection:write({~keyBytes[3] & 0xFF})
        log:info(string.format("!KB2: %02X\nInitialised", ~keyBytes[3] & 0xFF))
        return keyBytes[2], keyBytes[3]
    else
        log:error("Didn't receive sync byte")
        return
    end
end


function Datalink.singleByteMatch(data)
    if #data > 0 then
        return 1
    else
        return 0
    end
end


function Datalink.receiveMessage()
    local timeouts_retried = 0
    local first_recv = true
    local message

    ::begin_decode::
    if timeouts_retried > TO_RETRIES then
        log:error("Maximum number of retries attempted when receiving")
        return
    end

    local len

    if first_recv then
        len = connection:readFrameMatch(INBETWEEN_MESSAGES_MAX, Datalink.singleByteMatch)
    else
        len = connection:readFrameMatch(INBETWEEN_BYTES_MAX, Datalink.singleByteMatch)
    end

    if #len == 0 then
        first_recv = false
        goto timeout -- No read
    else
        if len[1] == 0 then
            goto begin_decode -- Retry decode with no timing penalty
        end
    end

    first_recv = false

    connection:sleep(INBETWEEN_BYTES_MIN)
    connection:write({~len[1]})
    message = {len[1]}

    for i = 1, len[1] do
        if i == len[1] then
            -- EXT byte
            local byte = connection:readFrameMatch(INBETWEEN_BYTES_MAX, Datalink.singleByteMatch)
            if #byte == 0 or byte[1] ~= 0x03 then
                log:error("No EXT when decoding message, unexpected length")
                return
            else
                table.insert(message, byte[1])
                return message
            end
        end

        local byte = connection:readFrameMatch(INBETWEEN_BYTES_MAX, Datalink.singleByteMatch)

        if #byte == 0 then
            goto timeout -- No read
        end

        table.insert(message, byte[1])
        connection:sleep(INBETWEEN_BYTES_MIN)
        connection:write({~byte[1]}) -- Send inv
    end
    
    ::timeout::
    connection:sleep(INBETWEEN_BYTES_MAX)
    timeouts_retried = timeouts_retried + 1
    goto begin_decode
end


function Datalink.sendMessage(messageNo, payload)
    local timeouts_retried = 0
    local len = 2 + #payload
    local tosend = {len, messageNo}
    for i = 1, #payload do
        tosend[#tosend+1] = payload[i]
    end

    ::begin_transmission::
    if timeouts_retried > TO_RETRIES then
        log:error("Maximum number of retries attempted when sending")
        return false
    end

    for i = 1, #tosend do
        connection:write({tosend[i]})
        local rep = connection:readFrameMatch(INBETWEEN_BYTES_MAX, Datalink.singleByteMatch)

        if #rep == 0 then
            log:error("Timeout waiting for response, retrying...")
            goto retry
        elseif rep[1] ~=  ~tosend[i] & 0xFF then
            log:error(string.format("Invalid invert, expected %02X got %02X", ~tosend[i] & 0xFF, rep[1]))
            connection:sleep(INBETWEEN_BYTES_MAX)
            goto retry
        end
        connection:sleep(INBETWEEN_BYTES_MIN)
    end

    if true then
        connection:write({0x03}) -- end of msg
        return true
    end

    ::retry::
    timeouts_retried = timeouts_retried + 1
    connection:sleep(INBETWEEN_BYTES_MAX)
    goto begin_transmission
end


function Datalink.accumulateResponses()
    local messages = {}

    ::decode_another::
    local incoming = Datalink.receiveMessage()

    if not incoming then
        log:error("Failed to receive message from ECU")
        return messages
    end

    -- Get message
    table.insert(messages, incoming)
    log:info("Received message: " .. dataToHexString(incoming))

    local messageNo = incoming[2]
    setCounter(messageNo+1)

    if incoming[3] == 0x09 or incoming[3] == 0x0A then -- ACK or NOACK_UNKNOWN
        return messages --done receiving
    end

    connection:sleep(INBETWEEN_MESSAGES_MIN)

    local sent = Datalink.sendMessage(getCounter(), {0x09}) -- Send ack

    if not sent then
        log:info("Failed to send acknowledgement")
        return messages
    end

    goto decode_another
end


-- returns messageno, type, data, string
function Datalink.decodeMessage(frame)
    if frame[1] ~= #frame-1 then
        return nil, nil, nil, "DECODE: Frame length not valid\n" .. dataToHexString(frame)
    end

    local data = {}

    for i = 4, #frame-1 do
        data[#data+1] = frame[i]
    end

    return frame[2], frame[3], data, Datalink.messageToString(frame[3], data)
end

function Datalink.messageToString(type, data)
    local typeStr = "Unknown"
    local repResentation = ""

    if type == Datalink.blockTitles.CLEAR_ERRORS then
        typeStr = "CLEAR ERRORS"
    elseif type == Datalink.blockTitles.END_COMMUNICATION then
        typeStr = "END COMMUNICATON"
    elseif type == Datalink.blockTitles.GET_ERRORS then
        typeStr = "GET ERRORS"
    elseif type == Datalink.blockTitles.ACKNOWLEDGE then
        typeStr = "ACK"
    elseif type == Datalink.blockTitles.GROUP_READING then
        typeStr = "GROUP READING"
    elseif type == Datalink.blockTitles.GROUP_READING_RESPONSE then
        typeStr = "GROUP READING RESPONSE"
    elseif type == Datalink.blockTitles.ASCII then
        typeStr = "ASCII"
        for i = 1, #data do
            repResentation = repResentation .. string.char(data[i])
        end
    elseif type == Datalink.blockTitles.NO_ACK_UNKNOWN then
        typeStr = "UNKNOWN SERVICE"
    elseif type == Datalink.blockTitles.GET_ERRORS_RESPONSE then
        typeStr = "GET ERRORS RESPONSE"
    end

    if repResentation == "" then
        return typeStr .. "\n" .. dataToHexString(data)

    else 
        return typeStr .. "\n" .. repResentation
    end
end


Datalink.blockTitles = {
    CLEAR_ERRORS = 0x05,
    END_COMMUNICATION = 0x06,
    GET_ERRORS = 0x07,
    ACKNOWLEDGE = 0x09,
    GROUP_READING = 0x29,
    GROUP_READING_RESPONSE = 0xE9,
    ASCII = 0xF6,
    GET_ERRORS_RESPONSE = 0xFC,
    NO_ACK_UNKNOWN = 0x0A
}