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
sleep 2

puts "fade to rgb color, test speed computation"
fade_rgb(255, 255, 128, 0, 255, 1)
sleep 0.5
fade_rgb(255, 0, 0, 0, 3, 1)
