# AirPlay Bridge

> Local custom test build 1.6.1, based on upstream 1.5.58; not an upstream release.
> The latency slider requests a playback buffer; it does not measure audible latency.

<p align="center">
  <img src="src/icons/preview_connected.png" width="128" alt="AirPlay Bridge">
</p>

**English** · [中文](README.zh-CN.md)

Streams Windows system audio to AirPlay 2 devices, from the tray. A free,
open-source replacement for TuneBlade after HomePod OS 27 — covers HomePod,
Apple TV and other receivers.

## 📦 Install

Download `AirPlay-Bridge-Setup-vX.Y.Z.exe` from
[Releases](https://github.com/RiverFlowsInUUU/AirPlay-Bridge/releases/latest)
and run it. Windows 10 / 11, 64-bit.

Installs to `Program Files\AirPlayBridge` with its runtime bundled —
**upgrades need no reconfiguration**, since the firewall rule and startup entry
follow that location. Windows may block it once: **More info** → **Run anyway**.

### 🔊 Local capture

This custom build prefers Windows process loopback, without installing a virtual cable or changing the default output.
It mutes the local default output while connected and restores it on disconnect. Unsupported systems fall back to endpoint capture.

## 🚀 First run

- Asks for interface language and whether to start at login (once each)
- Scans the local network, connects what it finds, and checks the firewall rule
  (one-click fix if it's missing)

## ✨ Features

**Audio & connection**

- 🆓 Free and open source (Apache-2.0)
- 🎵 AirPlay 2 with secure pairing (plus legacy AirPlay 1) — HomePod, Apple TV
  and other receivers
- 🎚 Lossless 16-bit/44.1 kHz; latency (1–300 ms), volume, quality and
  standby are all adjustable
- 💪 Glitch-resistant under load — audio threads run under MMCSS Pro Audio, and
  Windows 11 keeps the 1 ms timer even in the background

**Reliability**

- 🔄 Auto-reconnect — after drops, IP changes, or when another device takes over
- ⏰ Sleep/wake — streaming resumes on its own after the PC wakes
- 🛟 Force-kill safe — audio device and mute state are restored on next launch
- 💾 Config written atomically, auto-recovered from a backup if corrupted
- 🩺 Capture watchdog — an unplugged or busy device falls back or reconnects

**Everyday use**

- 🔎 Finds AirPlay devices, 🙈 hides the ones you don't want
- 🔇 Mutes this PC on connect, and restores the right device afterwards
- 😴 Standby when silent — screen off, back on the first sound
- 🖥 Starts at login, remembers settings, one instance at a time
- 🌐 English or Chinese; ℹ️ version and update check under **About**

## 🖱 Usage

Click the tray icon to open the dark panel; click again or press Esc to hide it.

| Control | Action |
|---|---|
| Device card | Connect; click the current device again to disconnect |
| Top switch | Pause and restore local sound; resume and mute the local output |
| Drag the header | Move the panel |
| Quit button | Exit and restore local sound |
| Volume slider | Change receiver volume immediately; arrow keys fine-tune |
| Latency button | Set the requested buffer and reconnect |
| Refresh | Scan and update the device list |
| Settings | Capture source, quality, standby, local mute, startup, language, device visibility, reconnect, diagnostics and logs |

The table below retains upstream configuration presets for reference; the custom panel exposes continuous volume and latency controls.

## 🎛 Adjustable

| Item | Presets | Range | Applies |
|---|---|---|---|
| Latency | 130 / 150 / **200** / 250 / 300 ms | continuous slider (1–300) | on reconnect |
| Volume | Off / 30% / 50% / 70% / 100% | continuous slider (0–100) | immediately |
| Quality | Fast / Standard / High / Very high / Maximum | continuous slider (0–100) | on reconnect |
| Standby when silent | 5 / 10 / 15 / 20 / 30 / 60 / 120 / 300 s | continuous slider (0–300) | immediately |

Bold marks the default. Latency below 250 ms risks crackling; 250 ms is the
tested sweet spot and 130 ms is experimental. Standby at 0 turns it off.

## 🔇 No sound

1. Confirm the tray menu shows **Connected**
2. Check the default playback device in Windows sound settings
3. Tray menu → More settings → Logs

## ⏳ Stuck on "Connecting"

The receiver calls back into this PC for clock sync, so the app needs an inbound
firewall rule. It checks this at startup:

| Step | Action |
|---|---|
| 1 | Tray menu → More settings → Connection check |
| 2 | Approve the admin prompt |

By hand: Windows Security → Firewall & network protection → Allow an app
through firewall → AirPlay Bridge → tick **Private**. Not listed? **Allow
another app** → **Browse**.

## ⚠️ Known limitations

- Without a virtual audio cable, sound stays on this PC's output device (muted
  by default — switchable from the menu)
- Changing latency or quality requires one reconnect
- Windows only
- A few seconds of silence after a drop

## 🔗 Related

- Report a problem → [Issues](https://github.com/RiverFlowsInUUU/AirPlay-Bridge/issues)
- Change log → [CHANGELOG.md](CHANGELOG.md)
- Development docs → [AGENTS.md](AGENTS.md) · [docs/](docs/)

## 🙏 Credits

- [airplay2-sender-cpp](https://github.com/akustikrausch/airplay2-sender-cpp) — AirPlay 2 protocol (Apache-2.0)
- [Mbed TLS](https://github.com/Mbed-TLS/mbedtls) — cryptography
- [VB-Audio Virtual Cable](https://vb-audio.com/Cable/) — virtual audio device

## 📄 License

Apache-2.0
