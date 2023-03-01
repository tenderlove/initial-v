<p align="center">
  <img src="artwork/logo.png">
</p>

# Initial V

Initial V is a BMW shifter that has been converted to a Bluetooth keyboard.
In this repository, you'll find [schematics and PCB designs](pcb), [stl files](housing),
[a Vim plugin](vim-plugin), and [client software](client) for turning a BMW
shifter in to a Bluetooth keyboard that can control Vim.

Think of this project as a very over-engineered [Vim clutch](https://github.com/alevchuk/vim-clutch).

## What does it do?

Initial V is a Bluetooth Keyboard specialized for controlling Vim.
The key presses sent depend on Vim's state.
The table below describes the key presses for each handle position according to the state of the editor:

|             |  Park  |  Up  | Down | Double Up | Double Down | Move Left | Left Up | Left Down | Move Right |
|-------      |--------|------|------|-----------|-------------|-----------|---------|-----------|------------|
| Normal Mode | `:w` on a modified buffer, `:wq` on unmodified buffer | Up key | Down key | `i` | `o` | `CTRL-V` | Up Key | Down key | `ESC` |
