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

puts "start program colorwheel"
start_program(255, 0, [5, 1, 10,  0, 0,  60, 0, 255, 255])

sleep 4
puts "stop"
stop(255)

puts "fade to hsv color"
fade_hsv(255, 300, 180, 100, 2, 1)
