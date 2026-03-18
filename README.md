# Equinox

Equinox is a subtractive synthesizer and sampler plugin built with [JUCE](https://juce.com/).

<img width="1053" alt="Equinox screenshot" src="https://user-images.githubusercontent.com/44835517/224553821-5c762d98-c61f-42be-a3e9-eb6fe983a364.png">

## Overview

- Subtractive synth and sampler engine
- Built with C++20 and CMake
- Plugin formats: `AU`, `VST3`, and `Standalone`

## Build

JUCE is added as a git submodule under `lib/JUCE`, so after initializing submodules you can build the project directly with CMake:

```bash
git submodule update --init --recursive
cmake -S . -B build
cmake --build build
```

Minimum requirements:

- CMake 3.24+
- A C++20-compatible compiler

## License

This project is licensed under the terms of the GNU General Public License v3.0. See [LICENSE](LICENSE) for details.
