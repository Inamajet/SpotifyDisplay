
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>

char* SSID           = "SSID";
char* PASSWORD       = "PASSWORD";
const char* CLIENT_ID     = "YOUR_SPOTIFY_CLIENT_ID";
const char* CLIENT_SECRET = "YOUR_SPOTIFY_CLIENT_SECRET";

#define TFT_CS    D1
#define TFT_DC    D2
#define TFT_RST   D3
#define TFT_SCLK  D9
#define TFT_MOSI  D8

#define BTN_PREV  D4
#define BTN_PLAY  D5
#define BTN_SKIP  D6

#define IM_RED        0xC000
#define IM_GOLD       0xFEA0
#define IM_DARK       0x0000
#define IM_WHITE      0xFFFF
#define IM_GREY       0x4208
#define IM_LIGHTGREY  0x8410
#define IM_CYAN       0x07FF

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
Spotify sp(CLIENT_ID, CLIENT_SECRET);

String lastArtist  = "";
String lastTrack   = "";
bool lastPlaying   = false;
bool isPlaying     = false;

unsigned long lastBtnPress = 0;
const unsigned long DEBOUNCE_MS = 300;
const unsigned long POLL_INTERVAL = 2000;
unsigned long lastPoll = 0;

void drawFrame() {
    tft.fillScreen(IM_DARK);
    tft.fillRect(0, 0, 160, 22, IM_RED);
    tft.setTextColor(IM_GOLD);
    tft.setTextSize(1);
    tft.setCursor(4, 7);
    tft.print("STARK INDUSTRIES");
    tft.drawFastHLine(0, 22, 160, IM_GOLD);
    tft.setTextColor(IM_GOLD);
    tft.setTextSize(1);
    tft.setCursor(4, 28);
    tft.print("NOW PLAYING");
    tft.fillRect(0, 104, 160, 24, IM_GREY);
    tft.drawFastHLine(0, 103, 160, IM_GOLD);
    tft.setTextColor(IM_GOLD);
    tft.setTextSize(1);
    tft.setCursor(8, 111);   tft.print("|<");
    tft.setCursor(72, 111);  tft.print(">||");
    tft.setCursor(136, 111); tft.print(">|");
    tft.drawCircle(153, 119, 4, IM_CYAN);
    tft.drawCircle(153, 119, 2, IM_CYAN);
    tft.drawPixel(153, 119, IM_CYAN);
}

void clearTrackArea() {
    tft.fillRect(0, 40, 160, 62, IM_DARK);
}

void printWrapped(String text, int x, int y, int maxWidth, uint16_t color, uint8_t size) {
    tft.setTextColor(color);
    tft.setTextSize(size);
    int charW = 6 * size;
    int charsPerLine = maxWidth / charW;
    if ((int)text.length() <= charsPerLine) {
        tft.setCursor(x, y);
        tft.print(text);
    } else {
        tft.setCursor(x, y);
        tft.print(text.substring(0, charsPerLine));
        String line2 = text.substring(charsPerLine);
        if ((int)line2.length() > charsPerLine) {
            line2 = line2.substring(0, charsPerLine - 3) + "...";
        }
        tft.setCursor(x, y + (9 * size));
        tft.print(line2);
    }
}

void displayTrackInfo(String artist, String track, bool playing) {
    clearTrackArea();
    printWrapped(artist, 4, 42, 152, IM_GOLD, 1);
    printWrapped(track, 4, 62, 152, IM_WHITE, 1);
    tft.fillRect(4, 88, 80, 12, IM_DARK);
    tft.setTextColor(playing ? IM_CYAN : IM_LIGHTGREY);
    tft.setTextSize(1);
    tft.setCursor(4, 89);
    tft.print(playing ? ">> PLAYING" : "|| PAUSED");
    tft.fillRect(58, 104, 50, 24, IM_GREY);
    tft.drawFastHLine(58, 103, 50, IM_GOLD);
    tft.setTextColor(IM_GOLD);
    tft.setTextSize(1);
    tft.setCursor(72, 111);
    tft.print(playing ? ">||" : " >");
}

void showStatus(String line1, String line2 = "", uint16_t color = IM_GOLD) {
    tft.fillScreen(IM_DARK);
    tft.fillRect(0, 0, 160, 22, IM_RED);
    tft.setTextColor(IM_GOLD);
    tft.setTextSize(1);
    tft.setCursor(4, 7);
    tft.print("STARK INDUSTRIES");
    tft.drawFastHLine(0, 22, 160, IM_GOLD);
    tft.setTextColor(color);
    tft.setTextSize(1);
    tft.setCursor(4, 36);
    tft.print(line1);
    if (line2.length() > 0) {
        tft.setCursor(4, 52);
        tft.print(line2);
    }
    for (int i = 0; i < 3; i++) {
        tft.fillCircle(60 + i * 16, 80, 4, IM_CYAN);
    }
}

void flashButtonFeedback(int btnX) {
    tft.fillRect(btnX, 104, 40, 24, IM_RED);
    delay(80);
    tft.fillRect(btnX, 104, 40, 24, IM_GREY);
    tft.drawFastHLine(btnX, 103, 40, IM_GOLD);
}

void setup() {
    Serial.begin(115200);
    pinMode(BTN_PREV, INPUT_PULLUP);
    pinMode(BTN_PLAY, INPUT_PULLUP);
    pinMode(BTN_SKIP, INPUT_PULLUP);
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(IM_DARK);
    showStatus("INITIALIZING...", "MARK L FIRMWARE", IM_GOLD);
    delay(800);
    showStatus("CONNECTING TO", "WIFI...", IM_GOLD);
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(1000);
        Serial.print(".");
        attempts++;
        tft.setTextColor(IM_CYAN);
        tft.setTextSize(1);
        tft.setCursor(4 + (attempts % 20) * 7, 68);
        tft.print(".");
    }
    if (WiFi.status() != WL_CONNECTED) {
        showStatus("WIFI FAILED", "Check credentials", IM_RED);
        Serial.println("\nWiFi connection failed!");
        while(true) delay(1000);
    }
    Serial.println("\nConnected!");
    showStatus("WIFI CONNECTED", WiFi.localIP().toString(), IM_CYAN);
    delay(1200);
    showStatus("CONNECTING TO", "SPOTIFY...", IM_GOLD);
    tft.setTextColor(IM_LIGHTGREY);
    tft.setTextSize(1);
    tft.setCursor(4, 72);
    tft.print("Visit IP to auth:");
    tft.setCursor(4, 84);
    tft.print(WiFi.localIP().toString());
    sp.begin();
    while (!sp.is_auth()) {
        sp.handle_client();
    }
    Serial.println("Spotify authenticated!");
    drawFrame();
    displayTrackInfo("Connecting...", "Fetching track...", false);
}

void loop() {
    sp.handle_client();
    unsigned long now = millis();
    if (now - lastBtnPress > DEBOUNCE_MS) {
        if (digitalRead(BTN_PREV) == LOW) {
            lastBtnPress = now;
            flashButtonFeedback(0);
            sp.previous();
            lastTrack = "";
            lastArtist = "";
        }
        if (digitalRead(BTN_PLAY) == LOW) {
            lastBtnPress = now;
            flashButtonFeedback(58);
            sp.start_resume_playback();
            delay(300);
            isPlaying = !isPlaying;
            displayTrackInfo(lastArtist, lastTrack, isPlaying);
        }
        if (digitalRead(BTN_SKIP) == LOW) {
            lastBtnPress = now;
            flashButtonFeedback(118);
            sp.skip();
            lastTrack = "";
            lastArtist = "";
        }
    }
    if (now - lastPoll > POLL_INTERVAL) {
        lastPoll = now;
        String currentArtist = sp.current_artist_names();
        String currentTrack  = sp.current_track_name();
        bool currentPlaying  = sp.is_playing();
        bool artistValid = currentArtist != "Something went wrong" && !currentArtist.isEmpty();
        bool trackValid  = currentTrack  != "Something went wrong" && currentTrack != "null" && !currentTrack.isEmpty();
        if (!artistValid || !trackValid) return;
        if (currentArtist.length() > 30) currentArtist = currentArtist.substring(0, 27) + "...";
        if (currentTrack.length()  > 30) currentTrack  = currentTrack.substring(0, 27)  + "...";
        bool trackChanged   = (currentTrack   != lastTrack);
        bool artistChanged  = (currentArtist  != lastArtist);
        bool playingChanged = (currentPlaying  != lastPlaying);
        if (trackChanged || artistChanged || playingChanged) {
            lastTrack   = currentTrack;
            lastArtist  = currentArtist;
            lastPlaying = currentPlaying;
            isPlaying   = currentPlaying;
            if (trackChanged || artistChanged) drawFrame();
            displayTrackInfo(lastArtist, lastTrack, isPlaying);
        }
    }
}
