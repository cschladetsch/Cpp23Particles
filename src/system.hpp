#pragma once
#include "particle.hpp"
#include "emitter.hpp"
#include <vector>
#include <thread>
#include <barrier> // C++20 feature
#include <atomic>
#include <unordered_map>
#include <utility> // for std::pair

// Hash function for grid cell coordinates
struct CellHash {
    size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

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
    
    // Spatial partitioning for fast particle interaction
    std::unordered_map<std::pair<int, int>, std::vector<size_t>, CellHash> spatial_grid;
    float interaction_cell_size = 30.0f;  // Size of each spatial grid cell
    bool particle_interaction_enabled = true;
    
    // Multithreading
    std::vector<std::jthread> worker_threads; // C++20 auto-joining threads
    std::barrier<> sync_point;
    std::atomic<bool> running{true};
    std::atomic<float> current_dt{0.0f};
    
public:
    ParticleSystem(size_t max_particles = 10000, 
                   unsigned int thread_count = std::thread::hardware_concurrency());
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
};
