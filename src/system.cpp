#include "system.hpp"
#include <cmath>
#include <algorithm>

ParticleSystem::ParticleSystem(size_t max_particles, unsigned int thread_count)
    : particles(max_particles), sync_point(thread_count + 1) // +1 for main thread
{
    // Initialize all particles as inactive
    for (auto& p : particles) {
        p.active = false;
    }
    
    // Initialize worker threads
    for (unsigned int i = 0; i < thread_count; ++i) {
        worker_threads.emplace_back([this, i, thread_count]() {
            this->workerFunction(i, thread_count);
        });
    }
}

ParticleSystem::~ParticleSystem() {
    running.store(false);
    // No need to join threads as std::jthread handles it
}

void ParticleSystem::update(float dt) {
    current_dt.store(dt);
    
    // Update spatial grid for particle interaction
    if (particle_interaction_enabled) {
        updateSpatialGrid();
    }
    
    // Emit new particles
    for (auto& emitter : emitters) {
        emitter.update(dt, particles);
    }
    
    // Signal worker threads to start processing
    sync_point.arrive_and_wait();
    
    // Wait for all threads to finish
    sync_point.arrive_and_wait();
}

void ParticleSystem::render(SDL_Renderer* renderer) {
    for (auto& particle : particles) {
        if (particle.active) {
            particle.render(renderer);
        }
    }
}

void ParticleSystem::reset() {
    // Deactivate all particles
    for (auto& p : particles) {
        p.active = false;
    }
    
    // Clear emitters and force fields
    emitters.clear();
    force_fields.clear();
    
    // Clear spatial grid
    spatial_grid.clear();
}

size_t ParticleSystem::addEmitter(const EmitterSettings& settings) {
    emitters.emplace_back(settings);
    return emitters.size() - 1;
}

void ParticleSystem::removeEmitter(size_t index) {
    if (index < emitters.size()) {
        emitters.erase(emitters.begin() + index);
    }
}

size_t ParticleSystem::addForceField(float x, float y, float radius, float strength) {
    force_fields.push_back({x, y, radius, strength});
    return force_fields.size() - 1;
}

void ParticleSystem::removeForceField(size_t index) {
    if (index < force_fields.size()) {
        force_fields.erase(force_fields.begin() + index);
    }
}

void ParticleSystem::updateForceField(size_t index, float x, float y) {
    if (index < force_fields.size()) {
        force_fields[index].x = x;
        force_fields[index].y = y;
    }
}

float ParticleSystem::getForceFieldStrength(size_t index) const {
    if (index < force_fields.size()) {
        return force_fields[index].strength;
    }
    return 0.0f;
}

void ParticleSystem::workerFunction(unsigned int id, unsigned int thread_count) {
    while (running.load()) {
        // Wait until all threads are ready for next frame
        sync_point.arrive_and_wait();
        
        float dt = current_dt.load();
        
        // Process a subset of particles
        size_t particles_per_thread = particles.size() / thread_count;
        size_t start_idx = id * particles_per_thread;
        size_t end_idx = (id == thread_count - 1) ? 
                         particles.size() : (id + 1) * particles_per_thread;
        
        // Update particles in this thread's range
        for (size_t i = start_idx; i < end_idx; ++i) {
            auto& p = particles[i];
            if (p.active) {
                // Apply forces
                applyGlobalForces(p);
                
                // Update particle physics
                p.update(dt);
            }
        }
        
        // Signal that this thread is done
        sync_point.arrive_and_wait();
    }
}

void ParticleSystem::applyGlobalForces(Particle& particle) {
    // Apply gravity
    particle.applyForce(0.0f, 98.0f);
    
    // Apply force fields
    for (const auto& field : force_fields) {
        if (!field.active) continue;
        
        float dx = field.x - particle.x;
        float dy = field.y - particle.y;
        float dist_sq = dx*dx + dy*dy;
        
        if (dist_sq < field.radius * field.radius && dist_sq > 0.01f) {
            float dist = std::sqrt(dist_sq);
            float force = field.strength / dist;
            particle.applyForce(dx / dist * force, dy / dist * force);
        }
    }
    
    // Apply particle-to-particle interaction
    if (particle_interaction_enabled) {
        // We'll use a grid-based approach for optimization
        int grid_x = static_cast<int>(particle.x / interaction_cell_size);
        int grid_y = static_cast<int>(particle.y / interaction_cell_size);
        
        // Check nearby grid cells
        const float repulsion_radius = 15.0f;  // Max distance for repulsion
        const float repulsion_radius_sq = repulsion_radius * repulsion_radius;
        const float repulsion_strength = 500.0f;  // Strength of repulsion
        
        // Loop through nearby particles
        for (int y = grid_y - 1; y <= grid_y + 1; y++) {
            for (int x = grid_x - 1; x <= grid_x + 1; x++) {
                auto cell_iter = spatial_grid.find(std::make_pair(x, y));
                if (cell_iter == spatial_grid.end()) continue;
                
                // Check particles in this cell
                for (const auto& other_idx : cell_iter->second) {
                    Particle& other = particles[other_idx];
                    if (!other.active || &other == &particle) continue;
                    
                    float dx = particle.x - other.x;
                    float dy = particle.y - other.y;
                    float dist_sq = dx*dx + dy*dy;
                    
                    // Apply repulsion force if particles are close enough
                    if (dist_sq < repulsion_radius_sq && dist_sq > 0.01f) {
                        float dist = std::sqrt(dist_sq);
                        float force = repulsion_strength * (1.0f - dist/repulsion_radius) / dist;
                        
                        particle.applyForce(dx * force, dy * force);
                    }
                }
            }
        }
    }
}

void ParticleSystem::updateSpatialGrid() {
    // Clear the grid
    spatial_grid.clear();
    
    // Add active particles to the grid
    for (size_t i = 0; i < particles.size(); ++i) {
        const auto& p = particles[i];
        if (p.active) {
            int grid_x = static_cast<int>(p.x / interaction_cell_size);
            int grid_y = static_cast<int>(p.y / interaction_cell_size);
            spatial_grid[std::make_pair(grid_x, grid_y)].push_back(i);
        }
    }
}
