function init()
    -- Change these variables to suit your needs
    token = "YOUR THING TOKEN"
    ssid = "SSID"
    password = "SSID PASSWORD"

    server = "api.thethings.io"
    keepalive = 30000

    button_pin = 4
    pushed = false

    gpio.mode(button_pin, gpio.INT)
    gpio.trig(button_pin, "low", checkButton)
end

-- Set the esp8266 as station an connect to desired WiFi
function connect(ssid, password)
    wifi.setmode(wifi.STATION)
    wifi.sta.config(ssid, password)
    wifi.sta.connect()
    tmr.alarm(0, 10000, 0, checkConnection)
end

-- Check the connection status, connect if disconnected and create the mqtt
-- client when connected
function checkConnection()
    if not wifi.sta.status() == 5 then
        print("Connecting...")
        connect(ssid, password)
    else
        print("Connected!")
    end
end

function sendPushed()
    if not wifi.sta.status() == 5 then
        checkConnection()
        return
    end
    socket = net.createConnection(net.TCP, 0)
    socket:connect(80, server)
    socket:on("connection",
        function()
            local payload = "{\"values\":[{\"key\":\"button\",\"value\":1}]}"
            local data =   "POST /v2/things/" .. token .. " HTTP/1.1\n"
            data = data .. "Host: api.thethings.io\n"
            data = data .. "Content-Length: " .. string.len(payload) .. "\n"
            data = data .. "Content-Type: application/json\n"
            data = data .. "Accept: application/json\n\n"
            data = data .. payload .. "\r\n"
            socket:send(data)
            socket:close()
            print(data)
        end
    )
end

function checkButton(level)
    if level == 0 and not pushed then
        print("PUSHED")
        pushed = true
        sendPushed()
    else
        if level == 1 and pushed then
            pushed = false
        end
    end
end

init()
checkConnection()
