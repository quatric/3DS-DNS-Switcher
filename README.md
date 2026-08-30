# 3DS DNS Switcher

A Nintendo 3DS HOME Menu application for quickly switching the DNS used by
existing Wi-Fi connections.

## Features

- Saves named primary-DNS profiles on the SD card.
- Shows the profile list on every launch, including first setup.
- Press `R` to choose and remember the target: all configured Wi-Fi slots or
  connection slot 1, 2, or 3.
- Press `A` to apply the highlighted profile immediately.
- Always uses `1.1.1.1` as the secondary DNS.
- Exits automatically after a successful change.
- Preserves SSIDs, passwords, DHCP, proxy, and other connection settings.

Profiles are stored at `sdmc:/3ds/dns-switcher/profiles.txt`, and the target is
stored at `sdmc:/3ds/dns-switcher/settings.txt`.

## Installation

Install [`standalone/3DS-DNS-Switcher.cia`](standalone/3DS-DNS-Switcher.cia)
with FBI. It appears directly on the HOME Menu and does not need Luma's plugin
loader.

## Controls

- D-Pad Up/Down: choose a profile or **Add DNS profile**
- `A`: select/apply
- `R`: cycle and save the Wi-Fi-slot target
- `Y`: delete the highlighted profile
- `START`: exit without applying

## Building

Install a current devkitARM/libctru toolchain and run `make` from the
`standalone` directory. This produces `3DS-DNS-Switcher.elf` and
`3DS-DNS-Switcher.3dsx`. Packaging the HOME Menu CIA additionally requires
`bannertool` and `makerom`; the supplied `app.rsf`, artwork, and audio are the
inputs used for the included CIA.

The CFG Wi-Fi block layout and CRC handling are based on LiquidFenrir's
[WifiManager](https://github.com/LiquidFenrir/WifiManager/tree/master/source).

## Important

This software directly modifies the console's configured Wi-Fi blocks. Keep a
backup of your network settings and use it at your own risk.
