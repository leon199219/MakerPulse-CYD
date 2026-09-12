#pragma once
#include <stdint.h>

// Types must live in a header so Arduino's auto-prototypes see them.
static const int MAX_MODELS = 12;
static const int MAX_ALL_MODELS = 400;
static const int PUBLISH_LIMIT = 10;
static const int MAX_PUBLISH_PAGES = 40;
static const int MAX_ALERTS = 40;

struct Stats {
  long downloads;
  long likes;
  long prints;
  long boosts;
  long collections;
  long comments;
};

struct ModelSnap {
  long id;
  long downloads;
  long likes;
  long prints;
  long boosts;
  long collections;
  long comments;
  char title[36];
};

struct AllSnap {
  uint32_t id;
  uint32_t downloads;
  uint32_t likes;
  uint32_t prints;
  uint32_t boosts;
  uint32_t collections;
  uint32_t comments;
};

struct ModelAlert {
  char title[36];
  long dDownloads;
  long dLikes;
  long dPrints;
  long dBoosts;
  long dCollections;
  long dComments;
};

struct PubHit {
  uint32_t id;
  uint32_t downloads;
  uint32_t likes;
  uint32_t prints;
  uint32_t boosts;
  uint32_t collections;
  uint32_t comments;
  char title[36];
};

struct JsonCur {
  const char* s;
  int n;
  int i;
};

struct WalkAcc {
  long commentSum;
  int found;
  const long* ids;
  int nids;
  ModelSnap featuredGot[MAX_MODELS];
  bool featuredOk[MAX_MODELS];
};
