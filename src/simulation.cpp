#include "simulation.hpp"

#include <algorithm>
#include <cmath>
#include <random>

namespace particle_life {
namespace {

constexpr double interaction_radius = 12.0;
constexpr double interaction_radius_sq = interaction_radius * interaction_radius;
constexpr double repulsion_radius = 2.0;
constexpr double force_scale = 0.018;
constexpr double damping = 0.91;
constexpr double max_speed = 1.35;
constexpr double time_step = 0.42;
constexpr double width = Simulation::width;
constexpr double height = Simulation::height;

}  // namespace

Simulation::Simulation(std::size_t particle_count, std::uint32_t seed)
    : particles_(particle_count), next_vx_(particle_count), next_vy_(particle_count) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> x_dist(0.0, width);
    std::uniform_real_distribution<double> y_dist(0.0, height);
    std::uniform_real_distribution<double> velocity_dist(-0.08, 0.08);
    std::uniform_real_distribution<double> affinity_dist(-1.0, 1.0);

    for (std::size_t source = 0; source < species_count; ++source) {
        for (std::size_t target = 0; target < species_count; ++target) {
            affinity_[source][target] = affinity_dist(rng);
        }
        affinity_[source][source] = 0.45 + 0.08 * static_cast<double>(source);
    }

    for (std::size_t i = 0; i < particle_count; ++i) {
        particles_[i] = {
            x_dist(rng),
            y_dist(rng),
            velocity_dist(rng),
            velocity_dist(rng),
            static_cast<std::uint8_t>(i % species_count),
        };
    }
}

double Simulation::wrapped_delta(double delta, double span) {
    if (delta > span * 0.5) {
        return delta - span;
    }
    if (delta < -span * 0.5) {
        return delta + span;
    }
    return delta;
}

double Simulation::wrap(double value, double span) {
    if (value >= span) {
        value -= span;
    } else if (value < 0.0) {
        value += span;
    }
    return value;
}

void Simulation::step() {
    for (std::size_t i = 0; i < particles_.size(); ++i) {
        const Particle& particle = particles_[i];
        double force_x = 0.0;
        double force_y = 0.0;

        for (std::size_t j = 0; j < particles_.size(); ++j) {
            if (i == j) {
                continue;
            }

            const Particle& other = particles_[j];
            const double dx = wrapped_delta(other.x - particle.x, width);
            const double dy = wrapped_delta(other.y - particle.y, height);
            const double distance_sq = dx * dx + dy * dy;

            if (distance_sq <= 1.0e-12 || distance_sq >= interaction_radius_sq) {
                continue;
            }

            const double distance = std::sqrt(distance_sq);
            double strength;
            if (distance < repulsion_radius) {
                strength = distance / repulsion_radius - 1.0;
            } else {
                const double envelope = 1.0 - std::abs(2.0 * distance / interaction_radius - 1.0);
                strength = affinity_[particle.species][other.species] * envelope;
            }

            const double scaled = force_scale * strength / distance;
            force_x += dx * scaled;
            force_y += dy * scaled;
        }

        double vx = (particle.vx + force_x) * damping;
        double vy = (particle.vy + force_y) * damping;
        const double speed_sq = vx * vx + vy * vy;
        if (speed_sq > max_speed * max_speed) {
            const double scale = max_speed / std::sqrt(speed_sq);
            vx *= scale;
            vy *= scale;
        }

        next_vx_[i] = vx;
        next_vy_[i] = vy;
    }

    for (std::size_t i = 0; i < particles_.size(); ++i) {
        Particle& particle = particles_[i];
        particle.vx = next_vx_[i];
        particle.vy = next_vy_[i];
        particle.x = wrap(particle.x + particle.vx * time_step, width);
        particle.y = wrap(particle.y + particle.vy * time_step, height);
    }
}

double Simulation::checksum() const {
    long double result = 0.0;
    for (std::size_t i = 0; i < particles_.size(); ++i) {
        const Particle& particle = particles_[i];
        const long double weight = static_cast<long double>((i % 97) + 1);
        result += weight * (particle.x * 0.7 + particle.y * 1.3 + particle.vx * 2.1 +
                            particle.vy * 2.9 + static_cast<double>(particle.species) * 0.17);
    }
    return static_cast<double>(result);
}

}  // namespace particle_life
