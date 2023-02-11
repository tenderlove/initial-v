#!/Users/aaron/.rubies/arm64/ruby-trunk/bin/ruby

require "myhidapi"

class Handle
  NONE      = 0
  BACKLIGHT = 1
  RESET     = 2
  DRIVE     = 3
  NEUTRAL   = 4
  REVERSE   = 5
  PARK      = 6

  def initialize
    devices = MyHIDAPI.enumerate 0x0, 0x0
    dev = devices.find { |dev|
      if dev.product_string =~ /Initial/i
        p dev
      end
      dev.product_string =~ /Initial/i && dev.usage == 0x61
    }
    @handle = dev.open
    100.times do
      break if @handle
      @handle = dev.open
    end
    raise "Couldn't connect" unless @handle
  end

  def reset!; send_command RESET; end
  def drive!; send_command DRIVE; end
  def neutral!; send_command NEUTRAL; end
  def reverse!; send_command REVERSE; end
  def park!; send_command PARK; end
  def backlight!; send_command BACKLIGHT; end

  private

  def send_command cmd, param = nil
    buf = [0x0, cmd, param].compact
    loop do
      break if @handle.write buf.pack('C*')
    end
  end
end

if __FILE__ == $0
  handle = Handle.new
  cmd = ARGV[0] || "reset"
  case cmd
  when "reset"
    handle.reset!
    sleep 1
    handle.backlight!
  when "drive" then handle.drive!
  when "neutral" then handle.neutral!
  when "park" then handle.park!
  when "reverse" then handle.reverse!
  end
end
