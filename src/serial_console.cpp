#include "serial_console.h"

#include <Arduino.h>
#include <ctype.h>
#include <string.h>

#include "buttons.h"
#include "tama_app.h"
#include "tama_storage.h"

namespace {
constexpr uint32_t kDefaultTapMs = 120;
char g_line[80] = {};
uint8_t g_line_len = 0;

void printHelp() {
  Serial.println("serial: commands: help, dump, save, tap A|B|C|AC|AB|BC|ABC [ms]");
}

uint8_t parseMask(const char* token) {
  uint8_t mask = 0;
  for (const char* p = token; *p != '\0'; ++p) {
    switch (toupper(static_cast<unsigned char>(*p))) {
      case 'A':
        mask |= kTamaButtonA;
        break;
      case 'B':
        mask |= kTamaButtonB;
        break;
      case 'C':
        mask |= kTamaButtonC;
        break;
      default:
        return 0;
    }
  }
  return mask;
}

void handleLine(char* line) {
  char* command = strtok(line, " \t");
  if (command == nullptr) {
    return;
  }

  for (char* p = command; *p != '\0'; ++p) {
    *p = static_cast<char>(tolower(static_cast<unsigned char>(*p)));
  }

  if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
    printHelp();
    return;
  }

  if (strcmp(command, "dump") == 0 || strcmp(command, "frame") == 0) {
    tamaAppPrintDebugFrame();
    return;
  }

  if (strcmp(command, "save") == 0) {
    Serial.printf("serial: save %s\n", tamaStorageSave() ? "ok" : "failed");
    return;
  }

  if (strcmp(command, "tap") == 0) {
    const char* mask_token = strtok(nullptr, " \t");
    if (mask_token == nullptr) {
      Serial.println("serial: tap needs A/B/C token");
      return;
    }
    const uint8_t mask = parseMask(mask_token);
    if (mask == 0) {
      Serial.println("serial: invalid tap token");
      return;
    }
    const char* duration_token = strtok(nullptr, " \t");
    uint32_t duration_ms = kDefaultTapMs;
    if (duration_token != nullptr) {
      duration_ms = strtoul(duration_token, nullptr, 10);
      if (duration_ms == 0) {
        duration_ms = kDefaultTapMs;
      }
    }
    buttonsInjectTamaMask(mask, duration_ms);
    Serial.printf("serial: tap mask=%u duration=%lums\n", mask,
                  static_cast<unsigned long>(duration_ms));
    return;
  }

  Serial.println("serial: unknown command");
}
}  // namespace

void serialConsoleUpdate() {
  while (Serial.available() > 0) {
    const char ch = static_cast<char>(Serial.read());
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      g_line[g_line_len] = '\0';
      handleLine(g_line);
      g_line_len = 0;
      g_line[0] = '\0';
      continue;
    }
    if (g_line_len + 1 < sizeof(g_line)) {
      g_line[g_line_len++] = ch;
    }
  }
}
