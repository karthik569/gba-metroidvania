#!/usr/bin/env python3
"""
AERO-VOID: Outpost Zero - Host Debug Logger & Telemetry Monitor
Extracts live/persistent debug logs and game save progress from GBA emulator .sav files.
"""

import os
import sys
import time
import struct
import argparse
from typing import Optional, Tuple

SRAM_SAVE_OFFSET = 0x0000
SRAM_SAVE_MAGIC = b"AEROSAVE"

SRAM_LOG_OFFSET = 0x0200
SRAM_LOG_MAGIC = b"AEROLOG1"
HEADER_SIZE = 24  # 8 bytes magic + 4*4 bytes u32

ROOM_NAMES = {
    0: "Landing Dock",
    1: "Airflow Shaft",
    2: "Security Armory",
    3: "Power Conduit",
    4: "Hub Corridor",
    5: "Sentinel Boss Arena",
    6: "Save Station"
}

# ANSI color codes for terminal display
COLORS = {
    "FATAL": "\033[95m", # Magenta
    "ERROR": "\033[91m", # Red
    "WARN ": "\033[93m", # Yellow
    "INFO ": "\033[96m", # Cyan
    "DEBUG": "\033[90m", # Gray
    "RESET": "\033[0m",
    "BOLD":  "\033[1m",
    "GREEN": "\033[92m"
}

def colorize(line: str) -> str:
    for lvl, code in COLORS.items():
        if f"[{lvl}]" in line:
            return f"{code}{line}{COLORS['RESET']}"
    return line

def read_save_data(sav_path: str) -> Optional[dict]:
    if not os.path.exists(sav_path):
        return None
    try:
        with open(sav_path, "rb") as f:
            f.seek(SRAM_SAVE_OFFSET)
            header = f.read(32)
            if len(header) < 32:
                return None
            magic = header[0:8]
            if magic != SRAM_SAVE_MAGIC:
                return None
            
            version, hp, max_hp, ms, max_ms, has_ms, room, x, y, frames, chk = struct.unpack_from(
                "<HhhhhBBhhIH", header, 8
            )
            return {
                "version": version,
                "health": hp,
                "max_health": max_hp,
                "missiles": ms,
                "max_missiles": max_ms,
                "has_missiles": bool(has_ms),
                "checkpoint_room": room,
                "room_name": ROOM_NAMES.get(room, f"Room {room}"),
                "checkpoint_x": x,
                "checkpoint_y": y,
                "playtime_frames": frames,
                "checksum": chk
            }
    except Exception as e:
        return None

def read_log_buffer(sav_path: str) -> Tuple[Optional[dict], str]:
    if not os.path.exists(sav_path):
        return None, ""
    try:
        with open(sav_path, "rb") as f:
            f.seek(SRAM_LOG_OFFSET)
            hdr_bytes = f.read(HEADER_SIZE)
            if len(hdr_bytes) < HEADER_SIZE:
                return None, ""
            
            magic, write_offset, entry_count, wrap_count, capacity = struct.unpack("<8sIIII", hdr_bytes)
            if magic != SRAM_LOG_MAGIC:
                return None, ""
            
            if capacity == 0 or capacity > 65536:
                capacity = 32232
            
            data_bytes = f.read(capacity)
            header_info = {
                "write_offset": write_offset,
                "entry_count": entry_count,
                "wrap_count": wrap_count,
                "capacity": capacity
            }

            if wrap_count == 0:
                raw_log = data_bytes[:write_offset]
            else:
                raw_log = data_bytes[write_offset:capacity] + data_bytes[:write_offset]

            text = raw_log.decode("ascii", errors="replace")
            # If wrapped, discard partial first line if not starting with '['
            if wrap_count > 0 and text and not text.startswith("["):
                first_nl = text.find("\n")
                if first_nl != -1:
                    text = text[first_nl + 1:]
            return header_info, text
    except Exception as e:
        return None, ""

def dump_logs(sav_path: str, out_path: str, print_to_stdout: bool = True) -> int:
    header, log_text = read_log_buffer(sav_path)
    if not header:
        print(f"[!] No valid AEROLOG1 buffer found in '{sav_path}'. Play the game to generate logs.")
        return 1
    
    lines = [l for l in log_text.splitlines() if l.strip()]
    
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(f"# AERO-VOID: Outpost Zero Debug Log\n")
        f.write(f"# Source: {sav_path}\n")
        f.write(f"# Entries: {header['entry_count']} (Wrapped: {header['wrap_count']} times)\n")
        f.write(f"# -------------------------------------------------------------\n")
        for line in lines:
            f.write(line + "\n")
            if print_to_stdout:
                print(colorize(line))
                
    print(f"\n{COLORS['GREEN']}✓ Successfully dumped {len(lines)} log lines to '{out_path}'{COLORS['RESET']}")
    return 0

def show_info(sav_path: str):
    print(f"{COLORS['BOLD']}=== Outpost Zero SRAM Telemetry: {sav_path} ==={COLORS['RESET']}")
    save_data = read_save_data(sav_path)
    if save_data:
        print(f"\n{COLORS['GREEN']}🎮 Active Game Save Progress:{COLORS['RESET']}")
        print(f"  • Room:         {save_data['checkpoint_room']} ({save_data['room_name']})")
        print(f"  • Spawn Pos:    ({save_data['checkpoint_x']}, {save_data['checkpoint_y']})")
        print(f"  • Health:       {save_data['health']} / {save_data['max_health']}")
        print(f"  • Missiles:     {save_data['missiles']} / {save_data['max_missiles']} (Unlocked: {save_data['has_missiles']})")
        print(f"  • Playtime:     {save_data['playtime_frames']} frames (~{save_data['playtime_frames'] // 60}s)")
    else:
        print(f"\n  • Save Slot:    Empty / No save data recorded yet")
    
    header, log_text = read_log_buffer(sav_path)
    if header:
        lines = [l for l in log_text.splitlines() if l.strip()]
        print(f"\n{COLORS['GREEN']}📜 Debug Log Ring Buffer:{COLORS['RESET']}")
        print(f"  • Total Entries:{header['entry_count']}")
        print(f"  • Wrap Count:   {header['wrap_count']}")
        print(f"  • Buffer Size:  {header['write_offset']} / {header['capacity']} bytes")
        print(f"  • Active Lines: {len(lines)}")
        if lines:
            print(f"\n  Recent entry:   {lines[-1]}")
    else:
        print(f"\n  • Log Buffer:   Not initialized")

def watch_logs(sav_path: str, out_path: str, poll_interval: float = 0.5):
    print(f"{COLORS['BOLD']}🚀 Monitoring '{sav_path}' in real-time... (Press Ctrl+C to stop){COLORS['RESET']}")
    print(f"Logs will stream below and append to '{out_path}'.\n")
    
    last_mtime = 0.0
    last_entries = 0
    
    out_file = open(out_path, "a", encoding="utf-8")
    
    try:
        while True:
            if os.path.exists(sav_path):
                mtime = os.path.getmtime(sav_path)
                if mtime > last_mtime:
                    last_mtime = mtime
                    header, log_text = read_log_buffer(sav_path)
                    if header and header['entry_count'] > last_entries:
                        lines = [l for l in log_text.splitlines() if l.strip()]
                        new_count = header['entry_count'] - last_entries
                        # Print only the newest lines
                        new_lines = lines[-new_count:] if new_count <= len(lines) else lines
                        for line in new_lines:
                            print(colorize(line))
                            out_file.write(line + "\n")
                        out_file.flush()
                        last_entries = header['entry_count']
            time.sleep(poll_interval)
    except KeyboardInterrupt:
        print("\n[!] Watcher stopped.")
    finally:
        out_file.close()

def main():
    parser = argparse.ArgumentParser(description="AERO-VOID: Outpost Zero GBA Debug Logger")
    parser.add_argument("save_file", nargs="?", default="outpost_zero.sav", help="Path to .sav file (default: outpost_zero.sav)")
    parser.add_argument("--dump", action="store_true", help="Dump all logs chronologically to file and console")
    parser.add_argument("--watch", action="store_true", help="Live tail the .sav file while playing")
    parser.add_argument("--info", action="store_true", help="Display SRAM telemetry (save slot progress and buffer statistics)")
    parser.add_argument("--out", default="outpost_zero.log", help="Output log text file path (default: outpost_zero.log)")
    args = parser.parse_args()

    # Default action if no flags provided: dump
    if not (args.dump or args.watch or args.info):
        args.dump = True

    if args.info:
        show_info(args.save_file)
    elif args.watch:
        watch_logs(args.save_file, args.out)
    elif args.dump:
        dump_logs(args.save_file, args.out)

if __name__ == "__main__":
    sys.exit(main())
