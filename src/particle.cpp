#include "particle.hpp"
#include <algorithm>
#include <cmath>

void Particle::update(float dt) {
    if (!active) return;
    
    // Update position and velocity
    vx += ax * dt;
    vy += ay * dt;
    x += vx * dt;
    y += vy * dt;
    
    // Update lifetime
    lifetime -= dt;
    if (lifetime <= 0.0f) {
        active = false;
    }
    
    // Reset acceleration
    ax = 0.0f;
    ay = 0.0f;
}

void Particle::render(SDL_Renderer* renderer) {
    if (!active) return;
    
    // Calculate life progress (0.0 to 1.0)
    float life_ratio = lifetime / max_lifetime;
    
    // Color transition based on lifetime
    uint8_t current_r, current_g, current_b, current_a;
    
    // Transition between colors based on lifetime
    if (colorful_mode) {
        // Rainbow effect - cycle through hue based on lifetime and position
        float hue = fmod(life_ratio * 360.0f + (x + y) * 0.1f, 360.0f);
        HSVtoRGB(hue, 1.0f, 1.0f, current_r, current_g, current_b);
        current_a = static_cast<uint8_t>(a * life_ratio);
    } else {
        // Normal color fade based on initial color
        current_r = r;
        current_g = g;
        current_b = b;
        current_a = static_cast<uint8_t>(a * life_ratio);
    }
    
    // Set draw color
    SDL_SetRenderDrawColor(renderer, current_r, current_g, current_b, current_a);
    
    // Draw particle as filled circle with size based on lifetime
    int radius = static_cast<int>(size * (0.7f + 0.3f * life_ratio)); // Size reduces with lifetime
    int cx = static_cast<int>(x);
    int cy = static_cast<int>(y);
    
    // Simple filled circle drawing
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            if ((w*w + h*h) <= (radius*radius)) {
                SDL_RenderDrawPoint(renderer, cx + w, cy + h);
            }
        }
    }
}

void Particle::applyForce(float fx, float fy) {
    ax += fx;
    ay += fy;
}

// Convert HSV to RGB for colorful effects
void Particle::HSVtoRGB(float h, float s, float v, uint8_t& r, uint8_t& g, uint8_t& b) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;
    
    float r_f, g_f, b_f;
    
    if (h >= 0 && h < 60) {
        r_f = c, g_f = x, b_f = 0;
    } else if (h >= 60 && h < 120) {
        r_f = x, g_f = c, b_f = 0;
    } else if (h >= 120 && h < 180) {
        r_f = 0, g_f = c, b_f = x;
    } else if (h >= 180 && h < 240) {
        r_f = 0, g_f = x, b_f = c;
    } else if (h >= 240 && h < 300) {
        r_f = x, g_f = 0, b_f = c;
    } else {
        r_f = c, g_f = 0, b_f = x;
    }
    
    r = static_cast<uint8_t>((r_f + m) * 255);
    g = static_cast<uint8_t>((g_f + m) * 255);
    b = static_cast<uint8_t>((b_f + m) * 255);
}
