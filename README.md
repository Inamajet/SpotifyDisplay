A simple display that shows the song you're currently playing on Spotify. It lets you skip and pause too.

I made this project because I thought it was cool and I get to customize it to my liking.

> Built as a [Hack Club Stasis](https://stasis.hackclub.com) starter project

## Features

- Displays current song title + artist in real time
- Live progress bar
- Physical buttons for play/pause, skip, and previous
- Connects to Spotify over WiFi
- Custom 3D-printed case with heat-set inserts

## Parts

| Part | What it does | Qty | Cost | Link |
|------|-------------|-----|------|------|
| XIAO ESP32-C3 | Main microcontroller | 1 | $9.90 | [Amazon](https://www.amazon.com/Seeed-Studio-XIAO-ESP32C3-Microcontroller/dp/B0B94JZ2YF) |
| 1.8" TFT LCD (ST7735) | Displays what's playing | 1 | $8.18 | [Amazon](https://www.amazon.com/Bewinner-Resolution-Interface-Full-Color-Controller/dp/B083NYBN4Q) |
| M3 Heat-Set Inserts | Assemble 3D printed case | 4 | $3.91 | [Amazon](https://www.amazon.com/ZWMSSLL-Heat-Set-Threaded-M3x12x5mm-Components/dp/B0DFWXCFZM) |
| Keyboard Switches | Buttons (play/skip/back) | 3 | — | — |
| Jumper Wires | Connections | ~10 | — | — |

**Total: ~$22**

## Wiring

```
ESP32-C3          ST7735 TFT
─────────         ──────────
3V3         →     VCC
GND         →     GND
SCK (GPIO8) →     SCK
MOSI(GPIO10)→     SDA
GPIO3       →     CS
GPIO4       →     DC
GPIO5       →     RST
3V3         →     LED

Buttons: GPIO2 (prev) · GPIO6 (play/pause) · GPIO7 (next)
```

## Case

3D-printed two-piece enclosure. Screen and ESP32 sit inside the base, lid bolts on with M3 heat-set inserts.

STL files in the `case/` folder.

## Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Install libraries: [SpotifyEsp32](https://github.com/FinianLandes/SpotifyEsp32), Adafruit_ST7735, Adafruit_GFX
3. Create a Spotify app at [developer.spotify.com](https://developer.spotify.com)
4. Fill in your WiFi + Spotify credentials in `Code/Code.ino`
5. Upload to ESP32, follow the on-screen IP to authenticate

## Skills Learned

- **WiFi** — ESP32 making HTTPS API calls
- **SPI** — Driving the ST7735 TFT display
- **CAD** — Designed a custom enclosure for 3D printing


## Credits

Inspired by the Spotify Car Thing and [Dongathan-Jong/SpotifyDisplay](https://github.com/Dongathan-Jong/SpotifyDisplay). Built for [Hack Club Stasis](https://stasis.hackclub.com).
