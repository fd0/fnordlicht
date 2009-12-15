#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

if ARGV.length != 1
    $stderr.puts "USAGE: #{$0} <addr>"
    exit 1
end

addr = ARGV.shift.to_i

puts "sending sync sequence"
sync()

puts "starting bootloader"
start_bootloader(addr)

sleep 0.2
puts "sending sync sequence"
sync()

puts "starting firmware"
boot_enter_application(addr)
