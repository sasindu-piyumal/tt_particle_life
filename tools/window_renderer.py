#!/usr/bin/env python3

import argparse
import struct
import subprocess
import time
import tkinter as tk
from pathlib import Path


PALETTE = ("#4deeea", "#b86bff", "#ffe66d", "#70e000", "#ff4d6d")
HEADER = struct.Struct("<4sIff")
FRAME_NUMBER = struct.Struct("<Q")
PARTICLE = struct.Struct("<ffB")


def read_exact(stream, size):
    data = bytearray()
    while len(data) < size:
        chunk = stream.read(size - len(data))
        if not chunk:
            raise EOFError("simulation stream ended")
        data.extend(chunk)
    return bytes(data)


class WindowRenderer:
    def __init__(self, binary, particles, width, height, duration):
        self.process = subprocess.Popen(
            [str(binary), "stream", str(particles), "100000000"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
        )
        assert self.process.stdout is not None

        magic, self.count, self.world_width, self.world_height = HEADER.unpack(
            read_exact(self.process.stdout, HEADER.size)
        )
        if magic != b"SWAR":
            raise RuntimeError("unexpected simulation stream")

        self.frame_size = FRAME_NUMBER.size + self.count * PARTICLE.size
        self.width = width
        self.height = height
        self.duration = duration
        self.started = None
        self.frames_drawn = 0
        self.closed = False

        self.root = tk.Tk()
        self.root.title("Particle Life")
        self.root.configure(bg="#080b16")
        self.root.protocol("WM_DELETE_WINDOW", self.close)

        self.canvas = tk.Canvas(
            self.root,
            width=width,
            height=height,
            bg="#080b16",
            highlightthickness=0,
        )
        self.canvas.pack()

        self.status = tk.Label(
            self.root,
            text="Preparing the particles...",
            bg="#080b16",
            fg="#d8f3ff",
            font=("TkFixedFont", 11),
            padx=10,
            pady=7,
        )
        self.status.pack(fill="x")

        radius = 2.4
        self.items = [
            self.canvas.create_oval(
                -radius,
                -radius,
                radius,
                radius,
                fill=PALETTE[index % len(PALETTE)],
                outline="",
            )
            for index in range(self.count)
        ]
        self.radius = radius
        self.root.after(1, self.draw_next)

    def draw_next(self):
        try:
            frame = read_exact(self.process.stdout, self.frame_size)
        except EOFError:
            self.close()
            return

        frame_number = FRAME_NUMBER.unpack_from(frame, 0)[0]
        offset = FRAME_NUMBER.size
        scale_x = self.width / self.world_width
        scale_y = self.height / self.world_height

        for item in self.items:
            x, y, _species = PARTICLE.unpack_from(frame, offset)
            offset += PARTICLE.size
            screen_x = x * scale_x
            screen_y = y * scale_y
            radius = self.radius
            self.canvas.coords(
                item,
                screen_x - radius,
                screen_y - radius,
                screen_x + radius,
                screen_y + radius,
            )

        self.frames_drawn += 1
        now = time.perf_counter()
        if self.started is None:
            self.started = now
            self.root.after(round(self.duration * 1000), self.close)
        elapsed = now - self.started
        display_fps = self.frames_drawn / elapsed if elapsed else 0.0
        remaining = max(0.0, self.duration - elapsed)
        self.status.configure(
            text=(
                f"frame {frame_number:7d}    particles {self.count:5d}    "
                f"display {display_fps:5.1f} fps    {remaining:4.1f}s remaining"
            )
        )
        self.root.after(1, self.draw_next)

    def close(self):
        if self.closed:
            return
        self.closed = True
        if self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=1)
            except subprocess.TimeoutExpired:
                self.process.kill()
        self.root.destroy()

    def run(self):
        self.root.mainloop()


def main():
    project = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Render Particle Life in a Tk window")
    parser.add_argument("--binary", type=Path, default=project / "build" / "particle_life")
    parser.add_argument("--particles", type=int, default=3500)
    parser.add_argument("--duration", type=float, default=10.0)
    parser.add_argument("--width", type=int, default=1100)
    parser.add_argument("--height", type=int, default=620)
    args = parser.parse_args()

    if args.particles <= 0:
        parser.error("--particles must be positive")
    if args.duration <= 0:
        parser.error("--duration must be positive")
    if not args.binary.is_file():
        parser.error(f"simulation binary not found: {args.binary}")

    WindowRenderer(
        args.binary,
        args.particles,
        args.width,
        args.height,
        args.duration,
    ).run()


if __name__ == "__main__":
    main()
