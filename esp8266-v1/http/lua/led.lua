function init()
    -- Change these variables to suit your needs
    token = "YOUR THING TOKEN"
    ssid = "SSID"
    password = "SSID PASSWORD"

    server = "api.devices.thethings.io"
    keepalive = 30000

    led_pin = 4
    gpio.mode(led_pin, gpio.OUTPUT)
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
        tmr.alarm(0, 500, 0, createSocket)
    end
end

function createSocket()
    socket = net.createConnection(net.TCP, 0)
    socket:connect(80, server)
    socket:on("connection", connection)
    socket:on("disconnection", disconnection)
    socket:on("receive", receive)
end

function connection(socket, string)
    print("CONNECTION")
    socket:send("GET /v2/things/" .. token .. "?keepAlive=" .. keepalive .. " HTTP/1.1\n")
    socket:send("Host: api.thethings.io\n")
    socket:send("Accept: application/json\n\n")
end

function disconnection(socket, string)
    print("DISCONNECTION")
    checkConnection()
end

function receive(socket, received)
    local received = string.upper(received)
    received = string.gsub(received, " ", "")
    if string.find(received, "\"VALUE\":\"ON\"") then
        gpio.write(led_pin, gpio.HIGH)
        print("ON")
    elseif string.find(received, "\"VALUE\":\"OFF\"") then
        gpio.write(led_pin, gpio.LOW)
        print("OFF")
    end
    print(received)
end

init()
checkConnection()
