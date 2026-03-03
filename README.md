# gstrokes-handwrite
A C++ handwriting recognition, using googles handwriting API and SDL. 

# Building 
Dependencies: SDL, libcurl, nlohmann-json

To build, run: 

```g++ draw2.cpp -o whatda2 `pkg-config --cflags --libs sdl2 libevdev` -lSDL2 -lSDL2_gfx -lcurl -Ofast```
