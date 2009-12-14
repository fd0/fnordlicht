#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()

puts "configure startup: program colorwheel (fast, start with yellow)"
config_startup_program(255, 0, [8, 1, 10,  60, 0,  60, 0, 0, 255, 255])
