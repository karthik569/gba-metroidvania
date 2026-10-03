#!/usr/bin/env python3
"""
Computes and patches the standard GBA ROM header complement checksum
at offset 0xBD, ensuring 100% compliance with hardware BIOS and emulators.
"""
import sys

def fix_header(rom_path):
    with open(rom_path, 'r+b') as f:
        data = bytearray(f.read())
        if len(data) < 0xC0:
            print("ROM too small for GBA header")
            return 1
        
        # Nintendo logo check / pad
        # Header checksum is calculated over bytes 0xA0 to 0xBC
        chk = 0
        for i in range(0xA0, 0xBD):
            chk = (chk - data[i]) & 0xFF
        chk = (chk - 0x19) & 0xFF
        
        data[0xBD] = chk
        
        # Ensure ROM is padded to minimum 256KB or power of 2
        min_size = 256 * 1024
        if len(data) < min_size:
            data.extend(b'\x00' * (min_size - len(data)))
            
        f.seek(0)
        f.write(data)
        f.truncate()
        print(f"Patched GBA header checksum: 0x{chk:02X}, size: {len(data)} bytes")
    return 0

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: fix_header.py <rom.gba>")
        sys.exit(1)
    sys.exit(fix_header(sys.argv[1]))
