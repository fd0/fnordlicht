#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()

puts "save 11 random hue values (hsv) in eeprom"
0.upto 10 do |i|
    hue = rand(360)
    puts "hue: %u" % hue
    save_hsv(255,   i,   hue, 255, 255,   10, 1,  10);
    sleep 0.1
end
