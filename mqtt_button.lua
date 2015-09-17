function init()
    -- Change these variables to suit your needs
    token = "YOUR THING TOKEN"
    ssid = "SSID"
    password = "SSID PASSWORD"

    server = "mqtt.thethings.io"
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
        createMQTTClient()
    end
end

function createMQTTClient()
    m = mqtt.Client("esp8266Client", 120, "", "")

    m:on("offline", function(client)
        print("MQTT offline, reconnecting...")
        checkConnection()
    end)

    m:connect(server, 1883, 0, function(conn)
        print("MQTT connected")
    end)
end

function sendPushed()
    if not wifi.sta.status() == 5 then
        checkConnection()
        return
    end
    m:publish("v2/things/"..token, "{\"values\":[{\"key\":\"button\",\"value\":1}]}", 0, 0, function(client) print("sent") end)
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
