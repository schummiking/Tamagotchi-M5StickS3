# Local ROM setup

TamaLIB needs a first-generation Tamagotchi ROM, but ROM data is not committed
to this repository.

Generate the local header from your own ROM dump:

```powershell
python tools\rom_to_header.py path\to\your_rom.txt data\rom.h --format text
```

The firmware auto-detects `data/rom.h` at compile time. Without that file it
still builds and shows a setup/debug screen on the StickS3.

Expected generated symbol:

```cpp
const u12_t kTamaRom[8192] = { ... };
```
