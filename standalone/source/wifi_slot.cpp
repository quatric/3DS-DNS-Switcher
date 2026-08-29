#include "wifi_slot.hpp"

namespace {
u16 Crc16(const u8 *data, size_t length) {
    u16 crc = 0;
    while (length--) {
        crc ^= *data++;
        for (int bit = 0; bit < 8; ++bit)
            crc = (crc & 1) ? static_cast<u16>((crc >> 1) ^ 0xA001)
                            : static_cast<u16>(crc >> 1);
    }
    return crc;
}
}

u16 WifiSlotCrc(const WifiSlot &slot) {
    // CFG validates exactly the first 0x410 bytes after version and CRC.
    return Crc16(reinterpret_cast<const u8 *>(&slot) + 4, 0x410);
}

Result SetSlotDns(int slotIndex, const u8 primary[4]) {
    if (slotIndex < 0 || slotIndex > 2)
        return MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_APPLICATION, RD_OUT_OF_RANGE);

    WifiSlot slot{};
    Result rc = CFG_GetConfigInfoBlk8(sizeof(slot), WifiBlockBase + slotIndex,
                                      reinterpret_cast<u8 *>(&slot));
    if (R_FAILED(rc))
        return rc;
    if (slot.version == 0)
        return MAKERESULT(RL_USAGE, RS_NOTFOUND, RM_APPLICATION, RD_NOT_FOUND);

    slot.enableAutoDns = 0; // Manual DNS; DHCP remains unchanged.
    for (int i = 0; i < 4; ++i)
        slot.primaryDns[i] = primary[i];
    slot.secondaryDns[0] = 1;
    slot.secondaryDns[1] = 1;
    slot.secondaryDns[2] = 1;
    slot.secondaryDns[3] = 1;
    slot.crc = WifiSlotCrc(slot);

    rc = CFG_SetConfigInfoBlk8(sizeof(slot), WifiBlockBase + slotIndex,
                               reinterpret_cast<const u8 *>(&slot));
    if (R_SUCCEEDED(rc))
        rc = CFG_UpdateConfigSavegame();
    return rc;
}
