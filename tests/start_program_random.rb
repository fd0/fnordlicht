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

puts "start program random"
start_program(255, 1, [Time.now.usec & 0xff, Time.now.usec & 0xff, 1,   5, 1, 10,  255, 255,  40])
