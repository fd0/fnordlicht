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
fade_rgb(255,  0, 0, 0,  255, 0)
puts "reset offsets "
config_offsets(255,  0, 0,   0, 255, 255)

puts "fade slow rgb (big steps)"
fade_updown_rgb(255,  255, 255, 0,  10, 10,  3)

puts "fade slow hsv (big steps)"
fade_updown_hsv(255,  120, 255, 255,  10, 10,  3)

puts "configuring delay offset"
config_offsets(255,  0, -5,   0, 255, 255)

puts "fade slow rgb (big steps, but modified to fast)"
fade_updown_rgb(255,  255, 255, 0,  10, 10,  1)

puts "fade slow hsv (big steps, but modified to fast)"
fade_updown_hsv(255,  120, 255, 255,  10, 10,  1)
