#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "types.hpp"

SDL_Texture* create_brush_stamp(SDL_Renderer* renderer, int radius);
void draw_bezier_stamps(SDL_Renderer* r, SDL_Texture* tex, float x0, float y0, float x1, float y1, float x2, float y2, Color col);
void draw_suggestions(SDL_Renderer* r, TTF_Font* font, AppState& state);
