#!/usr/bin/env python3
"""Convert a local Tamagotchi ROM dump into data/rom.h.

Supported formats:
- text: hex 12-bit words separated by whitespace, comma, or braces
- words-le16 / words-be16: 16-bit words, padded to 8192 words when needed
- packed12-be: two 12-bit words packed as b0, b1, b2 big-endian
- packed12-le: two 12-bit words packed as b0, b1, b2 little-endian
"""

from __future__ import annotations

import argparse
import re
from pathlib import Path

WORD_COUNT = 8192
P1_P2_WORD_COUNT = 6144


def parse_text(data: bytes) -> list[int]:
    text = data.decode("utf-8", errors="ignore")
    tokens = re.findall(r"0x[0-9a-fA-F]+|[0-9a-fA-F]+", text)
    words: list[int] = []
    for token in tokens:
        words.append(int(token, 16) & 0x0FFF)
    return words


def parse_words(data: bytes, byteorder: str) -> list[int]:
    if len(data) % 2:
        raise ValueError("16-bit word ROM length must be even")
    return [
        int.from_bytes(data[i : i + 2], byteorder) & 0x0FFF
        for i in range(0, len(data), 2)
    ]


def parse_packed12_be(data: bytes) -> list[int]:
    if len(data) % 3:
        raise ValueError("packed12-be ROM length must be divisible by 3")
    words: list[int] = []
    for i in range(0, len(data), 3):
        b0, b1, b2 = data[i], data[i + 1], data[i + 2]
        words.append(((b0 << 4) | (b1 >> 4)) & 0x0FFF)
        words.append((((b1 & 0x0F) << 8) | b2) & 0x0FFF)
    return words


def parse_packed12_le(data: bytes) -> list[int]:
    if len(data) % 3:
        raise ValueError("packed12-le ROM length must be divisible by 3")
    words: list[int] = []
    for i in range(0, len(data), 3):
        b0, b1, b2 = data[i], data[i + 1], data[i + 2]
        words.append((b0 | ((b1 & 0x0F) << 8)) & 0x0FFF)
        words.append(((b1 >> 4) | (b2 << 4)) & 0x0FFF)
    return words


def detect_format(data: bytes, source: Path) -> str:
    if source.suffix.lower() in {".txt", ".h", ".c", ".cpp"}:
        return "text"
    if len(data) in {0x3000, 0x4000}:
        return "words-be16"
    if len(data) == WORD_COUNT * 2:
        return "words-le16"
    if len(data) == WORD_COUNT * 3 // 2:
        return "packed12-be"
    return "text"


def parse_rom(data: bytes, source: Path, fmt: str) -> list[int]:
    if fmt == "auto":
        fmt = detect_format(data, source)
    if fmt == "text":
        return parse_text(data)
    if fmt == "words-le16":
        return parse_words(data, "little")
    if fmt == "words-be16":
        return parse_words(data, "big")
    if fmt == "packed12-be":
        return parse_packed12_be(data)
    if fmt == "packed12-le":
        return parse_packed12_le(data)
    raise ValueError(f"unsupported format: {fmt}")


def normalize_word_count(words: list[int]) -> list[int]:
    if len(words) == WORD_COUNT:
        return words
    if len(words) == P1_P2_WORD_COUNT:
        return words + [0] * (WORD_COUNT - P1_P2_WORD_COUNT)
    raise ValueError(
        f"expected {WORD_COUNT} words, or {P1_P2_WORD_COUNT} words for P1/P2 padding, "
        f"got {len(words)}"
    )


def write_header(words: list[int], output: Path) -> None:
    source_word_count = len(words)
    words = normalize_word_count(words)
    if any(word > 0x0FFF for word in words):
        raise ValueError("all words must be 12-bit values")

    output.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "#pragma once",
        "",
        '#include "../lib/hal_types.h"',
        "",
        f"constexpr unsigned int kTamaRomSourceWordCount = {source_word_count};",
        f"constexpr unsigned int kTamaRomWordCount = {WORD_COUNT};",
        "const u12_t kTamaRom[kTamaRomWordCount] = {",
    ]
    for offset in range(0, WORD_COUNT, 8):
        chunk = ", ".join(f"0x{word:03X}" for word in words[offset : offset + 8])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")
    output.write_text("\n".join(lines), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path, nargs="?", default=Path("data/rom.h"))
    parser.add_argument(
        "--format",
        choices=["auto", "text", "words-le16", "words-be16", "packed12-be", "packed12-le"],
        default="auto",
    )
    args = parser.parse_args()

    data = args.input.read_bytes()
    words = parse_rom(data, args.input, args.format)
    write_header(words, args.output)
    final_words = len(normalize_word_count(words))
    print(f"wrote {args.output} with {len(words)} source words padded to {final_words} words")


if __name__ == "__main__":
    main()
