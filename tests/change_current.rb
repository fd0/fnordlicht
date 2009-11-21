#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()
puts "stop fading"
stop(255)

puts "fade to hsv color"
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
