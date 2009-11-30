#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()

puts "save rgb: light orange (fast fading)"
save_rgb(255,   0,   255, 150, 0,   10, 1,  10);
