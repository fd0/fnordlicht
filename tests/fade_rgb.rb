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

puts "fade to rgb color"
fade_rgb(255, 0, 0, 0, 255, 1)
fade_rgb(255, 255, 0, 0, 2, 1)
sleep 2
fade_rgb(255, 0, 255, 0, 2, 1)
sleep 2
fade_rgb(255, 0, 0, 255, 2, 1)
