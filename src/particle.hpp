#pragma once
#include <SDL2/SDL.h>
#include <cstdint>

struct Particle {
    float x, y;           // Position
    float vx, vy;         // Velocity
    float ax, ay;         // Acceleration
    float lifetime;       // Current lifetime
    float max_lifetime;   // Maximum lifetime
    float size;           // Particle size
    uint8_t r, g, b, a;   // Color (RGBA)
    bool active = false;  // Whether particle is active
    bool colorful_mode = false; // Rainbow mode
    
    void update(float dt);
    void render(SDL_Renderer* renderer);
    void applyForce(float fx, float fy);
    
private:
    // Helper function for rainbow colors
    void HSVtoRGB(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b);
};
