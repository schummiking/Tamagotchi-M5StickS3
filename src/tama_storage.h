#pragma once

#include <Arduino.h>

bool tamaStorageInit();
bool tamaStorageRestore();
bool tamaStorageSave();
bool tamaStorageFlush();
void tamaStorageMarkDirty();
void tamaStorageUpdate(bool running, bool any_pressed, uint32_t idle_age_ms);
bool tamaStorageIsMounted();
bool tamaStorageHasSave();
uint32_t tamaStorageLastSaveMs();
void tamaStoragePrintDiagnostics();
