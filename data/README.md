# Local ROM setup

TamaLIB needs a first-generation Tamagotchi ROM, but ROM data is not committed
to this repository.

Generate the local header from your own ROM dump:

```powershell
python tools\rom_to_header.py path\to\your_rom.txt data\rom.h --format text
```

For the common MAME P1/P2 files (`tama.bin`, `tama.b`, `tamag2.bin`), use:

```powershell
python tools\rom_to_header.py path\to\tama.bin data\rom.h --format words-be16
```

`--format auto` also treats 0x3000/0x4000-byte MAME files as 16-bit
big-endian words. P1/P2 files with 6144 source words are padded to 8192 words
for TamaLIB.

The firmware auto-detects `data/rom.h` at compile time. Without that file it
still builds and shows a setup/debug screen on the StickS3.

Expected generated symbol:

```cpp
const u12_t kTamaRom[8192] = { ... };
```
