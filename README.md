# gstrokes-handwrite
A C++ handwriting recognition, using googles handwriting API and SDL. 

## Building 
Dependencies: SDL, libcurl, nlohmann-json

To build, run: 
```bash
cmake --build build 
```

## Arguments
 -l, --lang <lang>   Set recognition language (e.g. en, ja, he, zh-TW). Default: en\n
 -f, --font <path>   Set the font to use. 
 -r, --realtime      Enable real-time recognition during stroke
 -h, --help          Show help message
