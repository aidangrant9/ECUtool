function entry()
    local todump = "=== ECU Identification Dump ===\n\n"
    local count = 0
    
    for i = 0, 0xFF do
        local success, id, msg = Services.readECUIdentification(i)
        connection:sleep(5)

        if success and id then
            count = count + 1
            local str = ""
            local hex = ""
            
            for j = 1, #id do
                local char = string.char(id[j])
                str = str .. char
                hex = hex .. string.format("%02X ", id[j])
            end
            
            todump = todump .. string.format("ID 0x%02X:\n", i)
            todump = todump .. "  ASCII: " .. str .. "\n"
            todump = todump .. "  HEX:   " .. hex .. "\n\n"
        end
    end
    
    if count > 0 then
        todump = todump .. "=== Total records found: " .. count .. " ==="
        log:info(todump)
    else
        log:info("No ECU identification records found.")
    end
end