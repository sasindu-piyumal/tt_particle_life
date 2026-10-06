#include "simulation.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::size_t parse_count(const char* value, const char* name) {
    try {
        const auto parsed = std::stoull(value);
        if (parsed == 0) {
            throw std::invalid_argument("zero");
        }
        return static_cast<std::size_t>(parsed);
    } catch (const std::exception&) {
        throw std::runtime_error(std::string(name) + " must be a positive integer");
    }
}

int benchmark(std::size_t particle_count, std::size_t frame_count, const std::string& output_path) {
    particle_life::Simulation simulation(particle_count, 20260724);
    constexpr std::size_t warmup_frames = 5;
    for (std::size_t frame = 0; frame < warmup_frames; ++frame) {
        simulation.step();
    }

    const auto start = std::chrono::steady_clock::now();
    for (std::size_t frame = 0; frame < frame_count; ++frame) {
        simulation.step();
    }
    const auto end = std::chrono::steady_clock::now();

    const double elapsed_seconds = std::chrono::duration<double>(end - start).count();
    const double fps = static_cast<double>(frame_count) / elapsed_seconds;
    const double frame_ms = elapsed_seconds * 1000.0 / static_cast<double>(frame_count);

    std::ofstream output(output_path);
    if (!output) {
        throw std::runtime_error("could not open results file: " + output_path);
    }
    output << std::setprecision(12) << "{\"simulation_fps\":" << fps << "}\n";

    std::cout << std::fixed << std::setprecision(3)
              << "particles=" << particle_count << " frames=" << frame_count
              << " fps=" << fps << " frame_ms=" << frame_ms
              << " checksum=" << simulation.checksum() << '\n'
              << "wrote " << output_path << '\n';
    return 0;
}

void stream(std::size_t particle_count, std::size_t frame_count) {
    particle_life::Simulation simulation(particle_count, 20260724);
    const std::array<char, 4> magic = {'S', 'W', 'A', 'R'};
    const auto count = static_cast<std::uint32_t>(particle_count);
    const float world_width = static_cast<float>(particle_life::Simulation::width);
    const float world_height = static_cast<float>(particle_life::Simulation::height);
    std::cout.write(magic.data(), static_cast<std::streamsize>(magic.size()));
    std::cout.write(reinterpret_cast<const char*>(&count), sizeof(count));
    std::cout.write(reinterpret_cast<const char*>(&world_width), sizeof(world_width));
    std::cout.write(reinterpret_cast<const char*>(&world_height), sizeof(world_height));
    std::cout.flush();

    constexpr std::size_t settling_steps = 25;
    for (std::size_t step = 0; step < settling_steps; ++step) {
        simulation.step();
    }

    for (std::uint64_t frame = 0; frame < frame_count; ++frame) {
        simulation.step();
        std::cout.write(reinterpret_cast<const char*>(&frame), sizeof(frame));
        for (const auto& particle : simulation.particles()) {
            const float x = static_cast<float>(particle.x);
            const float y = static_cast<float>(particle.y);
            std::cout.write(reinterpret_cast<const char*>(&x), sizeof(x));
            std::cout.write(reinterpret_cast<const char*>(&y), sizeof(y));
            std::cout.write(reinterpret_cast<const char*>(&particle.species), sizeof(particle.species));
        }
        std::cout.flush();
    }
}

void usage(const char* program) {
    std::cerr << "Usage:\n"
              << "  " << program << " benchmark [particles=2200] [frames=30] [results-path]\n"
              << "  " << program << " stream [particles=3500] [frames=100000000]\n";
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            usage(argv[0]);
            return 2;
        }

        const std::string mode = argv[1];
        if (mode == "benchmark") {
            const auto particles = argc > 2 ? parse_count(argv[2], "particles") : 2200;
            const auto frames = argc > 3 ? parse_count(argv[3], "frames") : 30;
            const std::string output = argc > 4 ? argv[4] : "artemis_results.json";
            return benchmark(particles, frames, output);
        }
        if (mode == "stream") {
            const auto particles = argc > 2 ? parse_count(argv[2], "particles") : 3500;
            const auto frames = argc > 3 ? parse_count(argv[3], "frames") : 100000000;
            stream(particles, frames);
            return 0;
        }

        usage(argv[0]);
        return 2;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
