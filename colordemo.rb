#!/usr/bin/env ruby

require 'serialport'

def color(addr, r, g, b)
    $dev.write addr.chr
    $dev.write "\x01"
    $dev.write r.chr
    $dev.write g.chr
    $dev.write b.chr
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.flush
end

$dev = SerialPort.new("/dev/ttyUSB0", 115200)


# white
color(255, 255, 255, 255)
$stdin.readline

# primary colors
color(255, 255, 0, 0)
$stdin.readline
color(255, 0, 255, 0)
$stdin.readline
color(255, 0, 0, 255)
$stdin.readline

# secondary colors
color(255, 255, 255, 0)
$stdin.readline
color(255, 0, 255, 255)
$stdin.readline
color(255, 255, 0, 255)
$stdin.readline

# rainbow
color(0, 255,   0,   0)
color(1, 255, 255,   0)
color(2,   0, 255,   0)
color(3,   0, 255, 255)
color(4,   0,   0, 255)
color(5, 255,   0,   0)
color(6, 255, 255,   0)
color(7,   0, 255,   0)
color(8,   0, 255, 255)
color(4,   0,   0, 255)
$stdin.readline
color(0, 255,   0,   0)
color(1, 255,   0,   0)
color(2, 255, 255,   0)
color(3, 255, 255,   0)
color(4,   0, 255,   0)
color(5,   0, 255,   0)
color(6,   0, 255, 255)
color(7,   0, 255, 255)
color(8,   0,   0, 255)
$stdin.readline
color(0, 255,   0,   0)
color(1, 255,   0,   0)
color(2, 255,   0,   0)
color(3, 255, 255,   0)
color(4, 255, 255,   0)
color(5, 255, 255,   0)
color(6,   0, 255,   0)
color(7,   0, 255,   0)
color(8,   0, 255,   0)
$stdin.readline
color(0,   0, 255, 255)
color(1,   0, 255, 255)
color(2,   0, 255, 255)
color(3,   0,   0, 255)
color(4,   0,   0, 255)
color(5,   0,   0, 255)
color(6, 255,   0, 255)
color(7, 255,   0, 255)
color(8, 255,   0, 255)
$stdin.readline



color(255, 0, 0, 0)
