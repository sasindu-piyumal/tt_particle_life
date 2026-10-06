#!/usr/bin/env python3

import argparse
import subprocess
import sys
from pathlib import Path


def positive_int(value):
    parsed = int(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be positive")
    return parsed


def positive_float(value):
    parsed = float(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be positive")
    return parsed


def main():
    project = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description="Benchmark the Particle Life simulation")
    parser.add_argument("--particles", type=positive_int, default=2200)
    parser.add_argument("--frames", type=positive_int, default=30)
    parser.add_argument("--visualize", dest="visualize", action="store_true", default=True)
    parser.add_argument("--no-visualize", dest="visualize", action="store_false")
    parser.add_argument("--duration", type=positive_float, default=10.0)
    parser.add_argument("--visual-particles", type=positive_int, default=3500)
    args = parser.parse_args()

    binary = project / "build" / "particle_life"
    if not binary.is_file():
        parser.error("build/particle_life does not exist; run the compile command first")

    results = project / "artemis_results.json"
    results.unlink(missing_ok=True)
    subprocess.run(
        [
            str(binary),
            "benchmark",
            str(args.particles),
            str(args.frames),
            str(results),
        ],
        cwd=project,
        check=True,
    )

    if args.visualize:
        visualize = subprocess.run(
            [
                sys.executable,
                str(project / "tools" / "window_renderer.py"),
                "--binary",
                str(binary),
                "--particles",
                str(args.visual_particles),
                "--duration",
                str(args.duration),
            ],
            cwd=project,
        )
        if visualize.returncode != 0:
            print(
                "warning: --visualize pass failed (exit "
                f"{visualize.returncode}); measured results above are unaffected "
                "(likely no graphical display available, e.g. a headless runner)",
                file=sys.stderr,
            )


if __name__ == "__main__":
    main()
