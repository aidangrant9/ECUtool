Datalink = {}

-- Constructs a valid data link message based on the key byte
-- Returns nil if not possible (payload too big)
function Datalink.constructFrame(payload, source, target, functionalAddressing, keyByte)
    if not keyByte then
        keyByte = Datalink.getKeyByte()
    end

    local al0 = false -- Length in format byte supported
    local al1 = false -- Additional length byte supported
    local hb0 = false -- 1 byte header supported
    local hb1 = false -- Target/Source address in header supported

    if (keyByte & 0x1) > 0 then
        al0 = true
    end
    if (keyByte & 0x2) > 0 then
        al1 = true
    end
    if (keyByte & 0x4) > 0 then
        hb0 = true
    end
    if (keyByte & 0x8) > 0 then
        hb1 = true
    end

    if (#payload > 63) and (not al1) then
        return nil, "PAYLOAD_TOO_BIG_FOR_HEADER"
    end  

    if (#payload > 255) then
        return nil, "PAYLOAD_EXCEEDS_KWP2000_LIMIT"
    end

    -- ?
    if (not al1) and (not al0) then
         return nil
    end

    local message = {}
    local fmt = 0

    -- Include addressing information
    if hb1 then
        fmt = fmt | 0x80
        if functionalAddressing then
            fmt = fmt | 0x40
        end
    end

    -- Include length information
    if (al0) then
        fmt = fmt | #payload
    end

    -- add priority (?)
    if bmw_mode then
        fmt = fmt | 0x38
    end

    message[1] = fmt

    -- Include addressing info
    if hb1 then
        message[#message + 1] = target
        message[#message + 1] = source 
    end

    -- Include length byte
    if (not al0) then
        message[#message + 1] = #payload
    end

    -- Add payload
    for i = 1, #payload do
        message[#message + 1] = payload[i]
    end

    -- Add checksum
    message[#message + 1] = Datalink.calcChecksum(message)

    return message
end

--[[
-- Mod256 checksum
function Datalink.calcChecksum(data)
    local ret = 0
    for i = 1, #data do
        ret = ret + data[i]
    end
    return ret & 0xFF
end
]]--

function Datalink.calcChecksum(data)
    local ret = 0
    for i = 1, #data do
        if bmw_mode then
            ret = ret ~ data[i]
        else
            ret = ret + data[i]
        end
    end
    return ret & 0xFF
end

-- Parses frame into table or returns nil (checksum is validated)
function Datalink.parseFrame(data)
    if #data == 0 then
        return nil, "EMPTY_FRAME"
    end

    local ret = {}

    local tmp = data[1]

    -- clear "priority"
    if bmw_mode then
        data[1] = data[1] & 0xC0
    end

    local lengthIncluded = (data[1] & 0x3F) == 0 
    local addressIncluded = (data[1] & 0x80) > 0
    local functionalAddressing = (data[1] & 0x40) > 0

    if bmw_mode then
        data[1] = tmp
    end

    local headerLen = 1

    if lengthIncluded then
        headerLen = headerLen + 1
    end

    if addressIncluded then
        headerLen = headerLen + 2
    end

    local frameLen = headerLen + 1

    if not lengthIncluded then
        frameLen = frameLen + (data[1] & 0x3F)
    else
        if #data < headerLen then
            return nil, "FRAME_TOO_SHORT"
        end
        frameLen = frameLen + data[headerLen]
    end

    if #data < frameLen then
        return nil, "FRAME_TOO_SHORT"
    end

    local payloadLen = frameLen - 1 - headerLen

    if addressIncluded then
        ret.target = data[2]
        ret.source = data[3]
    else
        ret.target = nil
        ret.source = nil
    end

    local header = {}
    for i = 1, headerLen do
        header[i] = data[i]
    end

    local payload = {}
    for i = 1, payloadLen do
        payload[i] = data[i + headerLen]
    end

    local frame = {}
    for i = 1, frameLen do
        frame[i] = data[i]
    end

    ret.lengthIncluded = lengthIncluded
    ret.addressIncluded = addressIncluded
    ret.functionalAddressing = functionalAddressing
    ret.header = header
    ret.payload = payload
    ret.frame = frame
    ret.checksum = data[frameLen]

    local excludingChecksum = {}

    for i = 1, #frame-1 do
        excludingChecksum[#excludingChecksum + 1] = frame[i]
    end

    if Datalink.calcChecksum(excludingChecksum) ~= ret.checksum and strictChecksum then
        return nil, "CHECKSUM_MISMATCH"
    end

    return ret
end

-- Returns length of frame it decoded
function Datalink.frameMatch(data)
    local parsed = Datalink.parseFrame(data)
    
    if parsed then
        return #parsed.frame
    end

    return 0
end

function Datalink.getKeyByte()
    local global = tonumber(connection:getGlobalState("keyByte"), 16)
    if global and global < 0x100 then
        return global
    end

    if keyByte then
        return keyByte
    end

    -- default keyByte
    return 0xE9
end