#include <3ds.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "wifi_slot.hpp"

namespace {
constexpr const char *ProfileDirectory = "sdmc:/3ds/dns-switcher";
constexpr const char *ProfilePath = "sdmc:/3ds/dns-switcher/profiles.txt";
constexpr const char *SettingsPath = "sdmc:/3ds/dns-switcher/settings.txt";
constexpr size_t MaxProfiles = 12;

const char *TargetName(int target) {
    static const char *Names[] = {
        "All configured Wi-Fi slots",
        "Connection slot 1",
        "Connection slot 2",
        "Connection slot 3"
    };
    return Names[target >= 0 && target <= 3 ? target : 0];
}

int LoadTarget() {
    FILE *file = std::fopen(SettingsPath, "r");
    if (!file) return 0;
    int target = 0;
    const bool valid = std::fscanf(file, "target=%d", &target) == 1 &&
                       target >= 0 && target <= 3;
    std::fclose(file);
    return valid ? target : 0;
}

bool SaveTarget(int target) {
    mkdir("sdmc:/3ds", 0777);
    mkdir(ProfileDirectory, 0777);
    FILE *file = std::fopen(SettingsPath, "w");
    if (!file) return false;
    std::fprintf(file, "target=%d\n", target);
    return std::fclose(file) == 0;
}

struct DnsProfile {
    std::string name;
    std::string dns;
};

void PresentFrame() {
    gfxFlushBuffers();
    gspWaitForVBlank();
}

void ClearConsole() {
    consoleClear();
    std::printf("\x1b[1;1H");
}

bool ParseIpv4(const char *text, u8 out[4]) {
    unsigned values[4];
    char tail;
    if (std::sscanf(text, "%u.%u.%u.%u%c", &values[0], &values[1],
                    &values[2], &values[3], &tail) != 4)
        return false;
    for (int i = 0; i < 4; ++i) {
        if (values[i] > 255)
            return false;
        out[i] = static_cast<u8>(values[i]);
    }
    return true;
}

bool AskText(const char *hint, char *output, size_t capacity) {
    SwkbdState keyboard;
    swkbdInit(&keyboard, SWKBD_TYPE_QWERTY, 2, capacity - 1);
    swkbdSetHintText(&keyboard, hint);
    swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    return swkbdInputText(&keyboard, output, capacity) == SWKBD_BUTTON_CONFIRM;
}

void SanitizeName(char *name) {
    for (; *name; ++name)
        if (*name == '\t' || *name == '\r' || *name == '\n') *name = ' ';
}

std::vector<DnsProfile> LoadProfiles() {
    std::vector<DnsProfile> profiles;
    FILE *file = std::fopen(ProfilePath, "r");
    if (!file) return profiles;

    char line[96];
    while (profiles.size() < MaxProfiles && std::fgets(line, sizeof(line), file)) {
        char *tab = std::strchr(line, '\t');
        if (!tab) continue;
        *tab++ = '\0';
        tab[std::strcspn(tab, "\r\n")] = '\0';
        u8 parsed[4];
        if (line[0] && ParseIpv4(tab, parsed))
            profiles.push_back({line, tab});
    }
    std::fclose(file);
    return profiles;
}

bool SaveProfiles(const std::vector<DnsProfile> &profiles) {
    mkdir("sdmc:/3ds", 0777);
    mkdir(ProfileDirectory, 0777);
    FILE *file = std::fopen(ProfilePath, "w");
    if (!file) return false;
    for (const DnsProfile &profile : profiles)
        std::fprintf(file, "%s\t%s\n", profile.name.c_str(), profile.dns.c_str());
    const bool ok = std::fclose(file) == 0;
    return ok;
}

bool AddProfile(std::vector<DnsProfile> &profiles) {
    if (profiles.size() >= MaxProfiles) return false;
    char name[33]{};
    char dns[16]{};
    u8 parsed[4];
    if (!AskText("Profile name", name, sizeof(name))) return false;
    if (!AskText("Primary DNS", dns, sizeof(dns))) return false;
    if (!ParseIpv4(dns, parsed)) {
        ClearConsole();
        std::printf("Invalid IPv4 address.\n\nPress B to return.\n");
        while (aptMainLoop()) {
            PresentFrame();
            hidScanInput();
            if (hidKeysDown() & KEY_B) break;
        }
        return false;
    }
    SanitizeName(name);
    profiles.push_back({name, dns});
    if (!SaveProfiles(profiles)) {
        profiles.pop_back();
        return false;
    }
    return true;
}

int ChooseProfile(std::vector<DnsProfile> &profiles, int &target) {
    size_t selected = 0;
    bool redraw = true;
    while (aptMainLoop()) {
        const size_t addIndex = profiles.size();
        if (redraw) {
            ClearConsole();
            std::printf("3DS DNS Switcher\n\nSaved DNS profiles:\n\n");
            for (size_t i = 0; i < profiles.size(); ++i)
                std::printf("%c %s  [%s]\n", i == selected ? '>' : ' ',
                            profiles[i].name.c_str(), profiles[i].dns.c_str());
            std::printf("%c + Add DNS profile\n", selected == addIndex ? '>' : ' ');
            std::printf("\nTarget: %s\n", TargetName(target));
            std::printf("A: select  R: change target\n");
            std::printf("Y: delete profile      START: exit\n");
            redraw = false;
        }

        PresentFrame();
        hidScanInput();
        const u32 down = hidKeysDown();
        if ((down & KEY_DUP) && selected > 0) { --selected; redraw = true; }
        if ((down & KEY_DDOWN) && selected < addIndex) { ++selected; redraw = true; }
        if (down & KEY_A) {
            if (selected < profiles.size()) return static_cast<int>(selected);
            if (AddProfile(profiles)) selected = profiles.size() - 1;
            redraw = true;
        }
        if (down & KEY_R) {
            target = (target + 1) % 4;
            SaveTarget(target);
            redraw = true;
        }
        if ((down & KEY_Y) && selected < profiles.size()) {
            profiles.erase(profiles.begin() + selected);
            SaveProfiles(profiles);
            if (selected > profiles.size()) selected = profiles.size();
            redraw = true;
        }
        if (down & KEY_START) return -1;
    }
    return -1;
}

Result ApplyProfile(const DnsProfile &profile, int target, int &changed) {
    u8 primary[4];
    if (!ParseIpv4(profile.dns.c_str(), primary)) return -1;
    changed = 0;
    Result lastError = 0;
    const int first = target == 0 ? 0 : target - 1;
    const int last = target == 0 ? 2 : target - 1;
    for (int slot = first; slot <= last; ++slot) {
        const Result rc = SetSlotDns(slot, primary);
        if (R_SUCCEEDED(rc)) ++changed;
        else lastError = rc;
    }
    return changed > 0 ? 0 : lastError;
}
}

int main() {
    gfxInitDefault();
    gfxSetDoubleBuffering(GFX_TOP, false);
    consoleInit(GFX_TOP, nullptr);

    Result rc = cfguInit();
    if (R_FAILED(rc)) {
        std::printf("Could not open CFG service: %08lX\n", static_cast<unsigned long>(rc));
    } else {
        std::vector<DnsProfile> profiles = LoadProfiles();
        int target = LoadTarget();
        while (aptMainLoop()) {
            const int profileIndex = ChooseProfile(profiles, target);
            if (profileIndex < 0) break;

            int changed = 0;
            rc = ApplyProfile(profiles[profileIndex], target, changed);
            ClearConsole();
            if (R_SUCCEEDED(rc)) {
                std::printf("Applied %s to %d Wi-Fi slot%s.\n",
                            profiles[profileIndex].name.c_str(), changed,
                            changed == 1 ? "" : "s");
                PresentFrame();
                svcSleepThread(700000000LL);
                break; // Successful application exits automatically.
            }
            std::printf("DNS write failed: %08lX\n\nPress B to return.\n",
                        static_cast<unsigned long>(rc));
            while (aptMainLoop()) {
                PresentFrame();
                hidScanInput();
                if (hidKeysDown() & KEY_B) break;
            }
        }
        cfguExit();
    }

    gfxExit();
    return 0;
}
