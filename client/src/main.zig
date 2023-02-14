const std = @import("std");
const Allocator = std.mem.Allocator;
const process = std.process;
const warn = std.debug.print;

const c = @cImport({
    @cInclude("hidapi.h");
});

const stdlib = @cImport({
    @cInclude("stdlib.h");
});

const Command = enum {
    none,
    backlight,
    reset,
    drive,
    neutral,
    reverse,
    park,
    vim,
    general,
};

pub fn wcstombs(allocator: *const Allocator, str: [*c]c_int) ?[]const u8 {
    var buf: [4096]u8 = undefined;
    var len = stdlib.wcstombs(&buf, str, 4096);
    if (len > 0) {
        const result = allocator.alloc(u8, len) catch return null;
        std.mem.copy(u8, result, buf[0..len]);
        return result;
    } else {
        return null;
    }
}

const keywords = std.ComptimeStringMap(Command, .{
    .{ "drive", .drive },
    .{ "neutral", .neutral },
    .{ "reverse", .reverse },
    .{ "park", .park },
    .{ "vim", .vim },
    .{ "general", .general },
});

pub fn main() anyerror!void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

    defer arena.deinit();

    const allocator = arena.allocator();

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if (args.len != 2) {
        std.log.err("Got wrong number of args {d}", .{args.len});
        std.process.exit(1);
    }

    const command: Command = keywords.get(args.ptr[1]) orelse
        @panic("Unknown command");

    std.log.info("Got command {any}", .{command});

    const bytes = [_]u8{ 0x0, @enumToInt(command) };

    const devs = c.hid_enumerate(0, 0);
    var list_head = devs;
    while (list_head) |item| {
        if (item.*.product_id == 0x11a1 and
            item.*.vendor_id == 0x2e5 and
            item.*.usage == 0x61)
        {
            break;
        }
        list_head = list_head.*.next;
    }

    if (list_head != null) {
        var name = wcstombs(&allocator, list_head.*.product_string).?;
        std.log.info("{s}", .{name});
        var handle = c.hid_open(list_head.*.vendor_id, list_head.*.product_id, null);
        var counter: u16 = 0;

        while (handle == null) {
            handle = c.hid_open(list_head.*.vendor_id, list_head.*.product_id, null);
            counter += 1;
            if (counter > 1000) break;
        }

        if (handle) |good| {
            _ = c.hid_write(good, &bytes, 2);
        }
    } else {
        std.log.info("couldn't find device", .{});
    }
}

test "basic test" {
    try std.testing.expectEqual(10, 3 + 7);
}
