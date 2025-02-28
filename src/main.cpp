#include "system.hpp"
#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

// Function declaration
void drawCircle(SDL_Renderer* renderer, int x, int y, int radius);

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Create window
    SDL_Window* window = SDL_CreateWindow("Interactive Particle System", 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         SCREEN_WIDTH, 
                                         SCREEN_HEIGHT, 
                                         SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Enable alpha blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Create particle system
    ParticleSystem system(50000, 4); // 50k particles, 4 threads
    
    // Different emitter types
    std::vector<EmitterSettings> presets = {
        // Fountain (blue)
        {
            SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 100.0f, // position
            500.0f,    // rate
            200.0f,    // speed
            3.0f,      // size
            3.0f,      // lifetime
            EmitterType::Point,
            50, 100,   // r range
            150, 255,  // g range
            200, 255,  // b range
            150, 255   // a range
        },
        
        // Explosion (red-orange)
        {
            SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f,
            2000.0f,   // faster emission
            300.0f,    // faster particles
            2.0f,      // smaller size
            1.5f,      // shorter lifetime
            EmitterType::Circle,
            200, 255,  // r range (red)
            50, 150,   // g range
            0, 50,     // b range
            200, 255   // a range
        },
        
        // Snow effect (white)
        {
            SCREEN_WIDTH / 2.0f, 0.0f,
            200.0f,    // slower emission
            50.0f,     // slower speed
            2.0f,      // small size
            8.0f,      // long lifetime
            EmitterType::Line,
            200, 255,  // r range (white)
            200, 255,  // g range
            200, 255,  // b range
            150, 200   // a range
        },
        
        // Spiral (colorful)
        {
            SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f,
            600.0f,    // emission
            150.0f,    // speed
            2.0f,      // size
            5.0f,      // lifetime
            EmitterType::Spiral,
            50, 255,   // r range
            50, 255,   // g range
            50, 255,   // b range
            180, 255,  // a range
            true       // colorful_mode = true
        }
    };
    
    // Start with the fountain preset
    size_t current_preset = 0;
    size_t emitter_id = system.addEmitter(presets[current_preset]);
    
    // Add mouse-controlled force field
    size_t mouse_field = system.addForceField(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 150.0f, -500.0f);
    bool force_field_enabled = true;
    
    // Background color
    uint8_t bg_r = 10, bg_g = 10, bg_b = 30;
    bool dynamic_background = false;
    float bg_hue = 0.0f;
    
    // Main loop
    bool quit = false;
    SDL_Event e;
    
    auto last_time = std::chrono::high_resolution_clock::now();
    
    // Print instructions
    std::cout << "=== Colorful Particle System Controls ===" << std::endl;
    std::cout << "Mouse Movement: Move force field" << std::endl;
    std::cout << "Left Click: Create burst at cursor" << std::endl;
    std::cout << "Space: Toggle attraction/repulsion" << std::endl;
    std::cout << "F: Toggle force field on/off" << std::endl;
    std::cout << "1-4: Switch particle emitter type" << std::endl;
    std::cout << "C: Toggle colorful mode for current emitter" << std::endl;
    std::cout << "B: Toggle dynamic background" << std::endl;
    std::cout << "I: Toggle particle interaction" << std::endl;
    std::cout << "R: Reset system" << std::endl;
    std::cout << "Q/ESC: Quit" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    while (!quit) {
        // Calculate delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Cap delta time to avoid physics issues
        if (dt > 0.05f) dt = 0.05f;
        
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } 
            else if (e.type == SDL_MOUSEMOTION && force_field_enabled) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                system.updateForceField(mouse_field, static_cast<float>(x), static_cast<float>(y));
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Create a temporary burst emitter at mouse position
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    
                    EmitterSettings burst = presets[1]; // Use explosion preset
                    burst.x = x;
                    burst.y = y;
                    burst.rate = 500.0f; // One-time burst
                    
                    size_t burst_id = system.addEmitter(burst);
                    
                    // Schedule removal after short time
                    std::thread([&system, burst_id]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        system.removeEmitter(burst_id);
                    }).detach();
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        quit = true;
                        break;
                    
                    case SDLK_SPACE:
                        // Toggle force field strength
                        if (force_field_enabled) {
                            int x, y;
                            SDL_GetMouseState(&x, &y);
                            
                            system.removeForceField(mouse_field);
                            
                            // Check current field type and toggle
                            float strength = system.getForceFieldStrength(mouse_field);
                            strength = -strength; // Toggle between attract/repel
                            
                            mouse_field = system.addForceField(x, y, 150.0f, strength);
                        }
                        break;
                    
                    case SDLK_f:
                        // Toggle force field on/off
                        force_field_enabled = !force_field_enabled;
                        break;
                    
                    case SDLK_1:
                    case SDLK_2:
                    case SDLK_3:
                    case SDLK_4:
                        // Switch emitter type
                        system.removeEmitter(emitter_id);
                        current_preset = e.key.keysym.sym - SDLK_1;
                        if (current_preset < presets.size()) {
                            emitter_id = system.addEmitter(presets[current_preset]);
                        }
                        break;
                    
                    case SDLK_c:
                        // Toggle colorful mode for current emitter
                        system.removeEmitter(emitter_id);
                        presets[current_preset].colorful_mode = !presets[current_preset].colorful_mode;
                        emitter_id = system.addEmitter(presets[current_preset]);
                        break;
                        
                    case SDLK_b:
                        // Toggle dynamic background
                        dynamic_background = !dynamic_background;
                        break;
                        
                    case SDLK_i:
                        // Toggle particle interaction
                        system.toggleParticleInteraction(!system.isParticleInteractionEnabled());
                        std::cout << "Particle interaction: " 
                                  << (system.isParticleInteractionEnabled() ? "ON" : "OFF") 
                                  << std::endl;
                        break;
                    
                    case SDLK_r:
                        // Reset system
                        system.reset();
                        emitter_id = system.addEmitter(presets[current_preset]);
                        
                        int x, y;
                        SDL_GetMouseState(&x, &y);
                        mouse_field = system.addForceField(x, y, 150.0f, -500.0f);
                        force_field_enabled = true;
                        break;
                }
            }
        }
        
        // Update particle system
        system.update(dt);
        
        // Update dynamic background if enabled
        if (dynamic_background) {
            bg_hue += 10.0f * dt;
            if (bg_hue >= 360.0f) bg_hue -= 360.0f;
            
            // Convert HSV to RGB (simplified version)
            float h = bg_hue / 60.0f;
            int i = static_cast<int>(h);
            float f = h - i;
            float p = 0.0f;  // We want dark backgrounds
            float q = 0.1f * (1.0f - f);
            float t = 0.1f * f;

            switch (i % 6) {
                case 0: bg_r = 10; bg_g = static_cast<uint8_t>(t * 255); bg_b = static_cast<uint8_t>(p * 255); break;
                case 1: bg_r = static_cast<uint8_t>(q * 255); bg_g = 10; bg_b = static_cast<uint8_t>(p * 255); break;
                case 2: bg_r = static_cast<uint8_t>(p * 255); bg_g = 10; bg_b = static_cast<uint8_t>(t * 255); break;
                case 3: bg_r = static_cast<uint8_t>(p * 255); bg_g = static_cast<uint8_t>(q * 255); bg_b = 10; break;
                case 4: bg_r = static_cast<uint8_t>(t * 255); bg_g = static_cast<uint8_t>(p * 255); bg_b = 10; break;
                case 5: bg_r = 10; bg_g = static_cast<uint8_t>(p * 255); bg_b = static_cast<uint8_t>(q * 255); break;
            }
        }
        
        // Clear screen with current background color
        SDL_SetRenderDrawColor(renderer, bg_r, bg_g, bg_b, 255);
        SDL_RenderClear(renderer);
        
        // Render particles
        system.render(renderer);
        
        // Render force field indicator if enabled
        if (force_field_enabled) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            
            // Draw force field circle
            float strength = system.getForceFieldStrength(mouse_field);
            
            // Blue for repulsion, red for attraction, with glow effect
            if (strength < 0) {
                // Draw outer glow (larger, more transparent)
                SDL_SetRenderDrawColor(renderer, 100, 150, 255, 30);
                drawCircle(renderer, x, y, 170);
                
                // Draw inner circle
                SDL_SetRenderDrawColor(renderer, 100, 150, 255, 100);
                drawCircle(renderer, x, y, 150);
                
                // Draw center
                SDL_SetRenderDrawColor(renderer, 150, 200, 255, 150);
                drawCircle(renderer, x, y, 30);
            } else {
                // Draw outer glow (larger, more transparent)
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 30);
                drawCircle(renderer, x, y, 170);
                
                // Draw inner circle
                SDL_SetRenderDrawColor(renderer, 255, 100, 100, 100);
                drawCircle(renderer, x, y, 150);
                
                // Draw center
                SDL_SetRenderDrawColor(renderer, 255, 150, 150, 150);
                drawCircle(renderer, x, y, 30);
            }
        }
        
        // Update screen
        SDL_RenderPresent(renderer);
        
        // Cap to ~60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

// Draw a circle outline
void drawCircle(SDL_Renderer* renderer, int x, int y, int radius) {
    for (int w = -radius; w <= radius; w++) {
        for (int h = -radius; h <= radius; h++) {
            int dist_sq = w*w + h*h;
            if (dist_sq >= (radius-1)*(radius-1) && dist_sq <= radius*radius) {
                SDL_RenderDrawPoint(renderer, x + w, y + h);
            }
        }
    }
}
