#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()

puts "configure startup: program random (fast, no min_distance)"
config_startup_program(255, 1, [Time.now.usec & 0xff, Time.now.usec & 0xff, 1,   8, 1, 10, 0,  255, 255,  0])
