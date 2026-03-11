#pragma once

#include <vector>
#include <atomic>
#include <string>
#include <mutex>
#include <SDL2/SDL.h>

const int WIDTH = 1000;
const int HEIGHT = 800;
const int BRUSH_SIZE = 8;
struct Color { uint8_t r, g, b; };
const std::vector<Color> PALETTE = {{44, 44, 44}, {231, 76, 60}, {46, 204, 113}, {52, 152, 219}};

struct Stroke {
    std::vector<int> x, y;
    int color_index;
};

struct AppState {
    float x = 0, y = 0, prev_x = -1, prev_y = -1, mid_x = -1, mid_y = -1;
    int max_abs_x = 1, max_abs_y = 1;
    bool touching = false;
    bool quit = false;
    bool needs_recognition = false; // Flag to trigger recognition
    std::vector<Stroke> trace;
    std::atomic<bool> is_requesting{false};
    SDL_Texture* brush_texture = nullptr;
    std::mutex results_mutex;
    std::vector<std::string> results;
    uint32_t results_timestamp = 0;
    std::string last_drawn_text = "";
    bool results_stale = false;
};
