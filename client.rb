#!/usr/bin/env ruby

require 'serialport'

dev = SerialPort.new("/dev/ttyUSB0", 115200)

#dev.write "\x00\x01\xff\x00\x00"
#dev.write "\x01\x01\x00\xff\x00"
#dev.write "\xff\x01\x00\x00\xff"


#dev.write "\x00\x01\xff\xFF\xFF"
#dev.write "\x01\x01\x23\x4f\x00"
#dev.write "\x02\x01\xff\xff\x00"
#dev.write "\x03\x01\xff\x00\xff"
#dev.write "\x10\x01\xff\x00\x00"
#dev.write "\x11\x01\x00\xff\x00"
#dev.write "\x12\x01\xff\xff\x00"
#dev.write "\x13\x01\xff\x00\xff"
#dev.flush

loop do
    0.upto 3 do |addr|
        puts addr
        dev.write addr.chr
        #dev.write "\xff"
        dev.write "\x01"
        dev.write rand(255).chr
        dev.write rand(255).chr
        dev.write rand(255).chr
        dev.write "\x00\x00\x00\x00\x00"
        dev.flush
        sleep 0.2
    end

    if rand(3) == 0
        dev.write "\xff\x01\xff\xff\xff"
        dev.write "\x00\x00\x00\x00\x00"
        dev.flush
        sleep 0.2
        dev.write "\xff\x01\xff\x00\x00"
        dev.write "\x00\x00\x00\x00\x00"
        dev.flush
        sleep 0.1
        dev.write "\xff\x01\xff\xff\xff"
        dev.write "\x00\x00\x00\x00\x00"
        dev.flush
        sleep 0.2
    end
end
