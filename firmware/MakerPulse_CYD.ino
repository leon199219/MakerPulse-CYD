// MakerPulse CYD 2026.09.12a
#define MAKERPULSE_FW "2026.09.12a"
/*
 * ========== WHAT DO YOU NEED TO CHANGE? ==========
 *
 * A) Zip from the MakerPulse configurator
 *    Nothing. Wi-Fi, MakerWorld ID, Telegram and MQTT are already in config.h.
 *    Choose board "ESP32 Dev Module" and click Upload.
 *
 * B) From GitHub / without the configurator
 *    1. Copy config.h.example to config.h (same folder as this .ino).
 *    2. Open config.h — every line has an English comment.
 *    3. Replace JOUW_WIFI_NAAM, JOUW_WIFI_WACHTWOORD and MAKERWORLD_UID 0UL.
 *    Or: no config.h? Fill in the "Manual settings" block below.
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

class CydDisplay : public lgfx::LGFX_Device {
#if CYD_PANEL_ST7789
  lgfx::Panel_ST7789 _panel_instance;
#else
  lgfx::Panel_ILI9341 _panel_instance;
#endif
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM _light_instance;

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
    setPanel(&_panel_instance);
  }
};

static const char MW_UA[] =
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";

CydDisplay tft;
Preferences prefs;
Stats current = {0, 0, 0, 0, 0, 0};
Stats lastSent = {0, 0, 0, 0, 0, 0};
ModelSnap models[MAX_MODELS];
int modelCount = 0;
AllSnap allSnaps[MAX_ALL_MODELS];
AllSnap prevAllSnaps[MAX_ALL_MODELS];
int allSnapCount = 0;
int prevAllSnapCount = 0;
ModelAlert alerts[MAX_ALERTS];
int alertCount = 0;
bool hasBaseline = false;
char makerName[24] = MAKER_NAME;
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
char mqttClientId[28] = "";
char topicCmd[48] = "";
char topicState[48] = "";
char topicAvail[48] = "";
char topicDisc[64] = "";

void serviceNet();

bool mqttEnabled() { return MQTT_HOST[0] != '\0'; }

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

  tft.setTextDatum(middle_left);
  tft.setFont(&fonts::Font2);
  tft.setTextColor(COL_FG, COL_HEAD);
  tft.drawString(makerName, 8, headH - 14);

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
               POLL_INTERVAL_SEC);
    } else {
      snprintf(foot, sizeof(foot), "check %s  ·  %ds", clockStr().c_str(), POLL_INTERVAL_SEC);
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
    bool tg = strlen(TELEGRAM_BOT_TOKEN) > 0 && strlen(TELEGRAM_CHAT_ID) > 0;
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
  if (!NOTIFY_DOWNLOADS) dDl = 0;
  if (!NOTIFY_LIKES) dLk = 0;
  if (!NOTIFY_PRINTS) dPr = 0;
  if (!NOTIFY_BOOSTS) dBs = 0;
  if (!NOTIFY_COLLECTIONS) dCo = 0;
  if (!NOTIFY_COMMENTS) dCm = 0;
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

bool fetchPublishedPage(HTTPClient& http, WiFiClientSecure& client, int offset, String& body) {
  String url = String("https://api.bambulab.com/v1/design-service/publisheddesigns/") +
               String((unsigned long)MAKERWORLD_UID) + "?type=ALL&limit=" + String(PUBLISH_LIMIT) +
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
    for (int i = 0; i < nHits; i++) ingestPubHit(acc, pageHits[i]);
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
                 String((unsigned long)MAKERWORLD_UID);
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

    long downloads = doc["downloadCount"] | 0;
    long designDl = doc["MWCount"]["myDesignDownloadCount"] | 0;
    if (downloads == 0 && designDl > 0) downloads = designDl;

    current.downloads = downloads;
    current.likes = doc["likeCount"] | 0;
    current.prints = doc["MWCount"]["myDesignPrintCount"] | 0;
    current.boosts = doc["boostGained"] | 0;
    current.collections = doc["collectionCount"] | 0;
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
  return strlen(TELEGRAM_BOT_TOKEN) > 0 && strlen(TELEGRAM_CHAT_ID) > 0;
}

bool sendTelegram(const String& text) {
  if (!telegramEnabled()) return true;
  String url = String("https://api.telegram.org/bot") + TELEGRAM_BOT_TOKEN + "/sendMessage";
  JsonDocument msg;
  msg["chat_id"] = TELEGRAM_CHAT_ID;
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

String buildAlert() {
  String s;
  s.reserve(3600);
  s += "MakerPulse · ";
  s += makerName;
  s += "\n\n";
  appendTotal(s, "Downloads", current.downloads, lastSent.downloads);
  appendTotal(s, "Likes", current.likes, lastSent.likes);
  appendTotal(s, "Prints", current.prints, lastSent.prints);
  appendTotal(s, "Boosts", current.boosts, lastSent.boosts);
  appendTotal(s, "Collecties", current.collections, lastSent.collections);
  appendTotal(s, "Comments", current.comments, lastSent.comments);

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
  if (NOTIFY_DOWNLOADS && current.downloads != lastSent.downloads) return true;
  if (NOTIFY_LIKES && current.likes != lastSent.likes) return true;
  if (NOTIFY_PRINTS && current.prints != lastSent.prints) return true;
  if (NOTIFY_BOOSTS && current.boosts != lastSent.boosts) return true;
  if (NOTIFY_COLLECTIONS && current.collections != lastSent.collections) return true;
  if (NOTIFY_COMMENTS && current.comments != lastSent.comments) return true;
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

void applyBacklight(bool pub) {
  tft.setBrightness(blOn ? blBright : 0);
  saveBacklight();
  if (pub) publishLightState();
}

void parseLightJson(const String& body) {
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
  mqtt.publish(topicDisc, payload.c_str(), true);
}

void ensureMqtt() {
  if (!mqttEnabled()) return;
  if (mqtt.connected()) return;
  static unsigned long lastTry = 0;
  if (millis() - lastTry < 4000) return;
  lastTry = millis();
  bool ok;
  if (MQTT_USER[0]) {
    ok = mqtt.connect(mqttClientId, MQTT_USER, MQTT_PASS, topicAvail, 1, true, "offline");
  } else {
    ok = mqtt.connect(mqttClientId, topicAvail, 1, true, "offline");
  }
  if (!ok) return;
  mqtt.publish(topicAvail, "online", true);
  mqtt.subscribe(topicCmd);
  publishDiscovery();
  publishLightState();
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
  snprintf(topicDisc, sizeof(topicDisc), "homeassistant/light/%s/config", mqttClientId);
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(1024);
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

void handleRoot() {
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
  MDNS.begin("makerpulse-cyd");
  www.on("/", handleRoot);
  www.on("/light", handleLight);
  www.on("/light/", handleLight);
  www.begin();
}

void serviceNet() {
  www.handleClient();
  if (mqttEnabled()) {
    ensureMqtt();
    mqtt.loop();
  }
}

void connectWifi() {
  drawStatus("WiFi", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("makerpulse-cyd");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
  unsigned long interval = (unsigned long)POLL_INTERVAL_SEC * 1000UL;
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

  if (!hasBaseline) {
    saveBaseline();
  } else if (changed) {
    ledRgb(false, true, true);
    if (sendTelegram(buildAlert())) {
      saveBaseline();
    }
  } else if (prevAllSnapCount == 0 && allSnapCount > 0) {
    saveBaseline();
  }

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
  tft.setRotation(CYD_ROTATION);
  loadBaseline();
  applyBacklight(false);
  drawStatus("MakerPulse", "CYD start...");

  if (MAKERWORLD_UID == 0 || WIFI_SSID[0] == 0 || strcmp(WIFI_SSID, "JOUW_WIFI_NAAM") == 0 ||
      strcmp(WIFI_PASSWORD, "JOUW_WIFI_WACHTWOORD") == 0) {
    drawStatus("Vul config.h in", "WiFi + MakerWorld-ID");
    return;
  }

  if (hasBaseline) {
    current = lastSent;
    lastOk = true;
    drawDashboard();
  }

  connectWifi();
  if (WiFi.status() != WL_CONNECTED) {
    strncpy(lastError, "WiFi mislukt", sizeof(lastError) - 1);
    lastOk = false;
    drawDashboard();
    return;
  }

  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.cloudflare.com");
  setupHttp();
  setupMqtt();
  drawStatus("MakerWorld", WiFi.localIP().toString().c_str());
  delay(800);
  tick();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
    if (WiFi.status() == WL_CONNECTED) {
      configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.cloudflare.com");
      setupHttp();
      setupMqtt();
      tick();
    } else {
      delay(4000);
    }
    return;
  }
  serviceNet();
  waitInterval();
  tick();
}
