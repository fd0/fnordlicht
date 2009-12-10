#!/usr/bin/env ruby

$:.unshift(File.dirname(__FILE__)+"/lib")

require 'fnordlicht'
require 'serialport'

include Fnordlicht

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

def crc16_update(crc, data)
    return nil if crc < 0 || crc > 0xffff
    return nil if data < 0 || data > 255

    crc ^= data
    0.upto(7) do |i|
        if crc % 2 == 1
             crc = (crc >> 1) ^ 0xA001
        else
             crc = (crc >> 1)
        end
    end

    return crc
end


def compute_checksum(data)
    checksum  = 0xffff

    data.each_byte do |b|
        checksum = crc16_update(checksum, b)
    end

    return checksum
end

if ARGV.length != 2
    $stderr.puts "usage: %s ADDRESS FILE" % $0
    exit 1
end

address = ARGV.shift.to_i
file = ARGV.shift

puts "sending sync sequence"
sync()

puts "flash"
open(file, 'r') do |f|
    puts "configure bootloader"
    boot_config(address, 0)
    loop do
        d = f.read(512)
        break if d.nil?
        puts "writing chunk (%s bytes)" % d.length

        boot_init(address);
        d.split('').each_slice(13) do |s|
            boot_data(address, s.join(''))
            #sleep 0.05
        end
        checksum = compute_checksum(d)
        boot_crc_check(address, d.length, checksum, 5);

        puts "flashing..."
        boot_flash(address)
        sleep 0.3

        break if d.length < 512
    end
end

data = ""
open(file, 'r') do |f|
    d = f.read()
    break if d.nil?
    data += d
end
checksum = compute_checksum(data)
puts "verifying checksum (%u bytes): 0x%04x" % [data.length, checksum]
boot_crc_flash(address, 0, data.length, checksum, 50)
puts "done"
