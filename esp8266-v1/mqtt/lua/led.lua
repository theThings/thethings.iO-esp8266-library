function init()
    -- Change these variables to suit your needs
    token = "YOUR THING TOKEN"
    ssid = "SSID"
    password = "SSID PASSWORD"

    server = "mqtt.thethings.io"
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
        createMQTTClient()
    end
end

function createMQTTClient()
    m = mqtt.Client("esp8266Client", 120, "", "")

    m:on("offline", function(client)
        print("MQTT offline, reconnecting...")
        checkConnection()
    end)

    m:on("message", function(client, topic, message)
        print(topic .. ":" )
        message = string.upper(message)
        message = string.gsub(message, " ", "")
        if string.find(message, "\"VALUE\":\"ON\"}") then
            gpio.write(led_pin, gpio.HIGH)
        elseif string.find(message, "\"VALUE\":\"OFF\"}") then
            gpio.write(led_pin, gpio.LOW)
        end
    end)

    m:connect(server, 1883, 0, function(conn)
        print("MQTT connected")
        m:subscribe("v2/things/"..token, 1, function(client, topic, message)
            print("Subscribed to " .. token)
        end)
    end)
end

init()
checkConnection()
