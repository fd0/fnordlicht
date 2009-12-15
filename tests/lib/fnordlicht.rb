module Fnordlicht
    def sync(addr = 0)
        1.upto(15) do
            $dev.write "\e"
        end
        $dev.write addr.chr
        $dev.flush
    end

    def fade_rgb(addr, r, g, b, step, delay)
        $dev.write addr.chr
        $dev.write "\x01"
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
        $dev.write "\x02"
        $dev.write step.chr
        $dev.write delay.chr
        $dev.write [h].pack('v')
        $dev.write s.chr
        $dev.write v.chr
        $dev.write "\x00\x00\x00\x00\x00"
        $dev.write "\x00\x00"
        $dev.flush
    end

    def save_rgb(addr, slot, r, g, b, step, delay, pause)
        $dev.write addr.chr
        $dev.write "\x03"
        $dev.write slot.chr
        $dev.write step.chr
        $dev.write delay.chr
        $dev.write [pause].pack('S')
        $dev.write r.chr
        $dev.write g.chr
        $dev.write b.chr
        $dev.write "\x00\x00\x00\x00\x00"
        $dev.flush
    end

    def save_hsv(addr, slot, h, s, v, step, delay, pause)
        $dev.write addr.chr
        $dev.write "\x04"
        $dev.write slot.chr
        $dev.write step.chr
        $dev.write delay.chr
        $dev.write [pause].pack('S')
        $dev.write [h].pack('v')
        $dev.write s.chr
        $dev.write v.chr
        $dev.write "\x00\x00\x00\x00"
        $dev.flush
    end

    def save_current(addr, slot, step, delay, pause)
        $dev.write addr.chr
        $dev.write "\x05"
        $dev.write slot.chr
        $dev.write step.chr
        $dev.write delay.chr
        $dev.write [pause].pack('S')
        $dev.write "\x00\x00\x00\x00"
        $dev.write "\x00\x00\x00\x00"
        $dev.flush
    end

    def config_offsets(addr, step, delay, h, s, v)
        $dev.write addr.chr
        $dev.write "\x06"
        $dev.write [step].pack('c')
        $dev.write [delay].pack('c')
        $dev.write [h].pack('v')
        $dev.write s.chr
        $dev.write v.chr
        $dev.write "\x00\x00\x00\x00\x00\x00\x00"
        $dev.flush
    end

    def start_program(addr, program, params)
        $dev.write addr.chr
        $dev.write "\x07"
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

    def stop(addr, fading = 1)
        $dev.write addr.chr
        $dev.write "\x08"
        $dev.write fading.chr
        $dev.write "\x00\x00\x00\x00"
        $dev.write "\x00\x00\x00\x00\x00"
        $dev.write "\x00\x00\x00"
        $dev.flush
    end

    def modify_current(addr, step, delay, r, g, b, h, s, v)
        $dev.write addr.chr
        $dev.write "\x09"
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

    def pull_int(addr, delay)
        $dev.write addr.chr
        $dev.write "\x0A"
        $dev.write delay.chr

        $dev.write "\x00\x00"
        $dev.write "\x00\x00\x00\x00\x00"
        $dev.write "\x00\x00\x00\x00\x00"
        $dev.flush
    end

    def config_startup_nothing(addr)
        mode = 0 # do nothing

        $dev.write addr.chr
        $dev.write "\x0B"
        $dev.write mode.chr

        $dev.write "\x00\x00"
        $dev.write "\x00\x00\x00\x00\x00"
        $dev.write "\x00\x00\x00\x00\x00"
        $dev.flush
    end

    def config_startup_program(addr, program, params)
        mode = 1 # start program

        $dev.write addr.chr
        $dev.write "\x0B"
        $dev.write mode.chr
        $dev.write program.chr

        rest = 11-params.length
        puts "rest: %u" % rest if $verbose
        params.each do |p|
            $dev.write(p.chr)
        end
        1.upto(rest) do
            $dev.write("\x00")
        end
        $dev.flush
    end

    def powerdown(addr)
        $dev.write(addr.chr)
        $dev.write("\x0C")
        $dev.write("\x00"*13)
        $dev.flush
    end

    # secondary functions
    def fade_updown_rgb(addr, r, g, b, step, delay, sleep_time)
        fade_rgb(addr, r, g, b, step, delay)
        sleep(sleep_time)
        fade_rgb(addr, 0, 0, 0, step, delay)
        sleep(sleep_time)
    end

    def fade_updown_hsv(addr, h, s, v, step, delay, sleep_time)
        fade_hsv(addr, h, s, v, step, delay)
        sleep(sleep_time)
        fade_hsv(addr, h, s, 0, step, delay)
        sleep(sleep_time)
    end

    # bootloader functions
    def start_bootloader(addr)
        $dev.write(addr.chr)
        $dev.write("\x80")
        $dev.write("\x6b\x56\x27\xfc")
        $dev.write("\x00\x00\x00\x00\x00\x00\x00\x00\x00")
        $dev.flush
    end

    def boot_config(addr, start_addr)
        $dev.write(addr.chr)
        $dev.write("\x81")
        $dev.write [start_addr].pack('v')
        $dev.write("\x00"*11)
        $dev.flush
    end

    def boot_init(addr)
        $dev.write(addr.chr)
        $dev.write("\x82")
        $dev.write("\xff" * 13)
        $dev.flush
    end

    def boot_data(addr, data)
        $dev.write(addr.chr)
        $dev.write("\x83")

        # just write the first 13 bytes
        data = data[0..12]
        $dev.write(data)

        $dev.write("\xff" * (13-data.length))
        $dev.flush
    end

    def boot_crc_check(addr, len, checksum, delay)
        $dev.write(addr.chr)
        $dev.write("\x84")
        $dev.write [len].pack('v')
        $dev.write [checksum].pack('v')
        $dev.write delay.chr
        $dev.write("\x00"*8)
        $dev.flush
    end

    def boot_crc_flash(addr, start, len, checksum, delay)
        $dev.write(addr.chr)
        $dev.write("\x85")
        $dev.write [start].pack('v')
        $dev.write [len].pack('v')
        $dev.write [checksum].pack('v')
        $dev.write delay.chr
        $dev.write("\x00"*6)
        $dev.flush
    end

    def boot_flash(addr)
        $dev.write(addr.chr)
        $dev.write("\x86")
        $dev.write("\x00"*13)
        $dev.flush
    end

    def boot_enter_application(addr)
        $dev.write(addr.chr)
        $dev.write("\x87")
        $dev.write("\x00"*13)
        $dev.flush
    end
end
