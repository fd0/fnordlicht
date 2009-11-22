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

puts "fade to blue"
fade_updown_hsv(255,  240, 255, 255,  10, 1,  1)

puts "configuring saturation scale"
config_offsets(255,  10, 0,   0, 100, 255)

puts "fade to blue (modified to light blue)"
fade_updown_hsv(255,  240, 255, 255,  10, 1,  1)

puts "reset offsets "
config_offsets(255,  0, 0,   0, 255, 255)

puts "fade to blue"
fade_updown_hsv(255,  240, 255, 255,  10, 1,  1)

puts "configuring value scale"
config_offsets(255,  10, 0,   0, 255, 100)

puts "fade to blue (modified to blue, low intensity)"
fade_updown_hsv(255,  240, 255, 255,  10, 1,  1)

puts "reset offsets "
config_offsets(255,  0, 0,   0, 255, 255)

puts "fade to blue"
fade_updown_hsv(255,  240, 255, 255,  10, 1,  1)

puts "configuring hue offset"
config_offsets(255,  10, 0,   80, 255, 255)

puts "fade to blue (modified to pink)"
fade_updown_hsv(255,  240, 255, 255,  10, 1,  1)

puts "configuring negative hue offset"
config_offsets(255,  10, 0,   -80, 255, 255)

puts "fade to blue (modified to light green)"
fade_updown_hsv(255,  240, 255, 255,  10, 1,  1)

