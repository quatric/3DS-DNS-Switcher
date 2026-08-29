#pragma once

#include <3ds.h>

constexpr u32 WifiBlockBase = 0x00080000;
constexpr size_t WifiSlotSize = 0xC00;

// Layout used by CFG Wi-Fi blocks. The trailing bytes are intentionally kept
// opaque: this plugin must not disturb passwords, proxy settings, or metadata.
struct __attribute__((packed)) WifiSlot {
    u16 version;
    u16 crc;
    u8 networkEnable;
    u8 editableSecurity;
    u8 padding1[2];
    // Main SSID data, multi-SSID data, and their headers (offset 0x008..0x33F).
    u8 networkAndMultiSsid[0x338];
    u8 enableDhcp;
    u8 enableAutoDns;
    u8 padding2[2];
    u8 ipAddress[4];
    u8 gatewayAddress[4];
    u8 subnetMask[4];
    u8 primaryDns[4];
    u8 secondaryDns[4];
    u8 remainder[WifiSlotSize - 0x358];
};

static_assert(sizeof(WifiSlot) == WifiSlotSize, "Incorrect CFG Wi-Fi slot layout");

u16 WifiSlotCrc(const WifiSlot &slot);
Result SetSlotDns(int slotIndex, const u8 primary[4]);
