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

puts "save 11 random hue values (hsv) in eeprom"
0.upto 10 do |i|
    hue = rand(360)
    puts "hue: %u" % hue
    save_hsv(255,   i,   hue, 255, 255,   10, 1,  10);
    sleep 0.1
end

puts "fade to blue"
fade_hsv(255,     240, 255, 255,  10, 1)

sleep 2

puts "start program replay, 3-7 no repeat"
start_program(255, 2, [3, 7, 0])

sleep 10

puts "start program replay, 5-7 repeat from start"
start_program(255, 2, [5, 7, 1])

sleep 20

puts "start program replay, 5-7 repeat inverse"
start_program(255, 2, [5, 7, 2])
