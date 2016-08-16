function init()
    file.open('token', 'r')
    token = string.sub(file.readline(), 1, -2)
    file.close()

    file.open('wifi', 'r')
    ssid = string.sub(file.readline(), 1, -2)
    password = string.sub(file.readline(), 1, -2)
    file.close()

    file.open('version', 'r')
    version = tonumber(string.sub(file.readline(), 1, -2))
    file.close()

    server = "api.thethings.io"
    keepalive = 30000
end

-- Set the esp8266 as station an connect to desired WiFi
function connect(ssid, password)
    wifi.setmode(wifi.STATION)
    wifi.sta.config(ssid, password)
    wifi.sta.connect()
    tmr.alarm(0, 5000, 0, checkConnection)
end

first_connection = true
-- Check the connection status, connect if disconnected and create the mqtt
-- client when connected
function checkConnection()
    if first_connection or not wifi.sta.status() == 5 then
        first_connection = false
        print("Connecting...")
        connect(ssid, password)
    else
        print("Connected!")
        tmr.alarm(0, 5000, 0, checkVersion)
    end
end

function checkVersion()
    local socket = net.createConnection(net.TCP, 0)
    socket:connect(80, server)
    print("checking version")

    socket:on("connection", function(socket, string)
        socket:send('GET /v2/things/' .. token .. '/firmwares/latest HTTP/1.1\n')
        socket:send('Host: api.thethings.io\n')
        socket:send('Accept: application/json\n\n')
    end)

    socket:on("receive", function(socket, received)
        local latest = version
        file_id = ''
        local i, j = string.find(received, '{')
        local json = string.sub(received, i, -1)
        json = cjson.decode(json)
        if json['status'] == 'success' then
            for key, values in pairs(json['firmwares']) do
                if tonumber(values['version']) > latest then
                    latest = tonumber(values['version'])
                    file_id = values['_id']
                end
            end
        end
        if latest > version then
            print("updating to version" .. latest)
            tmr.alarm(1, 1000, 0, updateVersion)
        else
            print("no need to update")
        end
        socket:close()
    end)
end

function updateVersion()
    local socket = net.createConnection(net.TCP, 0)
    socket:connect(80, server)

    socket:on("connection", function(socket, string)
        first_file = true
        socket:send('GET /v2/things/' .. token .. '/firmwares/download/' .. file_id .. ' HTTP/1.1\n')
        socket:send('Host: api.thethings.io\n')
        socket:send('Accept: application/json\n\n')
        print("Editing tmp.lua")
        file.remove("tmp.lua")
    end)

    socket:on("receive", function(socket, received)
        if first_file then
            first_file = false
            local i, j = string.find(received, "function")
            received = string.sub(received, i, -1)
            file.open("tmp.lua", "w")
        end
        local i, j = string.find(received, "0")
        if i ~= 1 then
            file.write(received)
            file.flush()
        end
    end)

    socket:on("disconnection", function(socket)
        print("End write tmp.lua")
        file.close()
        socket:close()
    end)

end

init()
checkConnection()
