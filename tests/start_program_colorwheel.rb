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

puts "start program colorwheel (forward)"
start_program(255, 0, [5, 1, 1,  0,  0,  60, 0, 255, 255, 255])
sleep 10

puts "start program colorwheel (reverse)"
start_program(255, 0, [5, 1, 1,  0,  0,  60, 0, 1, 255, 255])
