#!/usr/bin/env ruby

require 'serialport'

def sync(addr = 0)
    1.upto(15) do
        $dev.write "\e"
    end
    $dev.write addr.chr
    $dev.flush
end

def stop(addr)
    $dev.write addr.chr
    $dev.write "\x0A"
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00\x00"
    $dev.flush
end

def fade_rgb(addr, r, g, b, step, delay)
    $dev.write addr.chr
    $dev.write "\x02"
    $dev.write step.chr
    $dev.write delay.chr
    $dev.write r.chr
    $dev.write g.chr
    $dev.write b.chr
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00\x00"
    $dev.flush
end

def fade_hsv(addr, h, s, v, step, delay)
    $dev.write addr.chr
    $dev.write "\x03"
    $dev.write step.chr
    $dev.write delay.chr
    $dev.write [h].pack('v')
    $dev.write s.chr
    $dev.write v.chr
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00"
    $dev.flush
end

def modify_current(addr, step, delay, r, g, b, h, s, v)
    $dev.write addr.chr
    $dev.write "\x0b"
    $dev.write step.chr
    $dev.write delay.chr
    $dev.write [r].pack('c')
    $dev.write [g].pack('c')
    $dev.write [b].pack('c')
    $dev.write [h].pack('v')
    $dev.write [s].pack('c')
    $dev.write [v].pack('c')
    $dev.write "\x00\x00\x00\x00"
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

puts "sending sync sequence"
sync()
sleep 0.5

puts "stop fading"
stop(255)
sleep 0.5

# test rgb fading
puts "fade to rgb color"
fade_rgb(255, 0, 0, 0, 255, 1)
sleep 0.5
fade_rgb(255, 0, 255, 255, 1, 1)
sleep 3
stop(255)

puts "fade to hsv color"
fade_hsv(255, 0, 255, 255, 2, 1)
sleep 2
fade_hsv(255, 120, 255, 255, 2, 1)
sleep 2
fade_hsv(255, 180, 255, 255, 2, 1)
sleep 2
fade_hsv(255, 240, 255, 255, 2, 1)
sleep 2

fade_hsv(255, 200, 255, 150, 2, 1)
sleep 2
fade_hsv(255, 200, 255, 20, 2, 1)
sleep 2
fade_hsv(255, 200, 255, 100, 2, 1)
sleep 2
fade_hsv(255, 200, 30, 100, 2, 1)
sleep 2
fade_hsv(255, 200, 100, 100, 2, 1)
sleep 2

puts "change current color - rgb"
modify_current(255, 1, 1,  50, 0, 0,  0, 0, 0)
sleep 2
modify_current(255, 1, 1,  50, 0, 0,  0, 0, 0)
sleep 2
modify_current(255, 1, 1,  50, 0, 0,  0, 0, 0)
sleep 2
modify_current(255, 1, 1,  -80, 0, 0,  0, 0, 0)
sleep 2
modify_current(255, 1, 1,  -100, 100, 127,  0, 0, 0)
sleep 2

puts "change current color - hsv"
fade_hsv(255,   60, 220, 120,    5, 1)
sleep 2

modify_current(255, 2, 1,  0, 0, 0,  235, 0, 0)
sleep 2
modify_current(255, 2, 1,  0, 0, 0,  235, 0, 0)
sleep 2
modify_current(255, 2, 1,  0, 0, 0,  235, 0, 0)
sleep 2
modify_current(255, 2, 1,  0, 0, 0,  235, 0, 0)
sleep 2
0.upto 2 do
    modify_current(255, 5, 1,  0, 0, 0,  0, 0, -80)
    sleep 1
    modify_current(255, 5, 1,  0, 0, 0,  0, 0, 80)
    sleep 1
end
