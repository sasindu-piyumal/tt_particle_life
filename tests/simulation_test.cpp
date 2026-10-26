#include "simulation.hpp"

#include <cmath>
#include <iostream>

namespace {

bool close(double left, double right, double tolerance = 1.0e-12) {
    return std::abs(left - right) <= tolerance;
}

int test_wrap() {
    // Test that wrap() properly wraps values to the range [0, span)
    const double width = particle_life::Simulation::width;
    const double height = particle_life::Simulation::height;
    
    // Test case 1: value already in range
    double result = particle_life::Simulation::wrap(50.0, width);
    if (result < 0.0 || result >= width) {
        std::cerr << "wrap() failed for value in range: " << result << '\n';
        return 1;
    }
    
    // Test case 2: value just at boundary
    result = particle_life::Simulation::wrap(160.0, width);
    if (result < 0.0 || result >= width) {
        std::cerr << "wrap() failed for value at boundary: " << result << '\n';
        return 1;
    }
    
    // Test case 3: large positive value (multiple wraps)
    result = particle_life::Simulation::wrap(330.0, width);
    if (result < 0.0 || result >= width) {
        std::cerr << "wrap() failed for large positive value: " << result << '\n';
        return 1;
    }
    
    // Test case 4: large negative value (multiple wraps)
    result = particle_life::Simulation::wrap(-200.0, width);
    if (result < 0.0 || result >= width) {
        std::cerr << "wrap() failed for large negative value: " << result << '\n';
        return 1;
    }
    
    // Test case 5: wrapped_delta for large positive delta
    result = particle_life::Simulation::wrapped_delta(150.0, width);
    if (std::abs(result) > width * 0.5) {
        std::cerr << "wrapped_delta() failed for large positive delta: " << result << '\n';
        return 1;
    }
    
    return 0;
}

}  // namespace

int main() {
    if (test_wrap() != 0) {
        return 1;
    }
    
    particle_life::Simulation first(128, 42);
    particle_life::Simulation second(128, 42);

    for (int step = 0; step < 20; ++step) {
        first.step();
        second.step();
    }

    const auto& first_particles = first.particles();
    const auto& second_particles = second.particles();
    if (first_particles.size() != 128 || second_particles.size() != 128) {
        std::cerr << "particle count changed\n";
        return 1;
    }

    for (std::size_t i = 0; i < first_particles.size(); ++i) {
        const auto& left = first_particles[i];
        const auto& right = second_particles[i];
        if (!close(left.x, right.x) || !close(left.y, right.y) || !close(left.vx, right.vx) ||
            !close(left.vy, right.vy) || left.species != right.species) {
            std::cerr << "simulation is not deterministic at particle " << i << '\n';
            return 1;
        }
        if (left.x < 0.0 || left.x >= particle_life::Simulation::width || left.y < 0.0 ||
            left.y >= particle_life::Simulation::height || !std::isfinite(left.vx) ||
            !std::isfinite(left.vy)) {
            std::cerr << "particle escaped simulation bounds at index " << i << '\n';
            return 1;
        }
    }

    const double result = first.checksum();
    constexpr double expected_checksum = 610659.60219911579;
    if (!std::isfinite(result) || !close(result, expected_checksum, 1.0e-6)) {
        std::cerr.precision(17);
        std::cerr << "semantic checksum changed: expected " << expected_checksum << ", got " << result
                  << '\n';
        return 1;
    }

    std::cout.precision(17);
    std::cout << "deterministic checksum: " << result << '\n';
    return 0;
}
