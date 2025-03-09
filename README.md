# English

# GC24 - An emulator for the Govno Core 24 processor

## Description
A 24-bit address bus. Because why not?\
(Inspired by Ricoh 5A22)

## Usage
To compile everything, run `./ball`. It compiles itself and gc24.\
To compile GovnBIOS & GovnOS, use `./prepare-disk disk.img`. This will install them onto the disk and create it, if it doesn't exist.
You can write programs either in GovnASM (and compile with ./kasm), or
in raw binary format.
To start the emulator, use `./gc24 file.bin`.
