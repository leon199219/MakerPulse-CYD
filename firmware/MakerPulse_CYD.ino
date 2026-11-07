// MakerPulse CYD 2026.11.07f
#define MAKERPULSE_FW "2026.11.07f"
/*
 * ========== WHAT DO YOU NEED TO CHANGE? ==========
 *
 * A) Flash this sketch and follow the screen
 *    No config.h needed. The CYD opens Wi-Fi "MakerPulse-...."
 *    (password makerpulse). Open http://192.168.4.1 and fill in
 *    Wi-Fi, MakerWorld, Telegram and MQTT. Later: http://<ip>/setup
 *
 * B) Zip from the MakerPulse configurator
 *    Nothing. Wi-Fi, MakerWorld ID, Telegram and MQTT are already in config.h.
 *    Choose board "ESP32 Dev Module" and click Upload.
 *
 * C) config.h by hand
 *    Copy config.h.example to config.h and fill it in.
 *    A saved setup page overrides config.h.
 *
 * Arduino IDE (once):
 *    Board: ESP32 Dev Module · Flash 4MB · Partition "Default 4MB with spiffs"
 *    Libraries: LovyanGFX, ArduinoJson 7, PubSubClient
 *    Put the folder at C:\MakerPulse_CYD\ (no spaces, no OneDrive).
 *
 * Keep mp_types.h in this folder. Do not edit the rest of this sketch.
 * ============================================
 */
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <PubSubClient.h>
#include <string.h>
#if defined(__has_include)
#if __has_include("config.h")
#include "config.h"
#endif
#else
#include "config.h"
#endif
#include "mp_types.h"

// --- Manual settings: only used when config.h is missing. ---
#ifndef WIFI_SSID
#define WIFI_SSID "JOUW_WIFI_NAAM"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "JOUW_WIFI_WACHTWOORD"
#endif
#ifndef MAKERWORLD_UID
#define MAKERWORLD_UID 0UL
#endif
#ifndef MAKER_NAME
#define MAKER_NAME "Maker"
#endif
#ifndef TELEGRAM_BOT_TOKEN
#define TELEGRAM_BOT_TOKEN ""
#endif
#ifndef TELEGRAM_CHAT_ID
#define TELEGRAM_CHAT_ID ""
#endif
#ifndef POLL_INTERVAL_SEC
#define POLL_INTERVAL_SEC 300
#endif
#ifndef TELEGRAM_INTERVAL_SEC
#define TELEGRAM_INTERVAL_SEC 0
#endif
#ifndef CYD_INVERT
#define CYD_INVERT 0
#endif
#ifndef CYD_ROTATION
#define CYD_ROTATION 1
#endif
#ifndef CYD_OFFSET_ROTATION
#define CYD_OFFSET_ROTATION 2
#endif
#ifndef CYD_PANEL_ST7789
#define CYD_PANEL_ST7789 0
#endif
#ifndef CYD_RGB_ORDER
#define CYD_RGB_ORDER 0
#endif
#ifndef BACKLIGHT_MODE
#define BACKLIGHT_MODE 0
#endif
#ifndef BACKLIGHT_IDLE_SEC
#define BACKLIGHT_IDLE_SEC 600
#endif
#ifndef MQTT_HOST
#define MQTT_HOST ""
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#ifndef MQTT_USER
#define MQTT_USER ""
#endif
#ifndef MQTT_PASS
#define MQTT_PASS ""
#endif
#ifndef NOTIFY_DOWNLOADS
#define NOTIFY_DOWNLOADS 1
#endif
#ifndef NOTIFY_LIKES
#define NOTIFY_LIKES 1
#endif
#ifndef NOTIFY_PRINTS
#define NOTIFY_PRINTS 1
#endif
#ifndef NOTIFY_BOOSTS
#define NOTIFY_BOOSTS 1
#endif
#ifndef NOTIFY_COLLECTIONS
#define NOTIFY_COLLECTIONS 1
#endif
#ifndef NOTIFY_COMMENTS
#define NOTIFY_COMMENTS 1
#endif
#ifndef NOTIFY_FOLLOWERS
#define NOTIFY_FOLLOWERS 1
#endif

static const int PIN_LED_R = 4;
static const int PIN_LED_G = 16;
static const int PIN_LED_B = 17;

static const uint16_t COL_BG = 0x1082;
static const uint16_t COL_HEAD = 0x18C3;
static const uint16_t COL_FG = 0xEF5C;
static const uint16_t COL_MUTED = 0x9CD2;
static const uint16_t COL_LINE = 0x2966;
static const uint16_t COL_UP = 0x864C;
static const uint16_t COL_ERR = 0xD26A;
static const uint16_t COL_GREEN = 0x0568;

class CydDisplay : public lgfx::LGFX_Device {
#if CYD_PANEL_ST7789
  lgfx::Panel_ST7789 _panel_instance;
#else
  lgfx::Panel_ILI9341 _panel_instance;
#endif
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;
  lgfx::Touch_XPT2046 _touch_instance;

 public:
  CydDisplay(void) {
    {
      auto cfg = _bus_instance.config();
#if defined(HSPI_HOST)
      cfg.spi_host = HSPI_HOST;
#else
      cfg.spi_host = SPI2_HOST;
#endif
      cfg.spi_mode = 0;
      cfg.freq_write = 27000000;
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = 1;
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 13;
      cfg.pin_miso = 12;
      cfg.pin_dc = 2;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = 15;
      cfg.pin_rst = -1;
      cfg.pin_busy = -1;
      cfg.memory_width = 240;
      cfg.memory_height = 320;
      cfg.panel_width = 240;
      cfg.panel_height = 320;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation = CYD_OFFSET_ROTATION;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits = 1;
      cfg.readable = false;
      cfg.invert = CYD_INVERT;
      cfg.rgb_order = CYD_RGB_ORDER;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = 21;
      cfg.invert = false;
      cfg.freq = 44100;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 300;
      cfg.x_max = 3900;
      cfg.y_min = 3700;
      cfg.y_max = 200;
      cfg.pin_int = 36;
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;
#if defined(VSPI_HOST)
      cfg.spi_host = VSPI_HOST;
#else
      cfg.spi_host = SPI3_HOST;
#endif
      cfg.freq = 2500000;
      cfg.pin_sclk = 25;
      cfg.pin_mosi = 32;
      cfg.pin_miso = 39;
      cfg.pin_cs = 33;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
    setPanel(&_panel_instance);
  }
};

static const char MW_UA[] =
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";

CydDisplay tft;
Preferences prefs;
Stats current = {0, 0, 0, 0, 0, 0, 0};
Stats lastSent = {0, 0, 0, 0, 0, 0, 0};
ModelSnap models[MAX_MODELS];
int modelCount = 0;
AllSnap allSnaps[MAX_ALL_MODELS];
AllSnap prevAllSnaps[MAX_ALL_MODELS];
int allSnapCount = 0;
int prevAllSnapCount = 0;
ModelAlert alerts[MAX_ALERTS];
int alertCount = 0;
bool hasBaseline = false;
time_t lastDigestUnix = 0;
unsigned long lastDigestMs = 0;
char makerName[24] = MAKER_NAME;
char cfgSsid[33] = WIFI_SSID;
char cfgPass[65] = WIFI_PASSWORD;
char cfgToken[80] = TELEGRAM_BOT_TOKEN;
char cfgChat[24] = TELEGRAM_CHAT_ID;
char cfgMqttHost[64] = MQTT_HOST;
char cfgMqttUser[32] = MQTT_USER;
char cfgMqttPass[64] = MQTT_PASS;
uint32_t cfgUid = (uint32_t)MAKERWORLD_UID;
char cfgQuery[160] = "";
bool cfgAutoName = false;
int cfgPoll = POLL_INTERVAL_SEC;
int cfgTgEvery = TELEGRAM_INTERVAL_SEC;
int cfgBlMode = BACKLIGHT_MODE;
int cfgRot = CYD_ROTATION;
bool cfgMirror = false;
uint16_t cfgMqttPort = MQTT_PORT;
uint8_t cfgNotify = (NOTIFY_DOWNLOADS ? 1 : 0) | (NOTIFY_LIKES ? 2 : 0) | (NOTIFY_PRINTS ? 4 : 0) |
                    (NOTIFY_BOOSTS ? 8 : 0) | (NOTIFY_COLLECTIONS ? 16 : 0) | (NOTIFY_COMMENTS ? 32 : 0) |
                    (NOTIFY_FOLLOWERS ? 64 : 0);
bool portalMode = false;
DNSServer dnsServer;
static const char AP_PASS[] = "makerpulse";
char lastError[40] = "";
bool lastOk = false;
int censusFound = 0;
int censusTotal = 0;
bool commentsReady = false;

WebServer www(80);
WiFiClient mqttNet;
PubSubClient mqtt(mqttNet);

bool blOn = true;
uint8_t blBright = 255;
unsigned long lastActivityMs = 0;
bool autoBacklight() { return cfgBlMode == 1; }
char mqttClientId[28] = "";
char topicCmd[48] = "";
char topicState[48] = "";
char topicAvail[48] = "";
char topicStats[48] = "";
char topicDisc[64] = "";

void serviceNet();

bool mqttEnabled() { return cfgMqttHost[0] != '\0'; }

bool httpsGet(const String& url, String& body) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(20000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(client, url)) return false;
  http.addHeader("Accept", "application/json");
  http.addHeader("User-Agent", MW_UA);
  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }
  body = http.getString();
  http.end();
  return body.length() > 0;
}

bool httpsPostJson(const String& url, const String& payload, String& response) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(15000);
  if (!http.begin(client, url)) return false;
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(payload);
  response = http.getString();
  http.end();
  return code >= 200 && code < 300;
}

void ledOff() {
  digitalWrite(PIN_LED_R, HIGH);
  digitalWrite(PIN_LED_G, HIGH);
  digitalWrite(PIN_LED_B, HIGH);
}

void ledRgb(bool r, bool g, bool b) {
  digitalWrite(PIN_LED_R, r ? LOW : HIGH);
  digitalWrite(PIN_LED_G, g ? LOW : HIGH);
  digitalWrite(PIN_LED_B, b ? LOW : HIGH);
}

String fmtNum(long n) {
  String s = String(n);
  String out;
  int len = s.length();
  for (int i = 0; i < len; i++) {
    if (i > 0 && (len - i) % 3 == 0) out += '.';
    out += s[i];
  }
  return out;
}

String fmtDelta(long d) {
  if (d > 0) return String("+") + fmtNum(d);
  if (d < 0) return String("-") + fmtNum(-d);
  return "";
}

String clockStr() {
  struct tm t;
  if (!getLocalTime(&t, 20)) return "--:--";
  char buf[8];
  strftime(buf, sizeof(buf), "%H:%M", &t);
  return String(buf);
}

void drawStatus(const char* title, const char* detail) {
  int W = tft.width();
  int H = tft.height();
  tft.fillScreen(COL_BG);
  tft.setTextDatum(middle_center);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_FG, COL_BG);
  tft.drawString(title, W / 2, H / 2 - 14);
  tft.setTextColor(COL_MUTED, COL_BG);
  tft.drawString(detail, W / 2, H / 2 + 10);
}

void drawRow(int y, int rowH, int W, const char* label, long value, long delta) {
  tft.fillRect(0, y, W, rowH, COL_BG);
  int cy = y + rowH / 2;
  tft.setTextDatum(middle_left);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_MUTED, COL_BG);
  tft.drawString(label, 8, cy);

  String num = fmtNum(value);
  String d = fmtDelta(delta);
  tft.setTextDatum(middle_right);
  if (d.length()) {
    tft.setTextColor(delta > 0 ? COL_UP : COL_ERR, COL_BG);
    tft.drawString(d, W - 8, cy);
    tft.setTextColor(COL_FG, COL_BG);
    tft.drawString(num, W - 52, cy);
  } else {
    tft.setTextColor(COL_FG, COL_BG);
    tft.drawString(num, W - 8, cy);
  }
}

void drawDashboard() {
  int W = tft.width();
  int H = tft.height();
  int headH = H >= 280 ? 54 : 46;
  int footH = H >= 280 ? 26 : 20;
  int rowH = (H - headH - footH) / 6;

  tft.fillScreen(COL_BG);
  tft.fillRect(0, 0, W, headH, COL_HEAD);

  tft.setTextDatum(top_left);
  tft.setFont(&fonts::Font0);
  tft.setTextColor(COL_MUTED, COL_HEAD);
  tft.drawString("MAKERPULSE  CYD", 8, 6);

  tft.setTextDatum(top_right);
  tft.drawString(clockStr() + (WiFi.status() == WL_CONNECTED ? "  WiFi" : "  --"), W - 8, 6);

  long df = hasBaseline ? current.followers - lastSent.followers : 0;
  String fan = fmtNum(current.followers);
  if (df > 0) {
    fan += " +";
    fan += fmtNum(df);
  }
  fan += " volgers";
  tft.setFont(&fonts::Font0);
  int fanW = tft.textWidth(fan);
  if (fanW > W / 2) {
    fan = fmtNum(current.followers);
    if (df > 0) {
      fan += " +";
      fan += fmtNum(df);
    }
    fanW = tft.textWidth(fan);
  }
  tft.setTextDatum(middle_right);
  tft.setTextColor(df > 0 ? COL_UP : COL_MUTED, COL_HEAD);
  tft.drawString(fan, W - 8, headH - 14);

  tft.setTextDatum(middle_left);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_FG, COL_HEAD);
  String name = makerName;
  int nameMax = W - 16 - fanW - 6;
  if (nameMax < 16) nameMax = 16;
  while (name.length() > 1 && tft.textWidth(name) > nameMax) name.remove(name.length() - 1);
  tft.drawString(name, 8, headH - 14);

  tft.drawFastHLine(0, headH, W, COL_LINE);

  long dd = hasBaseline ? current.downloads - lastSent.downloads : 0;
  long dl = hasBaseline ? current.likes - lastSent.likes : 0;
  long dp = hasBaseline ? current.prints - lastSent.prints : 0;
  long db = hasBaseline ? current.boosts - lastSent.boosts : 0;
  long dc = hasBaseline ? current.collections - lastSent.collections : 0;
  long dm = hasBaseline ? current.comments - lastSent.comments : 0;

  int y = headH;
  drawRow(y, rowH, W, "Downloads", current.downloads, dd);
  y += rowH;
  drawRow(y, rowH, W, "Likes", current.likes, dl);
  y += rowH;
  drawRow(y, rowH, W, "Prints", current.prints, dp);
  y += rowH;
  drawRow(y, rowH, W, "Boosts", current.boosts, db);
  y += rowH;
  drawRow(y, rowH, W, "Collecties", current.collections, dc);
  y += rowH;
  drawRow(y, rowH, W, "Comments", current.comments, dm);

  int footY = H - footH;
  tft.drawFastHLine(0, footY, W, COL_LINE);
  tft.setFont(&fonts::Font0);
  tft.setTextDatum(middle_left);
  if (lastOk) {
    tft.setTextColor(commentsReady ? COL_MUTED : COL_ERR, COL_BG);
    char foot[56];
    if (censusTotal > 0) {
      snprintf(foot, sizeof(foot), "%s  %d/%d  %ds", clockStr().c_str(), censusFound, censusTotal,
               cfgPoll);
    } else {
      snprintf(foot, sizeof(foot), "check %s  ·  %ds", clockStr().c_str(), cfgPoll);
    }
    tft.drawString(foot, 8, footY + footH / 2);
  } else {
    tft.setTextColor(COL_ERR, COL_BG);
    tft.drawString(lastError[0] ? lastError : "check mislukt", 8, footY + footH / 2);
  }
  tft.setTextDatum(middle_right);
  tft.setTextColor(COL_MUTED, COL_BG);
  if (WiFi.status() == WL_CONNECTED) {
    tft.drawString(WiFi.localIP().toString(), W - 8, footY + footH / 2);
  } else {
    bool tg = cfgToken[0] && cfgChat[0];
    tft.drawString(tg ? "Telegram" : "geen TG", W - 8, footY + footH / 2);
  }
}

int collectDesignIds(JsonDocument& doc, long* ids, int maxn) {
  int n = 0;
  JsonArray info = doc["personal"]["designsInfo"].as<JsonArray>();
  if (!info.isNull()) {
    for (JsonObject o : info) {
      long id = o["id"] | 0;
      if (id <= 0) continue;
      bool seen = false;
      for (int i = 0; i < n; i++) {
        if (ids[i] == id) {
          seen = true;
          break;
        }
      }
      if (seen) continue;
      ids[n++] = id;
      if (n >= maxn) return n;
    }
  }
  JsonArray pinned = doc["personal"]["pinnedDesigns"].as<JsonArray>();
  if (!pinned.isNull()) {
    for (JsonVariant v : pinned) {
      long id = v.as<long>();
      if (id <= 0) continue;
      bool seen = false;
      for (int i = 0; i < n; i++) {
        if (ids[i] == id) {
          seen = true;
          break;
        }
      }
      if (seen) continue;
      ids[n++] = id;
      if (n >= maxn) return n;
    }
  }
  return n;
}

const AllSnap* findPrevAll(uint32_t id) {
  for (int i = 0; i < prevAllSnapCount; i++) {
    if (prevAllSnaps[i].id == id) return &prevAllSnaps[i];
  }
  return nullptr;
}

void queueAlert(const char* title, long dDl, long dLk, long dPr, long dBs, long dCo, long dCm) {
  if (!hasBaseline) return;
  if (!(cfgNotify & 1)) dDl = 0;
  if (!(cfgNotify & 2)) dLk = 0;
  if (!(cfgNotify & 4)) dPr = 0;
  if (!(cfgNotify & 8)) dBs = 0;
  if (!(cfgNotify & 16)) dCo = 0;
  if (!(cfgNotify & 32)) dCm = 0;
  if (!dDl && !dLk && !dPr && !dBs && !dCo && !dCm) return;
  if (alertCount >= MAX_ALERTS) return;
  strncpy(alerts[alertCount].title, title, sizeof(alerts[alertCount].title) - 1);
  alerts[alertCount].title[sizeof(alerts[alertCount].title) - 1] = 0;
  alerts[alertCount].dDownloads = dDl;
  alerts[alertCount].dLikes = dLk;
  alerts[alertCount].dPrints = dPr;
  alerts[alertCount].dBoosts = dBs;
  alerts[alertCount].dCollections = dCo;
  alerts[alertCount].dComments = dCm;
  alertCount++;
}

void rememberAndAlert(long id, const char* title, long downloads, long likes, long prints,
                      long boosts, long collections, long comments) {
  if (id > 0 && allSnapCount < MAX_ALL_MODELS) {
    allSnaps[allSnapCount].id = (uint32_t)id;
    allSnaps[allSnapCount].downloads = (uint32_t)downloads;
    allSnaps[allSnapCount].likes = (uint32_t)likes;
    allSnaps[allSnapCount].prints = (uint32_t)prints;
    allSnaps[allSnapCount].boosts = (uint32_t)boosts;
    allSnaps[allSnapCount].collections = (uint32_t)collections;
    allSnaps[allSnapCount].comments = (uint32_t)comments;
    allSnapCount++;
  }
  const AllSnap* prev = id > 0 ? findPrevAll((uint32_t)id) : nullptr;
  if (!prev) return;
  queueAlert(title && title[0] ? title : "Model", downloads - (long)prev->downloads,
             likes - (long)prev->likes, prints - (long)prev->prints, boosts - (long)prev->boosts,
             collections - (long)prev->collections, comments - (long)prev->comments);
}

void fillSnapFromPub(const PubHit& h, ModelSnap& out) {
  out.id = (long)h.id;
  strncpy(out.title, h.title, sizeof(out.title) - 1);
  out.title[sizeof(out.title) - 1] = 0;
  out.downloads = (long)h.downloads;
  out.prints = (long)h.prints;
  out.likes = (long)h.likes;
  out.collections = (long)h.collections;
  out.comments = (long)h.comments;
  out.boosts = (long)h.boosts;
}

void jSkipWs(JsonCur& j) {
  while (j.i < j.n) {
    char c = j.s[j.i];
    if (c != ' ' && c != '\n' && c != '\r' && c != '\t') break;
    j.i++;
  }
}

bool jSkipString(JsonCur& j, char* out, int outMax) {
  if (j.i >= j.n || j.s[j.i] != '"') return false;
  j.i++;
  int o = 0;
  if (out && outMax > 0) out[0] = 0;
  while (j.i < j.n) {
    char c = j.s[j.i++];
    if (c == '"') {
      if (out && outMax > 0) {
        if (o >= outMax) o = outMax - 1;
        out[o] = 0;
      }
      return true;
    }
    if (c == '\\') {
      if (j.i >= j.n) return false;
      char e = j.s[j.i++];
      if (e == 'u') {
        if (j.i + 4 > j.n) return false;
        unsigned cp = 0;
        for (int k = 0; k < 4; k++) {
          char h = j.s[j.i++];
          cp <<= 4;
          if (h >= '0' && h <= '9') cp |= (unsigned)(h - '0');
          else if (h >= 'a' && h <= 'f') cp |= (unsigned)(h - 'a' + 10);
          else if (h >= 'A' && h <= 'F') cp |= (unsigned)(h - 'A' + 10);
          else return false;
        }
        char tmp[3];
        int tn = 0;
        if (cp < 0x80) {
          tmp[tn++] = (char)cp;
        } else if (cp < 0x800) {
          tmp[tn++] = (char)(0xC0 | (cp >> 6));
          tmp[tn++] = (char)(0x80 | (cp & 0x3F));
        } else {
          tmp[tn++] = (char)(0xE0 | (cp >> 12));
          tmp[tn++] = (char)(0x80 | ((cp >> 6) & 0x3F));
          tmp[tn++] = (char)(0x80 | (cp & 0x3F));
        }
        for (int t = 0; t < tn; t++) {
          if (out && o < outMax - 1) out[o++] = tmp[t];
        }
        continue;
      }
      char d = e;
      if (e == 'n') d = '\n';
      else if (e == 'r') d = '\r';
      else if (e == 't') d = '\t';
      if (out && o < outMax - 1) out[o++] = d;
      continue;
    }
    if (out && o < outMax - 1) out[o++] = c;
  }
  return false;
}

bool jParseU32(JsonCur& j, uint32_t& v) {
  jSkipWs(j);
  if (j.i + 4 <= j.n && j.s[j.i] == 'n' && j.s[j.i + 1] == 'u' && j.s[j.i + 2] == 'l' &&
      j.s[j.i + 3] == 'l') {
    j.i += 4;
    v = 0;
    return true;
  }
  if (j.i < j.n && j.s[j.i] == '-') return false;
  if (j.i >= j.n || j.s[j.i] < '0' || j.s[j.i] > '9') return false;
  uint32_t n = 0;
  while (j.i < j.n && j.s[j.i] >= '0' && j.s[j.i] <= '9') {
    uint32_t d = (uint32_t)(j.s[j.i] - '0');
    if (n > (0xFFFFFFFFu - d) / 10u) return false;
    n = n * 10u + d;
    j.i++;
  }
  if (j.i < j.n && j.s[j.i] == '.') {
    j.i++;
    while (j.i < j.n && j.s[j.i] >= '0' && j.s[j.i] <= '9') j.i++;
  }
  if (j.i < j.n && (j.s[j.i] == 'e' || j.s[j.i] == 'E')) {
    j.i++;
    if (j.i < j.n && (j.s[j.i] == '+' || j.s[j.i] == '-')) j.i++;
    while (j.i < j.n && j.s[j.i] >= '0' && j.s[j.i] <= '9') j.i++;
  }
  v = n;
  return true;
}

bool jSkipValue(JsonCur& j) {
  jSkipWs(j);
  if (j.i >= j.n) return false;
  char c = j.s[j.i];
  if (c == '"') return jSkipString(j, nullptr, 0);
  if (c == '{' || c == '[') {
    int depth = 0;
    bool inStr = false;
    bool esc = false;
    for (; j.i < j.n; j.i++) {
      char ch = j.s[j.i];
      if (inStr) {
        if (esc) {
          esc = false;
          continue;
        }
        if (ch == '\\') {
          esc = true;
          continue;
        }
        if (ch == '"') inStr = false;
        continue;
      }
      if (ch == '"') {
        inStr = true;
        continue;
      }
      if (ch == '{' || ch == '[') {
        depth++;
      } else if (ch == '}' || ch == ']') {
        depth--;
        if (depth == 0) {
          j.i++;
          return true;
        }
        if (depth < 0) return false;
      }
    }
    return false;
  }
  if (c == 't' || c == 'f' || c == 'n') {
    while (j.i < j.n && j.s[j.i] >= 'a' && j.s[j.i] <= 'z') j.i++;
    return true;
  }
  uint32_t tmp;
  return jParseU32(j, tmp);
}

bool walkHit(JsonCur& j, PubHit& hit) {
  memset(&hit, 0, sizeof(hit));
  strncpy(hit.title, "Model", sizeof(hit.title) - 1);
  jSkipWs(j);
  if (j.i >= j.n || j.s[j.i] != '{') return false;
  j.i++;
  jSkipWs(j);
  if (j.i < j.n && j.s[j.i] == '}') {
    j.i++;
    return true;
  }
  char key[24];
  while (j.i < j.n) {
    if (!jSkipString(j, key, sizeof(key))) return false;
    jSkipWs(j);
    if (j.i >= j.n || j.s[j.i] != ':') return false;
    j.i++;
    jSkipWs(j);
    if (strcmp(key, "id") == 0) {
      if (!jParseU32(j, hit.id)) return false;
    } else if (strcmp(key, "title") == 0) {
      if (!jSkipString(j, hit.title, sizeof(hit.title))) return false;
      if (!hit.title[0]) strncpy(hit.title, "Model", sizeof(hit.title) - 1);
    } else if (strcmp(key, "commentCount") == 0) {
      if (!jParseU32(j, hit.comments)) return false;
    } else if (strcmp(key, "downloadCount") == 0) {
      if (!jParseU32(j, hit.downloads)) return false;
    } else if (strcmp(key, "likeCount") == 0) {
      if (!jParseU32(j, hit.likes)) return false;
    } else if (strcmp(key, "printCount") == 0) {
      if (!jParseU32(j, hit.prints)) return false;
    } else if (strcmp(key, "collectionCount") == 0) {
      if (!jParseU32(j, hit.collections)) return false;
    } else if (strcmp(key, "boostCnt") == 0) {
      if (!jParseU32(j, hit.boosts)) return false;
    } else {
      if (!jSkipValue(j)) return false;
    }
    jSkipWs(j);
    if (j.i >= j.n) return false;
    if (j.s[j.i] == ',') {
      j.i++;
      jSkipWs(j);
      continue;
    }
    if (j.s[j.i] == '}') {
      j.i++;
      return true;
    }
    return false;
  }
  return false;
}

void ingestPubHit(WalkAcc& acc, const PubHit& hit) {
  if (hit.id == 0) return;
  acc.found++;
  acc.commentSum += (long)hit.comments;
  rememberAndAlert((long)hit.id, hit.title, (long)hit.downloads, (long)hit.likes, (long)hit.prints,
                   (long)hit.boosts, (long)hit.collections, (long)hit.comments);
  for (int i = 0; i < acc.nids; i++) {
    if (acc.ids[i] == (long)hit.id) {
      fillSnapFromPub(hit, acc.featuredGot[i]);
      acc.featuredOk[i] = true;
      break;
    }
  }
}

bool walkPublishedPage(const String& body, int& totalOut, PubHit* hits, int maxHits, int& nHits) {
  JsonCur j;
  j.s = body.c_str();
  j.n = (int)body.length();
  j.i = 0;
  nHits = 0;
  jSkipWs(j);
  if (j.i >= j.n || j.s[j.i] != '{') return false;
  j.i++;
  jSkipWs(j);
  bool gotTotal = false;
  bool gotHits = false;
  totalOut = -1;
  if (j.i < j.n && j.s[j.i] == '}') return false;
  char key[24];
  while (j.i < j.n) {
    if (!jSkipString(j, key, sizeof(key))) return false;
    jSkipWs(j);
    if (j.i >= j.n || j.s[j.i] != ':') return false;
    j.i++;
    jSkipWs(j);
    if (strcmp(key, "total") == 0) {
      uint32_t t = 0;
      if (!jParseU32(j, t)) return false;
      totalOut = (int)t;
      gotTotal = true;
    } else if (strcmp(key, "hits") == 0) {
      if (j.i >= j.n || j.s[j.i] != '[') return false;
      j.i++;
      jSkipWs(j);
      gotHits = true;
      if (j.i < j.n && j.s[j.i] == ']') {
        j.i++;
      } else {
        while (j.i < j.n) {
          PubHit hit;
          if (!walkHit(j, hit)) return false;
          if (hit.id > 0) {
            if (nHits >= maxHits) return false;
            hits[nHits++] = hit;
          }
          jSkipWs(j);
          if (j.i >= j.n) return false;
          if (j.s[j.i] == ',') {
            j.i++;
            jSkipWs(j);
            continue;
          }
          if (j.s[j.i] == ']') {
            j.i++;
            break;
          }
          return false;
        }
      }
    } else {
      if (!jSkipValue(j)) return false;
    }
    jSkipWs(j);
    if (j.i >= j.n) return false;
    if (j.s[j.i] == ',') {
      j.i++;
      jSkipWs(j);
      continue;
    }
    if (j.s[j.i] == '}') {
      j.i++;
      jSkipWs(j);
      return gotTotal && gotHits && j.i == j.n;
    }
    return false;
  }
  return false;
}

bool jsonBodyComplete(const String& body) {
  const char* s = body.c_str();
  int n = (int)body.length();
  int i = 0;
  while (i < n && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) i++;
  if (i >= n || (s[i] != '{' && s[i] != '[')) return false;
  int obj = 0;
  int arr = 0;
  bool inStr = false;
  bool esc = false;
  for (; i < n; i++) {
    char c = s[i];
    if (inStr) {
      if (esc) {
        esc = false;
        continue;
      }
      if (c == '\\') {
        esc = true;
        continue;
      }
      if (c == '"') inStr = false;
      continue;
    }
    if (c == '"') {
      inStr = true;
      continue;
    }
    if (c == '{') {
      obj++;
    } else if (c == '}') {
      obj--;
      if (obj < 0) return false;
    } else if (c == '[') {
      arr++;
    } else if (c == ']') {
      arr--;
      if (arr < 0) return false;
    }
    if (obj == 0 && arr == 0 && (c == '}' || c == ']')) {
      i++;
      while (i < n && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) i++;
      return i == n && !inStr;
    }
  }
  return false;
}

bool jsonHasProfileShape(const String& body) {
  if (!jsonBodyComplete(body)) return false;
  const char* s = body.c_str();
  if (!strstr(s, "\"uid\"")) return false;
  return strstr(s, "\"downloadCount\"") || strstr(s, "\"MWCount\"");
}

bool commentsCensusComplete(int found, int reported) {
  if (found < 0 || reported < 0) return false;
  int expect = reported > MAX_ALL_MODELS ? MAX_ALL_MODELS : reported;
  if (expect == 0) return found == 0;
  return found == expect;
}

bool commentsNotACollapse(long sum) {
  if (!hasBaseline || lastSent.comments <= 0) return true;
  if (sum >= lastSent.comments) return true;
  long drop = lastSent.comments - sum;
  if (drop <= 10) return true;
  if (lastSent.comments >= 20 && sum * 2 < lastSent.comments) return false;
  if (drop >= 40) return false;
  return true;
}

bool fetchDesignCommentCount(HTTPClient& http, WiFiClientSecure& client, uint32_t id, uint32_t& out) {
  if (id == 0) return false;
  String url = String("https://api.bambulab.com/v1/design-service/design/") + String(id);
  const char* key = "\"commentCount\":";
  const int keyLen = 15;
  for (int attempt = 0; attempt < 2; attempt++) {
    http.end();
    client.stop();
    if (attempt) delay(200);
    serviceNet();
    if (!http.begin(client, url)) continue;
    http.addHeader("Accept", "application/json");
    http.addHeader("User-Agent", MW_UA);
    int code = http.GET();
    if (code != 200) {
      http.end();
      continue;
    }
    WiFiClient* stream = http.getStreamPtr();
    int matched = 0;
    bool inNum = false;
    bool digit = false;
    uint32_t val = 0;
    int seen = 0;
    unsigned long start = millis();
    bool ok = false;
    while (seen < 24576 && millis() - start < 12000) {
      if (!stream->available()) {
        delay(2);
        serviceNet();
        continue;
      }
      int c = stream->read();
      if (c < 0) break;
      seen++;
      if (!inNum) {
        if ((char)c == key[matched]) {
          matched++;
          if (matched == keyLen) inNum = true;
        } else {
          matched = ((char)c == key[0]) ? 1 : 0;
        }
        continue;
      }
      if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
        if (digit) {
          ok = true;
          break;
        }
        continue;
      }
      if (c >= '0' && c <= '9') {
        digit = true;
        val = val * 10u + (uint32_t)(c - '0');
        continue;
      }
      if (digit) ok = true;
      break;
    }
    http.end();
    client.stop();
    if (ok && digit) {
      out = val;
      return true;
    }
  }
  return false;
}

bool fetchPublishedPage(HTTPClient& http, WiFiClientSecure& client, int offset, String& body) {
  String url = String("https://api.bambulab.com/v1/design-service/publisheddesigns/") +
               String((unsigned long)cfgUid) + "?type=ALL&limit=" + String(PUBLISH_LIMIT) +
               "&offset=" + String(offset);
  for (int attempt = 0; attempt < 2; attempt++) {
    if (attempt) {
      http.end();
      client.stop();
      delay(250);
      serviceNet();
    }
    if (!http.begin(client, url)) continue;
    http.addHeader("Accept", "application/json");
    http.addHeader("User-Agent", MW_UA);
    int code = http.GET();
    if (code != 200) {
      http.end();
      continue;
    }
    body = http.getString();
    http.end();
    if (body.length() < 8) continue;
    if (!jsonBodyComplete(body)) continue;
    return true;
  }
  return false;
}

bool fetchAllComments(const long* ids, int nids) {
  commentsReady = false;
  censusFound = 0;
  censusTotal = 0;

  static WalkAcc acc;
  acc.commentSum = 0;
  acc.found = 0;
  acc.ids = ids;
  acc.nids = nids;
  for (int i = 0; i < MAX_MODELS; i++) acc.featuredOk[i] = false;

  allSnapCount = 0;
  alertCount = 0;

  static WiFiClientSecure client;
  static HTTPClient http;
  client.setInsecure();
  client.setTimeout(20000);
  http.setTimeout(20000);
  http.setReuse(false);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  client.stop();

  int offset = 0;
  int total = -1;
  for (int page = 0; page < MAX_PUBLISH_PAGES; page++) {
    serviceNet();
    delay(30);
    String body;
    static PubHit pageHits[PUBLISH_LIMIT + 6];
    int nHits = 0;
    int pageTotal = -1;
    bool walked = false;
    for (int attempt = 0; attempt < 2 && !walked; attempt++) {
      if (attempt) delay(200);
      if (!fetchPublishedPage(http, client, offset, body)) continue;
      nHits = 0;
      pageTotal = -1;
      walked = walkPublishedPage(body, pageTotal, pageHits, PUBLISH_LIMIT + 6, nHits);
      body = "";
    }
    if (!walked) {
      censusFound = acc.found;
      censusTotal = total > 0 ? total : pageTotal;
      allSnapCount = 0;
      alertCount = 0;
      client.stop();
      return false;
    }

    if (page == 0) {
      total = pageTotal;
      censusTotal = total;
      if (total < 0) {
        allSnapCount = 0;
        alertCount = 0;
        client.stop();
        return false;
      }
    }
    for (int i = 0; i < nHits; i++) {
      uint32_t cc = 0;
      if (!fetchDesignCommentCount(http, client, pageHits[i].id, cc)) {
        censusFound = acc.found;
        censusTotal = total > 0 ? total : pageTotal;
        allSnapCount = 0;
        alertCount = 0;
        client.stop();
        return false;
      }
      pageHits[i].comments = cc;
      ingestPubHit(acc, pageHits[i]);
    }
    censusFound = acc.found;
    offset += PUBLISH_LIMIT;
    if (page == 0 && total > 0 && nHits <= 0) {
      allSnapCount = 0;
      alertCount = 0;
      client.stop();
      return false;
    }
    if (total > 0 && acc.found >= total) break;
    if (nHits < PUBLISH_LIMIT) {
      if (total > 0 && acc.found < total) {
        allSnapCount = 0;
        alertCount = 0;
        client.stop();
        return false;
      }
      break;
    }
  }
  client.stop();
  censusFound = acc.found;
  censusTotal = total;

  if (!commentsCensusComplete(acc.found, total)) {
    allSnapCount = 0;
    alertCount = 0;
    return false;
  }
  if (hasBaseline && prevAllSnapCount >= 8 && acc.found + 3 < prevAllSnapCount) {
    allSnapCount = 0;
    alertCount = 0;
    return false;
  }
  if (!commentsNotACollapse(acc.commentSum)) {
    allSnapCount = 0;
    alertCount = 0;
    return false;
  }

  current.comments = acc.commentSum;
  commentsReady = true;
  modelCount = 0;
  for (int i = 0; i < nids && modelCount < MAX_MODELS; i++) {
    if (!acc.featuredOk[i]) continue;
    models[modelCount++] = acc.featuredGot[i];
  }
  return true;
}

bool fetchStats() {
  long ids[MAX_MODELS];
  int nids = 0;
  commentsReady = false;
  {
    String url = String("https://api.bambulab.com/v1/design-user-service/user/profile/") +
                 String((unsigned long)cfgUid);
    String body;
    if (!httpsGet(url, body) || !jsonHasProfileShape(body)) {
      strncpy(lastError, "MakerWorld offline", sizeof(lastError) - 1);
      lastOk = false;
      return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, body);
    body = "";
    if (err) {
      strncpy(lastError, "JSON onleesbaar", sizeof(lastError) - 1);
      lastOk = false;
      return false;
    }

    const char* name = doc["name"] | makerName;
    strncpy(makerName, name, sizeof(makerName) - 1);
    makerName[sizeof(makerName) - 1] = 0;

    // Profile downloadCount includes other activity. The screen shows
    // downloads of your published models only.
    long downloads = 0;
    if (!doc["MWCount"].isNull() && !doc["MWCount"]["myDesignDownloadCount"].isNull()) {
      downloads = doc["MWCount"]["myDesignDownloadCount"] | 0;
    } else {
      downloads = doc["downloadCount"] | 0;
    }

    current.downloads = downloads;
    current.likes = doc["likeCount"] | 0;
    current.prints = doc["MWCount"]["myDesignPrintCount"] | 0;
    current.boosts = doc["boostGained"] | 0;
    current.collections = doc["collectionCount"] | 0;
    current.followers = doc["fanCount"] | 0;
    nids = collectDesignIds(doc, ids, MAX_MODELS);
  }

  if (!fetchAllComments(ids, nids)) {
    if (hasBaseline) {
      current.comments = lastSent.comments;
      if (prevAllSnapCount > 0) {
        memcpy(allSnaps, prevAllSnaps, sizeof(AllSnap) * prevAllSnapCount);
        allSnapCount = prevAllSnapCount;
      }
      alertCount = 0;
      lastOk = true;
      strncpy(lastError, "Comments onvolledig", sizeof(lastError) - 1);
      lastError[sizeof(lastError) - 1] = 0;
      return true;
    }
    current.comments = 0;
    allSnapCount = 0;
    alertCount = 0;
    strncpy(lastError, "Comments ophalen mislukt", sizeof(lastError) - 1);
    lastError[sizeof(lastError) - 1] = 0;
    lastOk = false;
    return false;
  }

  lastOk = true;
  lastError[0] = 0;
  return true;
}

bool telegramEnabled() {
  return cfgToken[0] && cfgChat[0];
}

bool sendTelegram(const String& text) {
  if (!telegramEnabled()) return true;
  String url = String("https://api.telegram.org/bot") + cfgToken + "/sendMessage";
  JsonDocument msg;
  msg["chat_id"] = cfgChat;
  msg["text"] = text;
  msg["disable_web_page_preview"] = true;
  String payload;
  serializeJson(msg, payload);
  String response;
  bool ok = httpsPostJson(url, payload, response);
  if (!ok) strncpy(lastError, "Telegram mislukt", sizeof(lastError) - 1);
  return ok;
}

void appendTotal(String& s, const char* label, long value, long prev) {
  long d = value - prev;
  s += label;
  s += ": ";
  s += fmtNum(value);
  if (d) {
    s += "  (";
    s += fmtDelta(d);
    s += ")";
  }
  s += "\n";
}

void appendAlertLine(String& s, const char* label, long d) {
  if (!d) return;
  s += "  ";
  s += label;
  s += " ";
  s += fmtDelta(d);
  s += "\n";
}

String periodLabel(unsigned long sec) {
  if (sec >= 5400) {
    unsigned long h = (sec + 1800UL) / 3600UL;
    if (h < 1) h = 1;
    return String(h) + " uur";
  }
  unsigned long m = (sec + 30UL) / 60UL;
  if (m < 1) m = 1;
  return String(m) + " min";
}

unsigned long telegramEverySec() {
  unsigned long every = (unsigned long)cfgTgEvery;
  unsigned long poll = (unsigned long)cfgPoll;
  if (every < poll) every = poll;
  return every;
}

bool clockReady() {
  return time(nullptr) > 1700000000L;
}

void persistDigest() {
  if (!clockReady() || lastDigestUnix <= 0) return;
  prefs.begin("pulse", false);
  prefs.putLong("tgat", (long)lastDigestUnix);
  prefs.end();
}

void stampDigest() {
  lastDigestMs = millis();
  if (clockReady()) {
    lastDigestUnix = time(nullptr);
    persistDigest();
  }
}

bool digestDue() {
  unsigned long every = telegramEverySec();
  if (clockReady()) {
    if (lastDigestUnix <= 0) {
      stampDigest();
      return false;
    }
    return (unsigned long)(time(nullptr) - lastDigestUnix) >= every;
  }
  if (lastDigestMs == 0) return false;
  return (millis() - lastDigestMs) >= every * 1000UL;
}

unsigned long digestElapsedSec() {
  if (clockReady() && lastDigestUnix > 0) {
    time_t now = time(nullptr);
    if (now > lastDigestUnix) return (unsigned long)(now - lastDigestUnix);
  }
  if (lastDigestMs != 0 && millis() >= lastDigestMs) return (millis() - lastDigestMs) / 1000UL;
  return telegramEverySec();
}

String buildAlert() {
  String s;
  s.reserve(3600);
  s += "MakerPulse · ";
  s += makerName;
  s += "\n";
  s += "Samenvatting · ";
  s += periodLabel(digestElapsedSec());
  s += "\n\n";
  if (cfgNotify & 1) appendTotal(s, "Downloads", current.downloads, lastSent.downloads);
  if (cfgNotify & 2) appendTotal(s, "Likes", current.likes, lastSent.likes);
  if (cfgNotify & 4) appendTotal(s, "Prints", current.prints, lastSent.prints);
  if (cfgNotify & 8) appendTotal(s, "Boosts", current.boosts, lastSent.boosts);
  if (cfgNotify & 16) appendTotal(s, "Collecties", current.collections, lastSent.collections);
  if (cfgNotify & 32) appendTotal(s, "Comments", current.comments, lastSent.comments);
  if (cfgNotify & 64) appendTotal(s, "Volgers", current.followers, lastSent.followers);

  if (alertCount > 0) {
    s += "\n";
    for (int i = 0; i < alertCount; i++) {
      if (s.length() > 3200) {
        s += "…en meer modellen\n";
        break;
      }
      s += alerts[i].title;
      s += "\n";
      appendAlertLine(s, "Downloads", alerts[i].dDownloads);
      appendAlertLine(s, "Likes", alerts[i].dLikes);
      appendAlertLine(s, "Prints", alerts[i].dPrints);
      appendAlertLine(s, "Boosts", alerts[i].dBoosts);
      appendAlertLine(s, "Collecties", alerts[i].dCollections);
      appendAlertLine(s, "Comments", alerts[i].dComments);
    }
  }

  if (s.length() > 3500) {
    s.remove(3490);
    s += "\n…";
  }
  return s;
}

bool notableChange() {
  if (!hasBaseline) return false;
  if (alertCount > 0) return true;
  if ((cfgNotify & 1) && current.downloads != lastSent.downloads) return true;
  if ((cfgNotify & 2) && current.likes != lastSent.likes) return true;
  if ((cfgNotify & 4) && current.prints != lastSent.prints) return true;
  if ((cfgNotify & 8) && current.boosts != lastSent.boosts) return true;
  if ((cfgNotify & 16) && current.collections != lastSent.collections) return true;
  if ((cfgNotify & 32) && current.comments != lastSent.comments) return true;
  if ((cfgNotify & 64) && current.followers != lastSent.followers) return true;
  return false;
}

void loadBaseline() {
  prefs.begin("pulse", true);
  hasBaseline = prefs.getBool("ok", false);
  lastSent.downloads = prefs.getLong("dl", 0);
  lastSent.likes = prefs.getLong("lk", 0);
  lastSent.prints = prefs.getLong("pr", 0);
  lastSent.boosts = prefs.getLong("bst", 0);
  lastSent.collections = prefs.getLong("col", 0);
  lastSent.comments = prefs.getLong("cmt", 0);
  lastSent.followers = prefs.getLong("fan", 0);
  lastDigestUnix = (time_t)prefs.getLong("tgat", 0);
  prevAllSnapCount = prefs.getUShort("acnt", 0);
  if (prevAllSnapCount > MAX_ALL_MODELS) prevAllSnapCount = MAX_ALL_MODELS;
  {
    int loaded = 0;
    const int chunk = 100;
    while (loaded < prevAllSnapCount) {
      int n = prevAllSnapCount - loaded;
      if (n > chunk) n = chunk;
      char key[8];
      snprintf(key, sizeof(key), "as%d", loaded / chunk);
      size_t want = sizeof(AllSnap) * n;
      size_t got = prefs.getBytes(key, &prevAllSnaps[loaded], want);
      if (got != want) {
        prevAllSnapCount = loaded;
        break;
      }
      loaded += n;
    }
  }
  blOn = prefs.getBool("blon", true);
  blBright = prefs.getUChar("blbri", 255);
  prefs.end();
}

void saveBaseline() {
  lastSent = current;
  hasBaseline = true;
  if (allSnapCount > 0) {
    memcpy(prevAllSnaps, allSnaps, sizeof(allSnaps));
    prevAllSnapCount = allSnapCount;
  }
  prefs.begin("pulse", false);
  prefs.putBool("ok", true);
  prefs.putLong("dl", lastSent.downloads);
  prefs.putLong("lk", lastSent.likes);
  prefs.putLong("pr", lastSent.prints);
  prefs.putLong("bst", lastSent.boosts);
  prefs.putLong("col", lastSent.collections);
  prefs.putLong("cmt", lastSent.comments);
  prefs.putLong("fan", lastSent.followers);
  if (allSnapCount > 0) {
    prefs.putUShort("acnt", (uint16_t)allSnapCount);
    const int chunk = 100;
    int stored = 0;
    int idx = 0;
    while (stored < allSnapCount) {
      int n = allSnapCount - stored;
      if (n > chunk) n = chunk;
      char key[8];
      snprintf(key, sizeof(key), "as%d", idx++);
      prefs.putBytes(key, &allSnaps[stored], sizeof(AllSnap) * n);
      stored += n;
    }
    for (; idx < 4; idx++) {
      char key[8];
      snprintf(key, sizeof(key), "as%d", idx);
      prefs.remove(key);
    }
  }
  prefs.remove("asnap");
  prefs.remove("msnap");
  prefs.remove("csnap");
  prefs.remove("mcnt");
  prefs.remove("ccnt");
  prefs.end();
}

void saveBacklight() {
  prefs.begin("pulse", false);
  prefs.putBool("blon", blOn);
  prefs.putUChar("blbri", blBright);
  prefs.end();
}

String lightJson() {
  JsonDocument doc;
  doc["state"] = blOn ? "ON" : "OFF";
  doc["brightness"] = blBright;
  String out;
  serializeJson(doc, out);
  return out;
}

void publishLightState() {
  if (!mqtt.connected()) return;
  mqtt.publish(topicState, lightJson().c_str(), true);
}

void publishStats() {
  if (!mqtt.connected() || !lastOk) return;
  JsonDocument doc;
  doc["downloads"] = current.downloads;
  doc["likes"] = current.likes;
  doc["prints"] = current.prints;
  doc["boosts"] = current.boosts;
  doc["collections"] = current.collections;
  doc["comments"] = current.comments;
  doc["followers"] = current.followers;
  String payload;
  serializeJson(doc, payload);
  mqtt.publish(topicStats, payload.c_str(), true);
}

void applyBacklight(bool pub) {
  tft.setBrightness(blOn ? blBright : 0);
  saveBacklight();
  if (pub && !autoBacklight()) publishLightState();
}

void noteActivity() {
  lastActivityMs = millis();
  if (!autoBacklight()) return;
  if (!blOn || blBright == 0) {
    blOn = true;
    if (blBright == 0) blBright = 255;
  }
  tft.setBrightness(blBright);
}

void serviceBacklight() {
  if (!autoBacklight() || !blOn) return;
  if (millis() - lastActivityMs < (unsigned long)BACKLIGHT_IDLE_SEC * 1000UL) return;
  blOn = false;
  tft.setBrightness(0);
}

void serviceTouch() {
  if (!autoBacklight()) return;
  uint16_t x = 0;
  uint16_t y = 0;
  if (!tft.getTouchRaw(&x, &y)) return;
  if (x < 50 || y < 50 || x > 4000 || y > 4000) return;
  noteActivity();
}

bool statsMoved() {
  if (!hasBaseline) return false;
  return current.downloads != lastSent.downloads || current.likes != lastSent.likes ||
         current.prints != lastSent.prints || current.boosts != lastSent.boosts ||
         current.collections != lastSent.collections || current.comments != lastSent.comments ||
         current.followers != lastSent.followers;
}

void parseLightJson(const String& body) {
  if (autoBacklight()) return;
  if (!body.length()) return;
  JsonDocument doc;
  if (deserializeJson(doc, body)) return;
  if (!doc["brightness"].isNull()) {
    int b = doc["brightness"] | 0;
    if (b < 0) b = 0;
    if (b > 255) b = 255;
    blBright = (uint8_t)b;
    if (blBright > 0) blOn = true;
  }
  if (!doc["state"].isNull()) {
    const char* st = doc["state"];
    if (st) {
      if (strcmp(st, "ON") == 0 || strcmp(st, "on") == 0) {
        blOn = true;
        if (blBright == 0) blBright = 255;
      } else if (strcmp(st, "OFF") == 0 || strcmp(st, "off") == 0) {
        blOn = false;
      }
    }
  }
  applyBacklight(true);
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  String body;
  body.reserve(len + 1);
  for (unsigned int i = 0; i < len; i++) body += (char)payload[i];
  parseLightJson(body);
}

void publishDiscovery() {
  if (!mqtt.connected()) return;
  JsonDocument d;
  d["name"] = "MakerPulse scherm";
  d["unique_id"] = mqttClientId;
  d["schema"] = "json";
  d["command_topic"] = topicCmd;
  d["state_topic"] = topicState;
  d["brightness"] = true;
  JsonArray modes = d["supported_color_modes"].to<JsonArray>();
  modes.add("brightness");
  d["availability_topic"] = topicAvail;
  d["payload_available"] = "online";
  d["payload_not_available"] = "offline";
  JsonObject dev = d["device"].to<JsonObject>();
  dev["identifiers"][0] = mqttClientId;
  dev["name"] = "MakerPulse CYD";
  dev["model"] = "ESP32-2432S028";
  dev["manufacturer"] = "MakerPulse";
  String payload;
  serializeJson(d, payload);
  if (autoBacklight()) {
    mqtt.publish(topicDisc, "", true);
  } else {
    mqtt.publish(topicDisc, payload.c_str(), true);
  }

  static const char* keys[] = {"downloads", "likes", "prints", "boosts", "collections", "comments", "followers"};
  static const char* names[] = {"Downloads", "Likes", "Prints", "Boosts", "Collecties", "Comments", "Volgers"};
  static const char* icons[] = {"mdi:download", "mdi:heart",        "mdi:printer-3d", "mdi:rocket-launch",
                                "mdi:bookmark", "mdi:comment",      "mdi:account-group"};
  for (int i = 0; i < 7; i++) {
    JsonDocument s;
    s["name"] = names[i];
    char uid[48];
    snprintf(uid, sizeof(uid), "%s_%s", mqttClientId, keys[i]);
    s["unique_id"] = uid;
    s["state_topic"] = topicStats;
    char tmpl[40];
    snprintf(tmpl, sizeof(tmpl), "{{ value_json.%s }}", keys[i]);
    s["value_template"] = tmpl;
    s["state_class"] = "measurement";
    s["icon"] = icons[i];
    s["availability_topic"] = topicAvail;
    s["payload_available"] = "online";
    s["payload_not_available"] = "offline";
    JsonObject dev = s["device"].to<JsonObject>();
    dev["identifiers"][0] = mqttClientId;
    dev["name"] = "MakerPulse CYD";
    dev["model"] = "ESP32-2432S028";
    dev["manufacturer"] = "MakerPulse";
    String body;
    serializeJson(s, body);
    char topic[80];
    snprintf(topic, sizeof(topic), "homeassistant/sensor/%s/%s/config", mqttClientId, keys[i]);
    mqtt.publish(topic, body.c_str(), true);
    mqtt.loop();
  }
}

void ensureMqtt() {
  if (!mqttEnabled()) return;
  if (mqtt.connected()) return;
  static unsigned long lastTry = 0;
  if (millis() - lastTry < 4000) return;
  lastTry = millis();
  bool ok;
  if (cfgMqttUser[0]) {
    ok = mqtt.connect(mqttClientId, cfgMqttUser, cfgMqttPass, topicAvail, 1, true, "offline");
  } else {
    ok = mqtt.connect(mqttClientId, topicAvail, 1, true, "offline");
  }
  if (!ok) return;
  mqtt.publish(topicAvail, "online", true);
  if (!autoBacklight()) mqtt.subscribe(topicCmd);
  publishDiscovery();
  if (!autoBacklight()) publishLightState();
  if (lastOk) publishStats();
}

void setupMqtt() {
  if (!mqttEnabled()) return;
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  snprintf(mqttClientId, sizeof(mqttClientId), "makerpulse_%s", mac.c_str());
  snprintf(topicCmd, sizeof(topicCmd), "makerpulse/%s/light/set", mac.c_str());
  snprintf(topicState, sizeof(topicState), "makerpulse/%s/light/state", mac.c_str());
  snprintf(topicAvail, sizeof(topicAvail), "makerpulse/%s/status", mac.c_str());
  snprintf(topicStats, sizeof(topicStats), "makerpulse/%s/stats", mac.c_str());
  snprintf(topicDisc, sizeof(topicDisc), "homeassistant/light/%s/config", mqttClientId);
  mqtt.setServer(cfgMqttHost, cfgMqttPort);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(1536);
}

void sendCors() {
  www.sendHeader("Access-Control-Allow-Origin", "*");
  www.sendHeader("Access-Control-Allow-Methods", "GET,POST,PUT,OPTIONS");
  www.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleLight() {
  if (www.method() == HTTP_OPTIONS) {
    sendCors();
    www.send(204);
    return;
  }
  if (www.method() == HTTP_POST || www.method() == HTTP_PUT) {
    parseLightJson(www.arg("plain"));
  }
  if (www.hasArg("state") || www.hasArg("brightness")) {
    JsonDocument doc;
    if (www.hasArg("state")) doc["state"] = www.arg("state");
    if (www.hasArg("brightness")) doc["brightness"] = www.arg("brightness").toInt();
    String body;
    serializeJson(doc, body);
    parseLightJson(body);
  }
  sendCors();
  www.send(200, "application/json", lightJson());
}

void copyField(char* dst, size_t n, const String& src) {
  if (!n) return;
  strncpy(dst, src.c_str(), n - 1);
  dst[n - 1] = 0;
}

void loadUserConfig() {
  prefs.begin("cfg", true);
  if (prefs.getBool("ready", false)) {
    copyField(cfgSsid, sizeof(cfgSsid), prefs.getString("ssid", cfgSsid));
    copyField(cfgPass, sizeof(cfgPass), prefs.getString("pass", cfgPass));
    copyField(makerName, sizeof(makerName), prefs.getString("name", makerName));
    copyField(cfgToken, sizeof(cfgToken), prefs.getString("token", cfgToken));
    copyField(cfgChat, sizeof(cfgChat), prefs.getString("chat", cfgChat));
    copyField(cfgMqttHost, sizeof(cfgMqttHost), prefs.getString("mqh", cfgMqttHost));
    copyField(cfgMqttUser, sizeof(cfgMqttUser), prefs.getString("mqu", cfgMqttUser));
    copyField(cfgMqttPass, sizeof(cfgMqttPass), prefs.getString("mqp", cfgMqttPass));
    cfgUid = prefs.getULong("uid", cfgUid);
    cfgPoll = prefs.getInt("poll", cfgPoll);
    cfgTgEvery = prefs.getInt("tge", cfgTgEvery);
    cfgBlMode = prefs.getInt("bl", cfgBlMode);
    cfgMqttPort = (uint16_t)prefs.getUInt("mqport", cfgMqttPort);
    cfgNotify = prefs.getUChar("nfy", cfgNotify);
    String nm = prefs.getString("name", "");
    if (nm.length()) copyField(makerName, sizeof(makerName), nm);
    copyField(cfgQuery, sizeof(cfgQuery), prefs.getString("qry", ""));
    cfgAutoName = prefs.getBool("auton", false);
  }
  if (prefs.isKey("rot")) cfgRot = prefs.getInt("rot", cfgRot) & 3;
  else cfgRot = (CYD_ROTATION + 1) & 3;
  if (prefs.isKey("mir")) cfgMirror = prefs.getBool("mir", false);
  else cfgMirror = true;
  prefs.end();
}

void applyOrientation() {
  uint8_t r = (uint8_t)(cfgRot & 3);
  if (cfgMirror) r |= 4;
  tft.setRotation(r);
}

bool configUsable() {
  if (!cfgUid || !cfgSsid[0]) return false;
  if (strcmp(cfgSsid, "JOUW_WIFI_NAAM") == 0) return false;
  if (strcmp(cfgPass, "JOUW_WIFI_WACHTWOORD") == 0) return false;
  return true;
}

String htmlEsc(const String& in) {
  String o;
  o.reserve(in.length() + 4);
  for (unsigned i = 0; i < in.length(); i++) {
    char c = in[i];
    if (c == '&') {
      o += '&';
      o += 'a';
      o += 'm';
      o += 'p';
      o += ';';
    } else if (c == '<') {
      o += '&';
      o += 'l';
      o += 't';
      o += ';';
    } else if (c == '"') {
      o += '&';
      o += 'q';
      o += 'u';
      o += 'o';
      o += 't';
      o += ';';
    } else o += c;
  }
  return o;
}

void sendSetupPage() {
  String html;
  html.reserve(7000);
  html += "<!DOCTYPE html><html><head><meta charset=utf-8>"
          "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
          "<title>MakerPulse</title><style>"
          "body{margin:0;background:#101110;color:#f3f3f3;font-family:system-ui,Segoe UI,sans-serif}"
          ".wrap{max-width:440px;margin:0 auto;padding:22px 16px 48px}"
          ".top{display:flex;align-items:center;gap:12px;margin-bottom:18px}"
          ".mark{width:10px;height:28px;border-radius:8px;background:#00ae42}"
          "h1{margin:0;font-size:1.25rem;font-weight:650;letter-spacing:-.02em}"
          ".sub{margin:2px 0 0;color:#8d8d8d;font-size:12px}"
          "label{display:block;margin:14px 0 6px;font-size:13px;color:#cfcfcf}"
          "input,select{width:100%;box-sizing:border-box;padding:12px;border-radius:10px;border:1px solid #2a2a2a;background:#181818;color:#fff}"
          "input:focus,select:focus{outline:2px solid #00ae42;border-color:#00ae42}"
          "button{margin-top:16px;width:100%;padding:13px;border:0;border-radius:10px;background:#00ae42;color:#fff;font-weight:650}"
          ".ghost{background:transparent;color:#e8e8e8;border:1px solid #3a3a3a}"
          ".chk{display:flex;align-items:center;gap:8px;margin:8px 0;color:#eee}"
          "small{color:#8d8d8d}</style></head><body><div class=wrap>"
          "<div class=top><div class=mark></div><div><h1>MakerPulse</h1>"
          "<p class=sub>MakerWorld op de CYD</p></div></div>"
          "<p><small>WiFi, een modellink of UID, Telegram en MQTT. Opslaan herstart het scherm.</small></p>"
          "<form id=f method=POST action=/save onsubmit=\"document.getElementById('go').textContent='Opslaan…';\">";
  html += "<label>WiFi-naam</label><input name=ssid maxlength=32 value=\"";
  html += htmlEsc(cfgSsid);
  html += "\"><label>WiFi-wachtwoord</label><input name=pass type=password maxlength=64 value=\"";
  html += htmlEsc(cfgPass);
  html += "\"><p><small id=nets>Netwerken zoeken…</small></p><select id=pick></select>";
  html += "<label>MakerWorld — link van een eigen model, of UID</label><input name=uid maxlength=180 placeholder=\"https://makerworld.com/…/models/…\" value=\"";
  html += cfgUid ? String(cfgUid) : "";
  html += "\"><label>Naam op het scherm</label><input name=name maxlength=22 placeholder=\"leeg = MakerWorld-naam\" value=\"";
  if (makerName[0] && strcmp(makerName, "Maker") != 0) html += htmlEsc(makerName);
  html += "\"><label>Telegram-token</label><input name=token maxlength=78 value=\"";
  html += htmlEsc(cfgToken);
  html += "\"><label>Telegram chat-ID</label><input name=chat maxlength=22 value=\"";
  html += htmlEsc(cfgChat);
  html += "\"><label>Telegram-samenvatting</label><select name=tgevery>";
  html += String("<option value=0") + (cfgTgEvery == 0 ? " selected" : "") + ">Elke check</option>";
  html += String("<option value=3600") + (cfgTgEvery == 3600 ? " selected" : "") + ">1 uur</option>";
  html += String("<option value=21600") + (cfgTgEvery == 21600 ? " selected" : "") + ">6 uur</option>";
  html += String("<option value=86400") + (cfgTgEvery == 86400 ? " selected" : "") + ">24 uur</option></select>";
  html += "<label>Check-interval</label><select name=poll>";
  html += String("<option value=120") + (cfgPoll == 120 ? " selected" : "") + ">2 min</option>";
  html += String("<option value=300") + (cfgPoll == 300 ? " selected" : "") + ">5 min</option>";
  html += String("<option value=600") + (cfgPoll == 600 ? " selected" : "") + ">10 min</option>";
  html += String("<option value=900") + (cfgPoll == 900 ? " selected" : "") + ">15 min</option></select>";
  html += "<label>Scherm</label><select name=bl>";
  html += String("<option value=0") + (cfgBlMode != 1 ? " selected" : "") + ">MQTT / Home Assistant</option>";
  html += String("<option value=1") + (cfgBlMode == 1 ? " selected" : "") + ">Automatisch, uit na 10 min</option></select>";
  html += "<label>Scherm draaien</label><select name=rot>";
  html += String("<option value=0") + (cfgRot == 0 ? " selected" : "") + ">0 graden</option>";
  html += String("<option value=1") + (cfgRot == 1 ? " selected" : "") + ">90 graden</option>";
  html += String("<option value=2") + (cfgRot == 2 ? " selected" : "") + ">180 graden</option>";
  html += String("<option value=3") + (cfgRot == 3 ? " selected" : "") + ">270 graden</option></select>";
  html += "<label class=chk><input type=checkbox name=mir";
  if (cfgMirror) html += " checked";
  html += "> Spiegelen</label>";
  html += "<label>MQTT-broker</label><input name=mqhost maxlength=62 placeholder=\"leeg = uit\" value=\"";
  html += htmlEsc(cfgMqttHost);
  html += "\"><label>MQTT-poort</label><input name=mqport inputmode=numeric value=\"";
  html += String(cfgMqttPort ? cfgMqttPort : 1883);
  html += "\"><label>MQTT-gebruiker</label><input name=mquser maxlength=30 value=\"";
  html += htmlEsc(cfgMqttUser);
  html += "\"><label>MQTT-wachtwoord</label><input name=mqpass type=password maxlength=62 value=\"";
  html += htmlEsc(cfgMqttPass);
  html += "\"><p>Telegram bij</p>";
  const char* bits[] = {"n1", "n2", "n4", "n8", "n16", "n32", "n64"};
  const char* labels[] = {"Downloads", "Likes", "Prints", "Boosts", "Collecties", "Comments", "Volgers"};
  const uint8_t masks[] = {1, 2, 4, 8, 16, 32, 64};
  for (int i = 0; i < 7; i++) {
    html += "<label class=chk><input type=checkbox name=";
    html += bits[i];
    if (cfgNotify & masks[i]) html += " checked";
    html += "> ";
    html += labels[i];
    html += "</label>";
  }
  html += "<button type=submit id=go>Opslaan en herstarten</button>"
          "<button class=ghost type=submit formaction=/reset formmethod=post>Wissen en opnieuw</button>"
          "</form></div><script>fetch('/scan').then(r=>r.json()).then(list=>{"
          "const p=document.getElementById('pick');const n=document.getElementById('nets');"
          "const ssid=document.querySelector('[name=ssid]');"
          "if(!list.length){n.textContent='Geen netwerken gevonden. Typ de naam hierboven.';return;}"
          "n.textContent='Of kies een gevonden netwerk:';"
          "list.forEach(s=>{const o=document.createElement('option');o.value=s;o.textContent=s;"
          "if(s===ssid.value)o.selected=true;p.appendChild(o);});"
          "p.onchange=()=>{if(p.value)ssid.value=p.value;};"
          "}).catch(()=>{document.getElementById('nets').textContent='';});</script></body></html>";
  www.send(200, "text/html; charset=utf-8", html);
}

void handleScan() {
  int n = WiFi.scanNetworks(false, false, false, 40);
  String out = "[";
  int added = 0;
  for (int i = 0; i < n && added < 15; i++) {
    String s = WiFi.SSID(i);
    if (!s.length()) continue;
    bool dup = false;
    for (int j = 0; j < i; j++) {
      if (WiFi.SSID(j) == s) {
        dup = true;
        break;
      }
    }
    if (dup) continue;
    if (added) out += ',';
    out += '"';
    s.replace("\\", "");
    s.replace("\"", "'");
    out += s;
    out += '"';
    added++;
  }
  out += ']';
  WiFi.scanDelete();
  www.send(200, "application/json", out);
}

int allowedPoll(int v) {
  if (v == 120 || v == 600 || v == 900) return v;
  return 300;
}

int allowedEvery(int v) {
  if (v == 3600 || v == 21600 || v == 86400) return v;
  return 0;
}

uint32_t digitsAfter(const String& s, const char* key) {
  int i = s.indexOf(key);
  if (i < 0) return 0;
  i += strlen(key);
  while (i < (int)s.length() && !isdigit(s[i])) {
    if (s[i] != '/') break;
    i++;
  }
  if (i >= (int)s.length() || !isdigit(s[i])) return 0;
  return strtoul(s.c_str() + i, nullptr, 10);
}

bool parseMakerField(const String& raw, uint32_t& uid, uint32_t& modelId, String& err) {
  uid = 0;
  modelId = 0;
  String text = raw;
  text.trim();
  if (!text.length()) {
    err = "Plak een modellink of je numerieke UID.";
    return false;
  }
  String low = text;
  low.toLowerCase();
  bool digits = text.length() >= 4;
  for (unsigned i = 0; digits && i < text.length(); i++) {
    if (!isdigit(text[i])) digits = false;
  }
  if (digits) {
    uid = strtoul(text.c_str(), nullptr, 10);
    if (uid < 1000) {
      err = "Die UID is te kort.";
      return false;
    }
    return true;
  }
  uint32_t direct = digitsAfter(low, "/u/");
  if (!direct) direct = digitsAfter(low, "/avatar/");
  if (direct >= 1000) {
    uid = direct;
    return true;
  }
  uint32_t model = digitsAfter(low, "/models/");
  if (!model) model = digitsAfter(low, "/model/");
  if (!model) model = digitsAfter(low, "/designs/");
  if (!model) model = digitsAfter(low, "/design/");
  if (model < 1000) {
    err = "Plak de link van een eigen model (…/models/…) of je numerieke UID.";
    return false;
  }
  modelId = model;
  return true;
}

bool pullJsonString(const char* win, const char* key, char* out, size_t n) {
  if (!n) return false;
  out[0] = 0;
  char* h = strstr(win, key);
  if (!h) return false;
  char* q = strchr(h, ':');
  if (!q) return false;
  q = strchr(q, '"');
  if (!q) return false;
  q++;
  size_t k = 0;
  while (*q && *q != '"' && *q != '\\' && k + 1 < n) out[k++] = *q++;
  out[k] = 0;
  return k > 0;
}

bool readCreator(const String& url, const char* anchor, uint32_t& uid, char* handle, size_t handleN) {
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.useHTTP10(true);
  http.setTimeout(12000);
  if (!http.begin(client, url)) return false;
  http.addHeader("Accept", "application/json");
  http.addHeader("User-Agent", MW_UA);
  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }
  WiFiClient* stream = http.getStreamPtr();
  char win[180];
  int n = 0;
  win[0] = 0;
  bool seen = anchor == nullptr;
  int after = 0;
  bool gotUid = false;
  bool gotHandle = handle == nullptr;
  unsigned long start = millis();
  while (millis() - start < 12000) {
    while (stream->available()) {
      char c = (char)stream->read();
      if (n < (int)sizeof(win) - 1) win[n++] = c;
      else {
        memmove(win, win + 1, sizeof(win) - 2);
        win[sizeof(win) - 2] = c;
        n = sizeof(win) - 1;
      }
      win[n] = 0;
      if (!seen && anchor && strstr(win, anchor)) {
        seen = true;
        n = 0;
        win[0] = 0;
        continue;
      }
      if (!seen) continue;
      after++;
      if (!gotUid) {
        char* u = strstr(win, "\"uid\"");
        if (u) {
          char* colon = strchr(u, ':');
          if (colon) {
            uint32_t v = strtoul(colon + 1, nullptr, 10);
            if (v >= 1000) {
              uid = v;
              gotUid = true;
            }
          }
        }
      }
      if (!gotHandle && handle) {
        if (pullJsonString(win, "\"handle\"", handle, handleN)) gotHandle = true;
      }
      if (gotUid && gotHandle) {
        http.end();
        return true;
      }
      if (after > 1200) break;
    }
    if (!http.connected()) break;
    delay(1);
  }
  http.end();
  return gotUid;
}

bool lookupCreator(uint32_t modelId, uint32_t& uid, char* handle, size_t handleN) {
  uid = 0;
  if (handle && handleN) handle[0] = 0;
  String url = String("https://api.bambulab.com/v1/design-service/design/") + String(modelId);
  return readCreator(url, "designCreator", uid, handle, handleN);
}

bool lookupProfileHandle(uint32_t uid, char* handle, size_t handleN) {
  uint32_t ignored = 0;
  String url = String("https://api.bambulab.com/v1/design-user-service/user/profile/") + String(uid);
  return readCreator(url, nullptr, ignored, handle, handleN) && handle[0];
}

bool nameNeedsFetch() {
  return cfgAutoName || !makerName[0] || strcmp(makerName, "Maker") == 0;
}

void saveIdentity() {
  prefs.begin("cfg", false);
  prefs.putULong("uid", cfgUid);
  prefs.putString("name", makerName);
  prefs.putString("qry", "");
  prefs.putBool("auton", false);
  prefs.end();
  cfgQuery[0] = 0;
  cfgAutoName = false;
}

void resolveIdentity() {
  if (WiFi.status() != WL_CONNECTED) return;
  bool needUid = cfgUid < 1000 && cfgQuery[0];
  bool needName = nameNeedsFetch();
  if (!needUid && !needName) return;
  drawStatus("MakerWorld", needUid ? "model lezen" : "naam ophalen");
  if (needUid) {
    uint32_t uid = 0;
    uint32_t model = 0;
    String err;
    if (!parseMakerField(cfgQuery, uid, model, err)) return;
    if (uid >= 1000) cfgUid = uid;
    if (model) {
      char handle[24] = "";
      uint32_t found = 0;
      if (!lookupCreator(model, found, handle, sizeof(handle))) return;
      cfgUid = found;
      if (needName && handle[0]) copyField(makerName, sizeof(makerName), handle);
      needName = nameNeedsFetch();
    }
  }
  if (cfgUid >= 1000 && needName) {
    char handle[24] = "";
    if (lookupProfileHandle(cfgUid, handle, sizeof(handle)) && handle[0]) {
      copyField(makerName, sizeof(makerName), handle);
    }
  }
  if (cfgUid >= 1000 && makerName[0] && strcmp(makerName, "Maker") != 0) saveIdentity();
  else if (cfgUid >= 1000) {
    prefs.begin("cfg", false);
    prefs.putULong("uid", cfgUid);
    prefs.putString("qry", "");
    prefs.end();
    cfgQuery[0] = 0;
  }
}

void handleSave() {
  String ssid = www.arg("ssid");
  ssid.trim();
  if (ssid.length() < 1 || ssid.length() > 32) {
    www.send(400, "text/plain; charset=utf-8", "Vul een WiFi-naam in.");
    return;
  }
  String uidTxt = www.arg("uid");
  uidTxt.trim();
  uint32_t uid = 0;
  uint32_t model = 0;
  String err;
  if (!parseMakerField(uidTxt, uid, model, err)) {
    www.send(400, "text/html; charset=utf-8",
             String("<!DOCTYPE html><meta charset=utf-8><body style=\"background:#101110;color:#f3f3f3;font-family:system-ui;padding:24px\">") +
                 "<p>" + err + "</p><p><a style=\"color:#00ae42\" href=/setup>Terug</a></p></body>");
    return;
  }
  String shown = www.arg("name");
  shown.trim();
  bool autoName = !shown.length() || shown == "Maker";
  uint8_t nfy = 0;
  if (www.hasArg("n1")) nfy |= 1;
  if (www.hasArg("n2")) nfy |= 2;
  if (www.hasArg("n4")) nfy |= 4;
  if (www.hasArg("n8")) nfy |= 8;
  if (www.hasArg("n16")) nfy |= 16;
  if (www.hasArg("n32")) nfy |= 32;
  if (www.hasArg("n64")) nfy |= 64;
  int port = www.arg("mqport").toInt();
  if (port < 1 || port > 65535) port = 1883;
  prefs.begin("cfg", false);
  prefs.putBool("ready", true);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", www.arg("pass"));
  prefs.putString("name", autoName ? "" : shown);
  prefs.putBool("auton", autoName);
  prefs.putString("qry", model ? uidTxt : "");
  prefs.putString("token", www.arg("token"));
  prefs.putString("chat", www.arg("chat"));
  prefs.putString("mqh", www.arg("mqhost"));
  prefs.putString("mqu", www.arg("mquser"));
  prefs.putString("mqp", www.arg("mqpass"));
  prefs.putULong("uid", uid);
  prefs.putInt("poll", allowedPoll(www.arg("poll").toInt()));
  prefs.putInt("tge", allowedEvery(www.arg("tgevery").toInt()));
  prefs.putInt("bl", www.arg("bl") == "1" ? 1 : 0);
  int rot = www.arg("rot").toInt();
  if (rot < 0 || rot > 3) rot = cfgRot & 3;
  prefs.putInt("rot", rot);
  prefs.putBool("mir", www.hasArg("mir"));
  prefs.putUInt("mqport", (uint32_t)port);
  prefs.putUChar("nfy", nfy);
  prefs.end();
  www.sendHeader("Connection", "close");
  www.send(200, "text/html; charset=utf-8",
           "<!DOCTYPE html><meta charset=utf-8><meta name=viewport content=\"width=device-width,initial-scale=1\">"
           "<body style=\"margin:0;background:#101110;color:#f3f3f3;font-family:system-ui,sans-serif;padding:28px\">"
           "<h1 style=\"font-weight:650\">Opgeslagen</h1><p>Het scherm herstart. De MakerWorld-naam wordt vanzelf ingevuld als je het veld leeg liet.</p></body>");
  delay(1200);
  ESP.restart();
}

void handleReset() {
  prefs.begin("cfg", false);
  prefs.clear();
  prefs.end();
  WiFi.disconnect(true, true);
  www.send(200, "text/html; charset=utf-8",
           "<!DOCTYPE html><meta charset=utf-8><meta name=viewport content=\"width=device-width,initial-scale=1\">"
           "<body style=\"margin:0;background:#101110;color:#f3f3f3;font-family:system-ui,sans-serif;padding:28px\">"
           "<h1 style=\"font-weight:650\">Gewist</h1><p>Het scherm start opnieuw en toont het instel-netwerk.</p></body>");
  delay(400);
  ESP.restart();
}

void handleNotFound() {
  if (!portalMode) {
    www.send(404, "text/plain", "niet gevonden");
    return;
  }
  www.sendHeader("Location", "http://192.168.4.1/", true);
  www.send(302, "text/plain", "");
}

void drawPortal(const char* apSsid) {
  int W = tft.width();
  int H = tft.height();
  tft.fillScreen(COL_BG);
  tft.fillRect(0, 0, W, 5, COL_GREEN);
  tft.setTextDatum(top_left);
  tft.setFont(&fonts::Font0);
  tft.setTextColor(COL_GREEN, COL_BG);
  tft.drawString("MAKERPULSE", 14, 14);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_FG, COL_BG);
  tft.drawString("Instellen", 14, 30);

  int y = 64;
  tft.fillCircle(24, y + 6, 9, COL_GREEN);
  tft.setTextDatum(middle_center);
  tft.setFont(&fonts::Font0);
  tft.setTextColor(COL_BG, COL_GREEN);
  tft.drawString("1", 24, y + 6);
  tft.setTextDatum(top_left);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_FG, COL_BG);
  tft.drawString("WiFi", 42, y - 4);
  tft.setFont(&fonts::Font0);
  tft.setTextColor(COL_GREEN, COL_BG);
  tft.drawString(apSsid, 42, y + 16);
  tft.setTextColor(COL_MUTED, COL_BG);
  tft.drawString("wachtwoord   makerpulse", 42, y + 30);

  y = 118;
  tft.fillCircle(24, y + 6, 9, COL_GREEN);
  tft.setTextDatum(middle_center);
  tft.setFont(&fonts::Font0);
  tft.setTextColor(COL_BG, COL_GREEN);
  tft.drawString("2", 24, y + 6);
  tft.setTextDatum(top_left);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_FG, COL_BG);
  tft.drawString("Browser", 42, y - 4);
  tft.setFont(&fonts::Font0);
  tft.setTextColor(COL_GREEN, COL_BG);
  tft.drawString("192.168.4.1", 42, y + 16);
  tft.setTextColor(COL_MUTED, COL_BG);
  tft.drawString("plak een link van je model", 42, y + 30);

  tft.drawString("Daarna herstart dit scherm.", 14, H - 18);
}

bool touchPressed() {
  uint16_t x = 0;
  uint16_t y = 0;
  if (!tft.getTouchRaw(&x, &y)) return false;
  return x >= 50 && y >= 50 && x <= 4000 && y <= 4000;
}

void handleRoot() {
  if (portalMode) {
    sendSetupPage();
    return;
  }
  JsonDocument doc;
  doc["name"] = makerName;
  doc["ip"] = WiFi.localIP().toString();
  doc["firmware"] = MAKERPULSE_FW;
  doc["state"] = blOn ? "ON" : "OFF";
  doc["brightness"] = blBright;
  doc["comments"] = current.comments;
  doc["censusFound"] = censusFound;
  doc["censusTotal"] = censusTotal;
  doc["commentsReady"] = commentsReady;
  String out;
  serializeJson(doc, out);
  sendCors();
  www.send(200, "application/json", out);
}

void setupHttp() {
  static bool started = false;
  if (started) return;
  started = true;
  MDNS.begin("makerpulse-cyd");
  www.on("/", handleRoot);
  www.on("/setup", sendSetupPage);
  www.on("/scan", handleScan);
  www.on("/save", HTTP_POST, handleSave);
  www.on("/reset", HTTP_POST, handleReset);
  www.on("/light", handleLight);
  www.on("/light/", handleLight);
  www.onNotFound(handleNotFound);
  www.begin();
}

void runSetupPortal() {
  portalMode = true;
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  if (mac.length() < 4) mac = "0000";
  String ap = "MakerPulse-" + mac.substring(mac.length() - 4);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap.c_str(), AP_PASS);
  dnsServer.start(53, "*", WiFi.softAPIP());
  setupHttp();
  blOn = true;
  blBright = 255;
  tft.setBrightness(255);
  drawPortal(ap.c_str());
  while (true) {
    dnsServer.processNextRequest();
    www.handleClient();
    delay(2);
  }
}

void serviceNet() {
  www.handleClient();
  serviceTouch();
  serviceBacklight();
  if (mqttEnabled()) {
    ensureMqtt();
    mqtt.loop();
  }
}

void connectWifi() {
  drawStatus("WiFi", cfgSsid);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("makerpulse-cyd");
  WiFi.begin(cfgSsid, cfgPass);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 25000) {
    ledRgb(false, false, true);
    delay(200);
    ledOff();
    delay(200);
  }
}

void waitInterval() {
  unsigned long start = millis();
  unsigned long interval = (unsigned long)cfgPoll * 1000UL;
  while (millis() - start < interval) {
    serviceNet();
    delay(20);
  }
}

void tick() {
  ledRgb(false, false, true);
  bool ok = fetchStats();
  if (!ok) {
    ledRgb(true, false, false);
    if (hasBaseline) current = lastSent;
    drawDashboard();
    delay(800);
    ledOff();
    return;
  }

  if (!commentsReady) {
    ledRgb(true, false, false);
    drawDashboard();
    delay(400);
    ledOff();
    return;
  }

  bool changed = hasBaseline && notableChange();
  if (changed && !commentsNotACollapse(current.comments)) {
    current.comments = lastSent.comments;
    for (int i = 0; i < alertCount; i++) alerts[i].dComments = 0;
    changed = notableChange();
  }
  drawDashboard();

  bool wake = !hasBaseline || statsMoved();
  if (!hasBaseline) {
    saveBaseline();
    stampDigest();
  } else if (!digestDue()) {
    // Period still open: keep the deltas for one summary at the end.
  } else if (changed) {
    ledRgb(false, true, true);
    if (sendTelegram(buildAlert())) {
      saveBaseline();
      stampDigest();
    }
  } else {
    stampDigest();
  }

  publishStats();
  if (wake) noteActivity();
  ledRgb(false, true, false);
  delay(400);
  ledOff();
}

void setup() {
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  ledOff();

  tft.init();
  loadBaseline();
  loadUserConfig();
  applyOrientation();
  if (autoBacklight()) noteActivity();
  applyBacklight(false);

  bool haveWifi = cfgSsid[0] && strcmp(cfgSsid, "JOUW_WIFI_NAAM") != 0 &&
                  strcmp(cfgPass, "JOUW_WIFI_WACHTWOORD") != 0;
  if (!haveWifi) runSetupPortal();

  connectWifi();
  if (WiFi.status() != WL_CONNECTED) {
    strncpy(lastError, "WiFi mislukt", sizeof(lastError) - 1);
    lastOk = false;
    drawStatus("WiFi mislukt", "Tik het scherm");
    return;
  }
  resolveIdentity();
  if (!configUsable()) {
    drawStatus("Model niet gevonden", "Tik om opnieuw");
    while (!touchPressed()) delay(40);
    runSetupPortal();
  }

  if (hasBaseline) {
    current = lastSent;
    lastOk = true;
  }

  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.cloudflare.com");
  setupHttp();
  setupMqtt();
  tft.fillScreen(COL_BG);
  tft.setTextDatum(middle_center);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_FG, COL_BG);
  tft.drawString("Verbonden", tft.width() / 2, tft.height() / 2 - 28);
  tft.setFont(&fonts::Font0);
  tft.setTextColor(COL_MUTED, COL_BG);
  String ip = WiFi.localIP().toString();
  tft.drawString(ip, tft.width() / 2, tft.height() / 2);
  tft.drawString(ip + "/setup", tft.width() / 2, tft.height() / 2 + 16);
  delay(1200);
  tick();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (touchPressed()) runSetupPortal();
    connectWifi();
    if (WiFi.status() == WL_CONNECTED) {
      configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.cloudflare.com");
      setupHttp();
      setupMqtt();
      tick();
    } else {
      drawStatus("WiFi mislukt", "Tik het scherm");
      for (int i = 0; i < 30; i++) {
        if (touchPressed()) runSetupPortal();
        delay(100);
      }
    }
    return;
  }
  serviceNet();
  waitInterval();
  tick();
}
