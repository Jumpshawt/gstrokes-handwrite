#include "render.hpp"
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <cmath>
#include <algorithm>

SDL_Texture* create_brush_stamp(SDL_Renderer* renderer, int radius) {
    int size = radius * 2;
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, size, size);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    std::vector<uint32_t> pixels(size * size, 0);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            float dx = (x + 0.5f) - (float)radius;
            float dy = (y + 0.5f) - (float)radius;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < (float)radius) {
                float alpha = (dist > (float)radius - 1.5f) ? 1.0f - (dist - ((float)radius - 1.5f)) / 1.5f : 1.0f;
                uint8_t a = (uint8_t)(alpha * 255);
                pixels[y * size + x] = (a << 24) | (255 << 16) | (255 << 8) | 255;
            }
        }
        }
    SDL_UpdateTexture(texture, NULL, pixels.data(), size * sizeof(uint32_t));
    return texture;
}

void draw_bezier_stamps(SDL_Renderer* r, SDL_Texture* tex, float x0, float y0, float x1, float y1, float x2, float y2, Color col) {
    float chord_len = std::sqrt(std::pow(x1-x0,2) + std::pow(y1-y0,2)) + std::sqrt(std::pow(x2-x1,2) + std::pow(y2-y1,2));
    int steps = std::max(5, (int)(chord_len / 1.5f)); 
    SDL_SetTextureColorMod(tex, col.r, col.g, col.b);
    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / steps;
        float it = 1.0f - t;
        float tx = it * it * x0 + 2 * it * t * x1 + t * t * x2;
        float ty = it * it * y0 + 2 * it * t * y1 + t * t * y2;
        SDL_Rect rect = {(int)(tx - BRUSH_SIZE/2), (int)(ty - BRUSH_SIZE/2), BRUSH_SIZE, BRUSH_SIZE};
        SDL_RenderCopy(r, tex, NULL, &rect);
    }
}

void draw_suggestions(SDL_Renderer *r, TTF_Font *font, AppState &state){
    if (!font) return;
    // Draw Results
    if (font) {
        std::vector<std::string> current_results;
        {
            std::lock_guard<std::mutex> lock(state.results_mutex);
            current_results = state.results;
        }
        
        if (!current_results.empty()) {
            // Check if results have changed since last draw to update animation state
            std::string current_merged_text = "";
            for (size_t i = 0; i < std::min<size_t>(5, current_results.size()); ++i) {
                current_merged_text += current_results[i] + " ";
            }

            if (current_merged_text != state.last_drawn_text) {
                state.last_drawn_text = current_merged_text;
                state.results_timestamp = SDL_GetTicks();
                state.results_stale = false; // New results received, no longer stale!
            }

            float elapsed = (SDL_GetTicks() - state.results_timestamp) / 1000.0f;
            float anim_progress = std::min(1.0f, elapsed / 0.1f); // 100ms intro animation
            
            // Ease-out cubic: 1 - (1-t)^3 for intro
            float ease_out = 1.0f - std::pow(1.0f - anim_progress, 3);
            
            // If touching/drawing or if results are generally stale, keep it grayed out
            SDL_Color textColor = (state.touching || state.results_stale) ? SDL_Color{150, 150, 150, 255} : SDL_Color{0, 0, 0, 255};

            // Instead of one big string, we'll track the starting X position and render each word in its own box
            int current_x = 20;
            
            for (size_t i = 0; i < std::min<size_t>(5, current_results.size()); ++i) {
                std::string word = current_results[i];
                
                SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, word.c_str(), textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(r, textSurface);
                    int tw = textSurface->w;
                    int th = textSurface->h;
                    
                    // Slide-up effect
                    int start_y = 50;
                    int end_y = 50;
                    int current_y = start_y - (int)((start_y - end_y) * ease_out);
                    
                    // Calculate final alpha (intro fade only)
                    float current_alpha = ease_out;
                    uint8_t alpha = (uint8_t)(255 * current_alpha);
                    
                    // Draw Background Box per word
                    SDL_Rect bgQuad = {current_x - 10, current_y - 10, tw + 20, th + 20};
                    
                    // Set blend mode for r to support transparent rects
                    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                    
                    // Draw white box with slight transparency and matching alpha fade
                    SDL_SetRenderDrawColor(r, 255, 255, 255, (uint8_t)(220 * current_alpha));
                    SDL_RenderFillRect(r, &bgQuad);
                    
                    // Draw box border
                    SDL_SetRenderDrawColor(r, 200, 200, 200, (uint8_t)(255 * current_alpha));
                    SDL_RenderDrawRect(r, &bgQuad);

                    // Draw Text
                    SDL_SetTextureAlphaMod(textTexture, alpha);
                    SDL_Rect renderQuad = {current_x, current_y, tw, th}; 
                    SDL_RenderCopy(r, textTexture, NULL, &renderQuad);
                    
                    SDL_DestroyTexture(textTexture);
                    SDL_FreeSurface(textSurface);
                    
                    // Advance X for the next box, adding some padding/margin between them
                    current_x += tw + 40; 
                }
            }
        }
    }
}
