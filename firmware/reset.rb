require "uchip/mcp2221"

# Find the first connected chip
chip = UChip::MCP2221.first || raise("Couldn't find the chip!")

p chip.gpio_value 0
p chip.gpio_value 1

chip.set_gpio_value 0, 0
chip.set_gpio_value 1, 0

chip.set_gpio_value 0, 1
chip.set_gpio_value 1, 0

chip.set_gpio_value 0, 0
chip.set_gpio_value 1, 1

chip.set_gpio_value 0, 0
chip.set_gpio_value 1, 0
