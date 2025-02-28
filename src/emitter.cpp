#include "emitter.hpp"
#include <cmath>

Emitter::Emitter(const EmitterSettings& settings)
    : settings(settings), rng(std::random_device{}())
{
}

int Emitter::update(float dt, std::vector<Particle>& particles) {
    time_accumulator += dt;
    
    // Calculate number of particles to emit
    float particles_to_emit = settings.rate * dt + time_accumulator;
    int whole_particles = static_cast<int>(particles_to_emit);
    time_accumulator = particles_to_emit - whole_particles;
    
    int emitted = 0;
    
    // Emit particles
    for (int i = 0; i < whole_particles; ++i) {
        // Find an inactive particle
        for (auto& p : particles) {
            if (!p.active) {
                emitParticle(p);
                
                // Apply any modifiers
                for (const auto& modifier : modifiers) {
                    modifier(p);
                }
                
                emitted++;
                break;
            }
        }
    }
    
    return emitted;
}

void Emitter::setPosition(float x, float y) {
    settings.x = x;
    settings.y = y;
}

void Emitter::addModifier(ParticleModifier modifier) {
    modifiers.push_back(std::move(modifier));
}

void Emitter::emitParticle(Particle& particle) {
    // Reset particle
    particle.active = true;
    particle.lifetime = settings.particle_lifetime;
    particle.max_lifetime = settings.particle_lifetime;
    particle.size = settings.particle_size;
    particle.colorful_mode = settings.colorful_mode;
    
    // Random colors
    std::uniform_int_distribution<uint32_t> r_dist(settings.min_r, settings.max_r);
    std::uniform_int_distribution<uint32_t> g_dist(settings.min_g, settings.max_g);
    std::uniform_int_distribution<uint32_t> b_dist(settings.min_b, settings.max_b);
    std::uniform_int_distribution<uint32_t> a_dist(settings.min_a, settings.max_a);
    
    particle.r = static_cast<uint8_t>(r_dist(rng));
    particle.g = static_cast<uint8_t>(g_dist(rng));
    particle.b = static_cast<uint8_t>(b_dist(rng));
    particle.a = static_cast<uint8_t>(a_dist(rng));
    
    // Position and velocity based on emitter type
    switch (settings.type) {
        case EmitterType::Point: {
            particle.x = settings.x;
            particle.y = settings.y;
            
            // Random direction
            std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
            float angle = angle_dist(rng);
            particle.vx = std::cos(angle) * settings.particle_speed;
            particle.vy = std::sin(angle) * settings.particle_speed;
            break;
        }
        
        case EmitterType::Circle: {
            // Random position on circle
            std::uniform_real_distribution<float> angle_dist(0.0f, 2.0f * M_PI);
            float angle = angle_dist(rng);
            
            // Random radius
            std::uniform_real_distribution<float> radius_dist(0.0f, 50.0f);
            float radius = radius_dist(rng);
            
            particle.x = settings.x + std::cos(angle) * radius;
            particle.y = settings.y + std::sin(angle) * radius;
            
            // Velocity outward from center
            particle.vx = std::cos(angle) * settings.particle_speed;
            particle.vy = std::sin(angle) * settings.particle_speed;
            break;
        }
        
        case EmitterType::Line: {
            // Random position on line (horizontal line by default)
            std::uniform_real_distribution<float> pos_dist(-100.0f, 100.0f);
            particle.x = settings.x + pos_dist(rng);
            particle.y = settings.y;
            
            // Upward velocity with some randomness
            std::uniform_real_distribution<float> vel_dist(-0.2f, 0.2f);
            particle.vx = vel_dist(rng) * settings.particle_speed;
            particle.vy = -1.0f * settings.particle_speed;
            break;
        }
        
        case EmitterType::Spiral: {
            // Spiral emitter - particles follow a spiral pattern
            std::uniform_real_distribution<float> radius_dist(5.0f, 20.0f);
            std::uniform_real_distribution<float> angle_speed_dist(2.0f, 5.0f);
            
            float angle = settings.spiral_angle;
            float radius = radius_dist(rng) + settings.spiral_radius;
            float angle_speed = angle_speed_dist(rng);
            
            // Position on spiral
            particle.x = settings.x + std::cos(angle) * radius;
            particle.y = settings.y + std::sin(angle) * radius;
            
            // Velocity tangent to spiral
            particle.vx = (-std::sin(angle) * angle_speed + std::cos(angle) * 0.5f) * settings.particle_speed;
            particle.vy = (std::cos(angle) * angle_speed + std::sin(angle) * 0.5f) * settings.particle_speed;
            
            // Update spiral parameters
            settings.spiral_angle += 0.1f;
            settings.spiral_radius += 0.05f;
            if (settings.spiral_radius > 100.0f) settings.spiral_radius = 5.0f;
            break;
        }
    }
    
    // Initialize acceleration
    particle.ax = 0.0f;
    particle.ay = 0.0f;
}
