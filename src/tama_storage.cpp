#include "tama_storage.h"

#include <LittleFS.h>
#include <string.h>

extern "C" {
#include "cpu.h"
#include "tamalib.h"
}

namespace {
constexpr const char* kSavePath = "/tama_state.bin";
constexpr const char* kTempSavePath = "/tama_state.tmp";
constexpr uint32_t kSaveMagic = 0x33544D54;  // TMT3
constexpr uint16_t kSaveVersion = 1;
constexpr uint32_t kIdleSaveDelayMs = 2000;
constexpr uint32_t kMinSaveIntervalMs = 5000;

struct SavePayload {
  u13_t pc;
  u12_t x;
  u12_t y;
  u4_t a;
  u4_t b;
  u5_t np;
  u8_t sp;
  u4_t flags;

  u32_t tick_counter;
  u32_t clk_timer_2hz_timestamp;
  u32_t clk_timer_4hz_timestamp;
  u32_t clk_timer_8hz_timestamp;
  u32_t clk_timer_16hz_timestamp;
  u32_t clk_timer_32hz_timestamp;
  u32_t clk_timer_64hz_timestamp;
  u32_t clk_timer_128hz_timestamp;
  u32_t clk_timer_256hz_timestamp;
  u32_t prog_timer_timestamp;
  bool_t prog_timer_enabled;
  u8_t prog_timer_data;
  u8_t prog_timer_rld;

  u32_t call_depth;
  bool_t cpu_halted;

  interrupt_t interrupts[INT_SLOT_NUM];
  MEM_BUFFER_TYPE memory[MEM_BUFFER_SIZE];
};

struct SaveFile {
  uint32_t magic;
  uint16_t version;
  uint16_t payload_size;
  uint32_t checksum;
  SavePayload payload;
};

bool g_mounted = false;
bool g_dirty = false;
uint32_t g_dirty_since_ms = 0;
uint32_t g_last_save_ms = 0;

uint32_t checksumPayload(const SavePayload& payload) {
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&payload);
  uint32_t hash = 2166136261UL;
  for (size_t i = 0; i < sizeof(SavePayload); ++i) {
    hash ^= bytes[i];
    hash *= 16777619UL;
  }
  return hash;
}

state_t* currentState() {
  state_t* state = tamalib_get_state();
  if (state == nullptr || state->pc == nullptr || state->memory == nullptr ||
      state->interrupts == nullptr) {
    return nullptr;
  }
  return state;
}

bool capturePayload(SavePayload& payload) {
  state_t* state = currentState();
  if (state == nullptr) {
    return false;
  }

  payload.pc = *state->pc;
  payload.x = *state->x;
  payload.y = *state->y;
  payload.a = *state->a;
  payload.b = *state->b;
  payload.np = *state->np;
  payload.sp = *state->sp;
  payload.flags = *state->flags;

  payload.tick_counter = *state->tick_counter;
  payload.clk_timer_2hz_timestamp = *state->clk_timer_2hz_timestamp;
  payload.clk_timer_4hz_timestamp = *state->clk_timer_4hz_timestamp;
  payload.clk_timer_8hz_timestamp = *state->clk_timer_8hz_timestamp;
  payload.clk_timer_16hz_timestamp = *state->clk_timer_16hz_timestamp;
  payload.clk_timer_32hz_timestamp = *state->clk_timer_32hz_timestamp;
  payload.clk_timer_64hz_timestamp = *state->clk_timer_64hz_timestamp;
  payload.clk_timer_128hz_timestamp = *state->clk_timer_128hz_timestamp;
  payload.clk_timer_256hz_timestamp = *state->clk_timer_256hz_timestamp;
  payload.prog_timer_timestamp = *state->prog_timer_timestamp;
  payload.prog_timer_enabled = *state->prog_timer_enabled;
  payload.prog_timer_data = *state->prog_timer_data;
  payload.prog_timer_rld = *state->prog_timer_rld;

  payload.call_depth = *state->call_depth;
  payload.cpu_halted = *state->cpu_halted;

  memcpy(payload.interrupts, state->interrupts, sizeof(payload.interrupts));
  memcpy(payload.memory, state->memory, sizeof(payload.memory));
  return true;
}

bool applyPayload(const SavePayload& payload) {
  state_t* state = currentState();
  if (state == nullptr) {
    return false;
  }

  *state->pc = payload.pc;
  *state->x = payload.x;
  *state->y = payload.y;
  *state->a = payload.a;
  *state->b = payload.b;
  *state->np = payload.np;
  *state->sp = payload.sp;
  *state->flags = payload.flags;

  *state->tick_counter = payload.tick_counter;
  *state->clk_timer_2hz_timestamp = payload.clk_timer_2hz_timestamp;
  *state->clk_timer_4hz_timestamp = payload.clk_timer_4hz_timestamp;
  *state->clk_timer_8hz_timestamp = payload.clk_timer_8hz_timestamp;
  *state->clk_timer_16hz_timestamp = payload.clk_timer_16hz_timestamp;
  *state->clk_timer_32hz_timestamp = payload.clk_timer_32hz_timestamp;
  *state->clk_timer_64hz_timestamp = payload.clk_timer_64hz_timestamp;
  *state->clk_timer_128hz_timestamp = payload.clk_timer_128hz_timestamp;
  *state->clk_timer_256hz_timestamp = payload.clk_timer_256hz_timestamp;
  *state->prog_timer_timestamp = payload.prog_timer_timestamp;
  *state->prog_timer_enabled = payload.prog_timer_enabled;
  *state->prog_timer_data = payload.prog_timer_data;
  *state->prog_timer_rld = payload.prog_timer_rld;

  *state->call_depth = payload.call_depth;
  *state->cpu_halted = payload.cpu_halted;

  memcpy(state->interrupts, payload.interrupts, sizeof(payload.interrupts));
  memcpy(state->memory, payload.memory, sizeof(payload.memory));

  cpu_sync_ref_timestamp();
  tamalib_refresh_hw();
  return true;
}
}  // namespace

bool tamaStorageInit() {
  g_mounted = LittleFS.begin(true);
  if (g_mounted) {
    Serial.println("storage: LittleFS mounted");
  } else {
    Serial.println("storage: LittleFS mount failed");
  }
  return g_mounted;
}

bool tamaStorageRestore() {
  if (!g_mounted || !LittleFS.exists(kSavePath)) {
    Serial.println("storage: no save found");
    return false;
  }

  File file = LittleFS.open(kSavePath, FILE_READ);
  if (!file) {
    Serial.println("storage: open save failed");
    return false;
  }

  SaveFile save = {};
  const size_t bytes_read = file.read(reinterpret_cast<uint8_t*>(&save), sizeof(save));
  file.close();

  if (bytes_read != sizeof(save) || save.magic != kSaveMagic ||
      save.version != kSaveVersion || save.payload_size != sizeof(SavePayload) ||
      save.checksum != checksumPayload(save.payload)) {
    Serial.println("storage: save invalid");
    return false;
  }

  if (!applyPayload(save.payload)) {
    Serial.println("storage: restore apply failed");
    return false;
  }

  g_dirty = false;
  Serial.printf("storage: restored %u bytes\n", static_cast<unsigned>(sizeof(save)));
  return true;
}

bool tamaStorageSave() {
  if (!g_mounted) {
    return false;
  }

  SaveFile save = {};
  save.magic = kSaveMagic;
  save.version = kSaveVersion;
  save.payload_size = sizeof(SavePayload);
  if (!capturePayload(save.payload)) {
    Serial.println("storage: capture failed");
    return false;
  }
  save.checksum = checksumPayload(save.payload);

  File file = LittleFS.open(kTempSavePath, FILE_WRITE);
  if (!file) {
    Serial.println("storage: temp save open failed");
    return false;
  }

  const size_t bytes_written = file.write(reinterpret_cast<const uint8_t*>(&save), sizeof(save));
  file.close();
  if (bytes_written != sizeof(save)) {
    LittleFS.remove(kTempSavePath);
    Serial.println("storage: temp save write failed");
    return false;
  }

  LittleFS.remove(kSavePath);
  if (!LittleFS.rename(kTempSavePath, kSavePath)) {
    LittleFS.remove(kTempSavePath);
    Serial.println("storage: save rename failed");
    return false;
  }

  g_dirty = false;
  g_last_save_ms = millis();
  Serial.printf("storage: saved %u bytes\n", static_cast<unsigned>(sizeof(save)));
  return true;
}

bool tamaStorageFlush() {
  if (!g_dirty) {
    return true;
  }
  return tamaStorageSave();
}

void tamaStorageMarkDirty() {
  if (!g_dirty) {
    g_dirty_since_ms = millis();
  }
  g_dirty = true;
}

void tamaStorageUpdate(bool running, bool any_pressed, uint32_t idle_age_ms) {
  if (!running || !g_dirty || any_pressed || idle_age_ms < kIdleSaveDelayMs) {
    return;
  }
  const uint32_t now = millis();
  if (g_last_save_ms != 0 && now - g_last_save_ms < kMinSaveIntervalMs) {
    return;
  }
  if (now - g_dirty_since_ms < kIdleSaveDelayMs) {
    return;
  }
  tamaStorageSave();
}

bool tamaStorageIsMounted() {
  return g_mounted;
}

bool tamaStorageHasSave() {
  return g_mounted && LittleFS.exists(kSavePath);
}

uint32_t tamaStorageLastSaveMs() {
  return g_last_save_ms;
}
