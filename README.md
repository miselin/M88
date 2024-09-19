# The M88 Computer

This repository contains the KiCad project(s) and ROM source code for the M88 computer project.
Learn more about the project at [https://m88.computer](https://m88.computer)

## M88

The top-level KiCad project is the main KiCad project for the M88's Micro-ATX motherboard.

## ISA-to-I2C Bridge

The `isa_i2c` directory contains a KiCad project for an ISA-to-I2C bridge, designed around
readily available parts with only one SMT IC.

## ISA Break-out

The `isa_breakout` directory contains a KiCad project for an ISA break-out card that makes
connecting ISA bus lines to a logic analyzer much easier.

## Programs

The `programs` directory contains the main BIOS ROM for the M88, as well as utilities for
building it. The M88 BIOS ROM is _not_ fully PC-compatible and it is not recommended to use it
on other systems, at this stage.

## Licensing

The computer design and schematics are licensed as CC BY-SA 4.0.

The software under the `programs` directory is licensed under the MIT license.
