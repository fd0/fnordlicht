#!/usr/bin/env ruby

$:.unshift "lib"

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()
puts "stop fading"
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

