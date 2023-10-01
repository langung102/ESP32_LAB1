## Build project
```
mkdir -b build
cd build
cmake .. -G Ninja
ninja
```

## Flash to ESP32
```
ninja flash
```