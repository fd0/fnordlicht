#!/usr/bin/env ruby

require 'serialport'

def msg(addr, cmd, data)
    $dev.write addr.chr
    $dev.write cmd.chr
    data[0..8].each do |d|
        $dev.write d.chr
    end
    $dev.flush
end

$dev = SerialPort.new("/dev/ttyUSB0", 19200)
file = 'testbin'

addr = 0

# config bootloader, write flash, 64 byte
msg(0, 0x80, [0, 0, 0, 64, 0,     0, 0, 0])

sleep 5

# read file, write to flash
f = open(file)
while not f.eof?
    data = f.read(8)
    msg(0, 0x81, [data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]])
    sleep 0.100
end
