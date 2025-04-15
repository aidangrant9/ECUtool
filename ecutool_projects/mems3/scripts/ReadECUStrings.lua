function entry()
    local records = 
    {
        --[0x80]=ECUIdTable,
        [0x90]=Vin,
        [0x92]=HWRef,
        [0x95]=SWRef,
        [0x8A]=SerialNo,
    }


    for k, v in pairs(records) do
        local success, id, msg = Services.readECUIdentification(k)

        if success and id then
            log:info(msg .. "\n" .. records[k](id))
        else
            log:info(msg)
        end
    end
end


function ECUIdTable(data)
    local outMsg = ""
    outMsg = outMsg .. "Coding Idx: " .. string.format("%02X%02X\n", data[1], data[2])
    outMsg = outMsg .. "Diag Idx: " .. string.format("%02X%02X\n", data[3], data[4])
    outMsg = outMsg .. "Bus Idx: " .. string.format("%02X%02X\n", data[5], data[6])
    outMsg = outMsg .. "WK Man: " .. string.format("%02X%02X\n", data[6], data[8])
    outMsg = outMsg .. "YR Man: " .. string.format("%02X%02X\n", data[9], data[10])
    outMsg = outMsg .. "Supplier Code: " .. string.format("%02X%02X\n", data[11], data[12])
    outMsg = outMsg .. "Software Version: " .. string.format("%02X%02X", data[13], data[14])
    return outMsg
end

function Vin(data)
    local outMsg = "VIN: "
    for i = 1, #data do
        outMsg = outMsg .. string.char(data[i])
    end
    return outMsg
end

function HWRef(data)
    local outMsg = "HWRef: "
    for i = 1, #data do
        outMsg = outMsg .. string.char(data[i])
    end
    return outMsg
end

function SWRef(data)
    local outMsg = "SWRef: "
    for i = 1, #data do
        outMsg = outMsg .. string.char(data[i])
    end
    return outMsg
end

function SerialNo(data)
    local outMsg = "SerialNo: "
    for i = 1, #data do
        outMsg = outMsg .. string.char(data[i])
    end
    return outMsg
end