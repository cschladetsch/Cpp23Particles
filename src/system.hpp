// system.hpp - Updated spatial grid implementation
#pragma once
#include "particle.hpp"
#include "emitter.hpp"
#include <vector>
#include <thread>
#include <barrier> // C++20 feature
#include <atomic>

struct ForceField {
    float x, y;          // Position
    float radius;        // Radius of effect
    float strength;      // Force strength (positive attracts, negative repels)
    bool active = true;  // Whether force field is active
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    std::vector<Emitter> emitters;
    std::vector<ForceField> force_fields;
    
    // Spatial partitioning - optimized implementation
    const float CELL_SIZE = 30.0f;
    const size_t MAX_PARTICLES_PER_CELL = 64;
    int GRID_WIDTH;
    int GRID_HEIGHT;
    std::vector<std::vector<size_t>> spatial_grid;
    bool particle_interaction_enabled = true;
    
    // Multithreading
    std::vector<std::jthread> worker_threads; // C++20 auto-joining threads
    std::barrier<> sync_point;
    std::atomic<bool> running{true};
    std::atomic<float> current_dt{0.0f};
    
public:
    ParticleSystem(size_t max_particles = 10000, 
                   unsigned int thread_count = std::thread::hardware_concurrency(),
                   int screen_width = 1280,
                   int screen_height = 720);
    ~ParticleSystem();
    
    void update(float dt);
    void render(SDL_Renderer* renderer);
    void reset();
    
    // Emitter management
    size_t addEmitter(const EmitterSettings& settings);
    void removeEmitter(size_t index);
    
    // Force field management
    size_t addForceField(float x, float y, float radius, float strength);
    void removeForceField(size_t index);
    void updateForceField(size_t index, float x, float y);
    float getForceFieldStrength(size_t index) const;
    
    // Particle interaction controls
    void toggleParticleInteraction(bool enabled) { particle_interaction_enabled = enabled; }
    bool isParticleInteractionEnabled() const { return particle_interaction_enabled; }
    
private:
    void workerFunction(unsigned int id, unsigned int thread_count);
    void applyGlobalForces(Particle& particle);
    void updateSpatialGrid();
    
    // Helper for spatial grid
    inline size_t getCellIndex(int x, int y) const {
        // Clamp to valid grid coordinates
        x = std::max(0, std::min(x, GRID_WIDTH - 1));
        y = std::max(0, std::min(y, GRID_HEIGHT - 1));
        return static_cast<size_t>(y * GRID_WIDTH + x);
    }
};
// src/system.hpp
