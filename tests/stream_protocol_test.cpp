// Exercises `particle_life stream`, the binary frame protocol that
// tools/window_renderer.py parses. Nothing previously checked this format, so
// a change to the header layout, frame layout, or particle ordering could
// silently break the renderer without any test failing.

// Must come before any system header so glibc declares popen()/pclose()
// under the strict (non-gnu) C++17 standard this project builds with.
#define _POSIX_C_SOURCE 200809L

#include "simulation.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#ifndef STREAM_BINARY_PATH
#error "STREAM_BINARY_PATH must be defined to the particle_life binary path"
#endif

namespace {

bool read_exact(FILE* stream, void* buffer, std::size_t size) {
    return std::fread(buffer, 1, size, stream) == size;
}

}  // namespace

int main() {
    constexpr std::size_t particle_count = 40;
    constexpr std::size_t frame_count = 5;
    constexpr float bounds_epsilon = 0.01f;

    const std::string command = std::string("\"") + STREAM_BINARY_PATH + "\" stream " +
                                 std::to_string(particle_count) + " " + std::to_string(frame_count);
    FILE* stream = popen(command.c_str(), "r");
    if (!stream) {
        std::cerr << "failed to launch stream process\n";
        return 1;
    }

    std::array<char, 4> magic{};
    std::uint32_t count = 0;
    float world_width = 0.0f;
    float world_height = 0.0f;
    const bool header_ok = read_exact(stream, magic.data(), magic.size()) &&
                            read_exact(stream, &count, sizeof(count)) &&
                            read_exact(stream, &world_width, sizeof(world_width)) &&
                            read_exact(stream, &world_height, sizeof(world_height));

    if (!header_ok || std::memcmp(magic.data(), "SWAR", magic.size()) != 0) {
        std::cerr << "did not receive a valid stream header\n";
        pclose(stream);
        return 1;
    }
    if (count != particle_count) {
        std::cerr << "header particle count mismatch: expected " << particle_count << ", got " << count
                   << '\n';
        pclose(stream);
        return 1;
    }
    if (world_width != static_cast<float>(particle_life::Simulation::width) ||
        world_height != static_cast<float>(particle_life::Simulation::height)) {
        std::cerr << "header world size mismatch: got " << world_width << "x" << world_height << '\n';
        pclose(stream);
        return 1;
    }

    std::uint64_t previous_frame = 0;
    for (std::size_t frame_index = 0; frame_index < frame_count; ++frame_index) {
        std::uint64_t frame_number = 0;
        if (!read_exact(stream, &frame_number, sizeof(frame_number))) {
            std::cerr << "stream ended early before frame " << frame_index << '\n';
            pclose(stream);
            return 1;
        }
        if (frame_index == 0 ? frame_number != 0 : frame_number != previous_frame + 1) {
            std::cerr << "frame numbers are not sequential from zero: got " << frame_number
                       << " after " << previous_frame << '\n';
            pclose(stream);
            return 1;
        }
        previous_frame = frame_number;

        for (std::uint32_t particle_index = 0; particle_index < count; ++particle_index) {
            float x = 0.0f;
            float y = 0.0f;
            std::uint8_t species = 0;
            if (!read_exact(stream, &x, sizeof(x)) || !read_exact(stream, &y, sizeof(y)) ||
                !read_exact(stream, &species, sizeof(species))) {
                std::cerr << "stream ended early mid-frame " << frame_index << '\n';
                pclose(stream);
                return 1;
            }
            if (!std::isfinite(x) || !std::isfinite(y) || x < -bounds_epsilon ||
                x >= static_cast<float>(particle_life::Simulation::width) + bounds_epsilon ||
                y < -bounds_epsilon ||
                y >= static_cast<float>(particle_life::Simulation::height) + bounds_epsilon) {
                std::cerr << "particle " << particle_index << " out of bounds in frame " << frame_index
                           << ": (" << x << ", " << y << ")\n";
                pclose(stream);
                return 1;
            }
            if (species >= particle_life::Simulation::species_count) {
                std::cerr << "particle " << particle_index << " has invalid species "
                           << static_cast<int>(species) << '\n';
                pclose(stream);
                return 1;
            }
        }
    }

    const int status = pclose(stream);
    if (status != 0) {
        std::cerr << "stream process exited with status " << status << '\n';
        return 1;
    }

    std::cout << "stream protocol ok: " << frame_count << " frames of " << count << " particles\n";
    return 0;
}
