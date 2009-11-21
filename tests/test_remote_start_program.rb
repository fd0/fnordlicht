#!/usr/bin/env ruby

require 'serialport'

def sync(addr = 0)
    1.upto(15) do
        $dev.write "\e"
    end
    $dev.write addr.chr
    $dev.flush
end

def stop(addr)
    $dev.write addr.chr
    $dev.write "\x0A"
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00\x00"
    $dev.flush
end

def fade_rgb(addr, r, g, b, step, delay)
    $dev.write addr.chr
    $dev.write "\x02"
    $dev.write step.chr
    $dev.write delay.chr
    $dev.write r.chr
    $dev.write g.chr
    $dev.write b.chr
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00\x00"
    $dev.flush
end

def fade_hsv(addr, h, s, v, step, delay)
    $dev.write addr.chr
    $dev.write "\x03"
    $dev.write step.chr
    $dev.write delay.chr
    $dev.write [h].pack('v')
    $dev.write s.chr
    $dev.write v.chr
    $dev.write "\x00\x00\x00\x00\x00"
    $dev.write "\x00\x00"
    $dev.flush
end

def modify_current(addr, step, delay, r, g, b, h, s, v)
    $dev.write addr.chr
    $dev.write "\x0b"
    $dev.write step.chr
    $dev.write delay.chr
    $dev.write [r].pack('c')
    $dev.write [g].pack('c')
    $dev.write [b].pack('c')
    $dev.write [h].pack('v')
    $dev.write [s].pack('c')
    $dev.write [v].pack('c')
    $dev.write "\x00\x00\x00\x00"
    $dev.flush
end

def config(addr, scripting)
    $dev.write addr.chr
    $dev.write "\x03"
    $dev.write scripting.chr
    $dev.write "\x00\x00\x00\x00\x00\x00\x00"
    $dev.flush
end

def fade(addr, speedl, speedh, r, g, b)
    $dev.write addr.chr
    $dev.write "\x02"
    $dev.write speedl.chr
    $dev.write speedh.chr
    $dev.write r.chr
    $dev.write g.chr
    $dev.write b.chr
    $dev.write "\x00\x00\x00"
    $dev.flush
end

def start_program(addr, program, params)
    $dev.write addr.chr
    $dev.write "\x09"
    $dev.write program.chr
    rest = 12-params.length
    puts "rest: %u" % rest if $verbose
    params.each do |p|
        $dev.write(p.chr)
    end
    1.upto(rest) do
        $dev.write("\x00")
    end
    $dev.flush
end

$dev = SerialPort.new("/dev/ttyUSB0", 19200)

puts "sending sync sequence"
sync()
sleep 0.5

puts "stop fading"
stop(255)
sleep 0.5

puts "start program colorwheel"
start_program(255, 0, [5, 1, 10,  0, 0,  60, 0, 255, 255])


sleep 4
puts "stop"
stop(255)

puts "fade to hsv color"
fade_hsv(255, 300, 180, 100, 2, 1)
