#!/usr/bin/env ruby

require 'serialport'

def color(addr, r, g, b)
    $dev.write addr.chr
    $dev.write "\x01"
    $dev.write r.chr
    $dev.write g.chr
    $dev.write b.chr
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.flush
end

def config(addr, scripting)
    $dev.write addr.chr
    $dev.write "\x03"
    $dev.write scripting.chr
    $dev.write "\x00\x00\x00\x00\x00\x00\x00"
    $dev.flush
end

def fade(addr, speedl, speedh, r, g, b)
    $dev.write addr.chr
    $dev.write "\x02"
    $dev.write speedl.chr
    $dev.write speedh.chr
    $dev.write r.chr
    $dev.write g.chr
    $dev.write b.chr
    $dev.write "\x00\x00\x00"
    $dev.flush
end

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

config(255, 0)

color(255, 0, 0, 0)

# white
fade(255, 0, 5, 0, 0, 0)

fade(255, 0, 5, 255, 255, 255)
sleep 4

fade(255, 0, 1, 0, 0, 0)
sleep 4

# primary colors
fade(255, 0, 1, 255, 0, 0)
sleep 4
fade(255, 0, 1, 0, 255, 0)
sleep 4
fade(255, 0, 1, 0, 0, 255)
sleep 4

# secondary colors
fade(255, 0, 1, 255, 255, 0)
sleep 4
fade(255, 0, 1, 0, 255, 255)
sleep 4
fade(255, 0, 1, 255, 0, 255)
sleep 4

# black
fade(255, 0, 1, 0, 0, 0)
sleep 4

# rainbow
fade(0, 0, 1, 255,   0,   0)
sleep 0.1
fade(1, 0, 1, 255, 255,   0)
sleep 0.1
fade(2, 0, 1,   0, 255,   0)
sleep 0.1
fade(3, 0, 1,   0, 255, 255)
sleep 0.1
fade(4, 0, 1,   0,   0, 255)
sleep 0.1
fade(5, 0, 1, 255,   0,   0)
sleep 0.1
fade(6, 0, 1, 255, 255,   0)
sleep 0.1
fade(7, 0, 1,   0, 255,   0)
sleep 0.1
fade(8, 0, 1,   0, 255, 255)
sleep 0.1
fade(4, 0, 1,   0,   0, 255)
sleep 4
fade(0, 0, 1, 255,   0,   0)
fade(1, 0, 1, 255,   0,   0)
fade(2, 0, 1, 255, 255,   0)
fade(3, 0, 1, 255, 255,   0)
fade(4, 0, 1,   0, 255,   0)
fade(5, 0, 1,   0, 255,   0)
fade(6, 0, 1,   0, 255, 255)
fade(7, 0, 1,   0, 255, 255)
fade(8, 0, 1,   0,   0, 255)
sleep 4
fade(0, 0, 1, 255,   0,   0)
fade(1, 0, 1, 255,   0,   0)
fade(2, 0, 1, 255,   0,   0)
fade(3, 0, 1, 255, 255,   0)
fade(4, 0, 1, 255, 255,   0)
fade(5, 0, 1, 255, 255,   0)
fade(6, 0, 1,   0, 255,   0)
fade(7, 0, 1,   0, 255,   0)
fade(8, 0, 1,   0, 255,   0)
sleep 4
fade(0, 0, 1,   0, 255, 255)
fade(1, 0, 1,   0, 255, 255)
fade(2, 0, 1,   0, 255, 255)
fade(3, 0, 1,   0,   0, 255)
fade(4, 0, 1,   0,   0, 255)
fade(5, 0, 1,   0,   0, 255)
fade(6, 0, 1, 255,   0, 255)
fade(7, 0, 1, 255,   0, 255)
fade(8, 0, 1, 255,   0, 255)
sleep 4


fade(255, 0, 2, 0, 0, 0)
sleep 4


3.times do
    delay = 0.2
    0.upto 8 do |addr|
        fade(addr, 0, 3, 255, 0, 0)
        sleep delay
    end
    0.upto 8 do |addr|
        fade(addr, 0, 3, 255, 255, 0)
        sleep delay
    end
    0.upto 8 do |addr|
        fade(addr, 0, 3, 0, 255, 0)
        sleep delay
    end
    0.upto 8 do |addr|
        fade(addr, 0, 3, 0, 255, 255)
        sleep delay
    end
    0.upto 8 do |addr|
        fade(addr, 0, 3, 0, 0, 255)
        sleep delay
    end
    0.upto 8 do |addr|
        fade(addr, 0, 3, 255, 0, 255)
        sleep delay
    end
end

sleep 4

fade(255, 0, 1, 0, 0, 0)
