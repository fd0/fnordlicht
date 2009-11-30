#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()

stop(255)

puts "starting slow fade from red to green"
fade_rgb(255, 255, 0, 0, 255, 1)
sleep 0.1

puts "save start color"
save_current(255, 0,  8, 1, 10)

fade_rgb(255, 0, 255, 0,   1, 2)

sleep 1.8
puts "save in the middle"
save_current(255, 1,  8, 1, 10)

sleep 3
puts "save at the end"
save_current(255, 2,  8, 1, 10)

sleep 0.1

puts "start program replay, 0-2, repeat from start"
start_program(255, 2, [0, 2, 1])
