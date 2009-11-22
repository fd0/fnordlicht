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

puts "fade slow rgb"
fade_updown_rgb(255,  255, 255, 0,  1, 1,  3)

puts "fade slow hsv"
fade_updown_hsv(255,  120, 255, 255,  1, 1,  3)

puts "configuring positive step offset"
config_offsets(255,  10, 0,   0, 255, 255)

puts "fade slow rgb (but modified to fast)"
fade_updown_rgb(255,  255, 255, 0,  1, 1,  1)

puts "fade slow hsv (but modified to fast)"
fade_updown_hsv(255,  120, 255, 255,  1, 1,  1)

puts "reset offsets "
config_offsets(255,  0, 0,   0, 255, 255)

puts "fade fast rgb"
fade_updown_rgb(255,  255, 255, 0,  10, 1,  1)

puts "fade fast hsv"
fade_updown_hsv(255,  120, 255, 255,  10, 1,  1)

puts "configuring negative step offset"
config_offsets(255,  -10, 0,   0, 255, 255)

puts "fade fast rgb (but modified to slow)"
fade_updown_rgb(255,  255, 255, 0,  10, 1,  3)

puts "fade fast hsv (but modified to slow)"
fade_updown_hsv(255,  120, 255, 255,  10, 1,  3)

