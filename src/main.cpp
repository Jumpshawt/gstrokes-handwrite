#include <SDL_keycode.h>
#include <iostream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <SDL2/SDL.h>
#include <thread>
#include <atomic>
#include <cmath>
#include <SDL2/SDL_ttf.h>
#include "input.hpp"
#include "log.h"

#include "types.hpp"
#include "render.hpp"
#include "network.hpp"

int main(int argc, char* argv[]) {
    log_info("GHandWrite started. Resolution: %dx%d", WIDTH, HEIGHT);
    std::string language = "en";
    std::string font_path = "/usr/share/fonts/noto/NotoSans-Regular.ttf";
    bool realtime = false;
    bool touchpad = false; 
    log_debug("Getting arguments.\n");
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l" || arg == "--lang") {
            if (i + 1 < argc) {
                language = argv[++i];
            } else {
                std::cerr << "Error: " << arg << " requires an argument.\n";
                return 1;
            }
        } else if (arg == "-f" || arg == "--font") {
            if (i + 1 < argc) {
                font_path = argv[++i];
            } else {
                std::cerr << "Error: " << arg << " requires an argument.\n";
                return 1;
            }
        } else if (arg == "-r" || arg == "--realtime") {
            realtime = true;
        } else if (arg == "-t" || arg == "--touchpad") {
            touchpad = true; 
        }
        else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -l, --lang <lang>   Set recognition language (e.g. en, ja, he, zh-TW). Default: en\n";
            std::cout << "  -f, --font <path>   Set the font to use. Default: " << font_path << "\n";
            std::cout << "  -r, --realtime      Enable real-time recognition during stroke\n";
            std::cout << "  -h, --help          Show help message\n";
            std::cout << "  -t  --touchpad      Enable touchpad support\n";
            return 0;
        } else {
             std::cerr << "Unknown argument: " << arg << "\n";
             return 1;
        }
    }
    

    log_debug("Getting touchpad.\n");
    std::string dev_path = find_touchpad();
    //TODO: add a check so this only runs when you have touchpad enabled
    if (dev_path.empty()) { std::cerr << "Touchpad not found\n"; return 1; }


    int fd = open(dev_path.c_str(), O_RDONLY | O_NONBLOCK);
    struct libevdev* dev = NULL;
    libevdev_new_from_fd(fd, &dev);
    

    AppState state;
    state.max_abs_x = libevdev_get_abs_maximum(dev, ABS_X);
    state.max_abs_y = libevdev_get_abs_maximum(dev, ABS_Y);

    log_debug("SDL init\n");
    SDL_Init(SDL_INIT_VIDEO);
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << "\n";
        return 1;
    }

    log_debug("Loading font.\n");
    TTF_Font* font = TTF_OpenFont(font_path.c_str(), 42); // Open font with size 42
    if (!font) {
        std::cerr << "Failed to load font " << font_path << ": " << TTF_GetError() << "\n";
    }

    log_debug("Creating window.\n");
    SDL_Window* window = SDL_CreateWindow("GHandWrite", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    log_debug("Creating renderer.\n");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    
    log_debug("Setting window modes.\n");
    SDL_SetWindowGrab(window, SDL_TRUE);
    log_debug("Relative mouse mode.\n");
    SDL_SetRelativeMouseMode(SDL_TRUE);
    log_debug("Show Cursor.\n");
    SDL_ShowCursor(SDL_DISABLE);

    log_debug("Brush size stuff.\n");
    state.brush_texture = create_brush_stamp(renderer, BRUSH_SIZE / 2);
    SDL_Texture* canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
    
    log_debug("Setting renderer.\n");
    SDL_SetRenderTarget(renderer, canvas);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    

    log_debug("Starting main loop.\n");
    //main loop
    while (!state.quit) {
        SDL_Event e;
        //SDL events 
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)) state.quit = true;

            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_c) {
                state.trace.clear();
                {
                    std::lock_guard<std::mutex> lock(state.results_mutex);
                    state.results.clear();
                    state.last_drawn_text = "";
                }
                SDL_SetRenderTarget(renderer, canvas);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderClear(renderer);
                
                std::cout << "\r                                     " << std::flush; //clear the terminal output to update the character
            }
        }

        struct input_event ev;
        bool input_happened = false;
        bool coords_updated = false;

        while (libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev) == 0) {
            if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                state.touching = ev.value;
                if (state.touching) {
                    state.prev_x = -1; state.mid_x = -1;
                    state.trace.push_back({{}, {}, (int)(state.trace.size() % PALETTE.size())});
                    state.results_stale = true; // Mark old results as stale while drawing
                } else {
                    // Trigger one last recognition when finger is lifted
                    state.needs_recognition = true;
                }
            }

            if (ev.type == EV_ABS && state.touching) {
                if (ev.code == ABS_X) { state.x = (ev.value * (float)WIDTH) / state.max_abs_x; coords_updated = true; }
                if (ev.code == ABS_Y) { state.y = (ev.value * (float)HEIGHT) / state.max_abs_y; coords_updated = true; }
            }
            
            if (ev.type == EV_SYN && ev.code == SYN_REPORT && state.touching && coords_updated) {
                coords_updated = false;
                if (state.prev_x != -1) {
                    float mx = (state.x + state.prev_x) / 2.0f;
                    float my = (state.y + state.prev_y) / 2.0f; if (state.mid_x != -1) {
                        SDL_SetRenderTarget(renderer, canvas);
                        draw_bezier_stamps(renderer, state.brush_texture, state.mid_x, state.mid_y, state.prev_x, state.prev_y, mx, my, PALETTE[state.trace.back().color_index]);
                        state.trace.back().x.push_back((int)mx);
                        state.trace.back().y.push_back((int)my);
                        input_happened = true;
                        
                        // Real-time trigger: every 8 points added; This is stupid! (except for when you're writing in cursive, in which case its great !)
                        if (realtime && state.trace.back().x.size() % 8 == 0) 
                            state.needs_recognition = true;
                    }
                    state.mid_x = mx; state.mid_y = my;
                }
                state.prev_x = state.x; state.prev_y = state.y;
            }
        }

        // Handle Thread Spawning
        if (state.needs_recognition && !state.is_requesting && !state.trace.empty()) {
            state.needs_recognition = false;
            state.is_requesting = true;
            std::thread(async_recognize, state.trace, &state.is_requesting, language, &state.results, &state.results_mutex).detach();
        }

        if (input_happened || true) {
            SDL_SetRenderTarget(renderer, NULL);
            SDL_RenderCopy(renderer, canvas, NULL, NULL);

            draw_suggestions(renderer, font, state); // Clean and descriptive

            SDL_RenderPresent(renderer);
        }
        SDL_Delay(5);
    }

    // Cleanup
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyTexture(state.brush_texture);
    SDL_DestroyTexture(canvas);
    libevdev_grab(dev, LIBEVDEV_UNGRAB); //TODO: this sometimes doesn't release properly, leading to broken trackpad inputs 
    libevdev_free(dev);
    close(fd);
    SDL_Quit();
    return 0;
}
