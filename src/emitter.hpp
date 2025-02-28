#pragma once
#include "particle.hpp"
#include <random>
#include <vector>
#include <functional>

enum class EmitterType {
    Point,
    Circle,
    Line,
    Spiral
};

struct EmitterSettings {
    float x, y;              // Position
    float rate;              // Particles per second
    float particle_speed;    // Initial particle speed
    float particle_size;     // Size of particles
    float particle_lifetime; // Lifetime in seconds
    EmitterType type;        // Type of emitter
    
    // Color range
    uint8_t min_r, max_r;
    uint8_t min_g, max_g;
    uint8_t min_b, max_b;
    uint8_t min_a, max_a;
    
    // Special effects
    bool colorful_mode = false;  // Rainbow mode
    
    // Spiral emitter parameters
    float spiral_angle = 0.0f;
    float spiral_radius = 5.0f;
};

using ParticleModifier = std::function<void(Particle&)>;

class Emitter {
private:
    EmitterSettings settings;
    float time_accumulator = 0.0f;
    std::mt19937 rng;
    std::vector<ParticleModifier> modifiers;
    
public:
    Emitter(const EmitterSettings& settings);
    
    // Returns number of particles emitted
    int update(float dt, std::vector<Particle>& particles);
    
    void setPosition(float x, float y);
    void addModifier(ParticleModifier modifier);
    
private:
    void emitParticle(Particle& particle);
};
