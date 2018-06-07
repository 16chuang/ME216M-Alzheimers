//////////////////////////////////////
//            DEFINITIONS           //
//////////////////////////////////////

//      Libraries      //
#include <Wire.h>
#include <Ticker.h>
#include <EEPROM.h>
#include <SPI.h>
#include <GxEPD.h>
#include <GxGDEH029A1/GxGDEH029A1.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

#include "AdafruitIO_WiFi.h"
#include <ESP8266HTTPClient.h>
#include "logo.h"


//      WiFi      //
//#define WIFI_SSID "Claire"
//#define WIFI_PASS "colebluestamp"
 #define WIFI_SSID "ME216M Wi-Fi Network" // WiFi name
 #define WIFI_PASS "me216marduino" // WiFi password
//#define WIFI_SSID "LOFT" // WiFi name
//#define WIFI_PASS "loftloft" // WiFi password

//      Adafruit IO      //
#define IO_USERNAME "claire_h"
#define IO_KEY "e5209e5b9fe4409eb29e6fb17d3ef8a8"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *feed = io.feed("me216m-synapse.events");
AdafruitIO_Feed *flipsFeed = io.feed("me216m-synapse.num-flips");
AdafruitIO_Feed *secondsFeed = io.feed("me216m-synapse.num-seconds");

#define SHOULD_REPORT true


//      Multiplexer pins     //
#define MUX_S0 0
#define MUX_S1 4
#define MUX_S2 5
#define MUX_S3 16
#define ANALOG_IN A0


//      Display pins     //
#define RST  -1
#define DIN  13
#define CLK  14
#define BUSY 12
#define CS   15
#define DC   2

GxIO_Class display_io(SPI, CS, DC, RST);
GxEPD_Class display(display_io, RST, BUSY);


//      Button pins (in mux)     //
#define SELECT_BUTTON 8
#define UP_BUTTON     11
#define DOWN_BUTTON   10

//      State machines     //
enum SetupState { STATE_WAIT_FOR_START, STATE_GAME_STARTED };
enum MatchState { STATE_WAIT_FOR_1, STATE_WAIT_FOR_2, STATE_ERROR };
SetupState setupState;
MatchState matchState;


//      Tile voltage readings (IDs)     //
#define NO_TILE 0
#define EMPTY   8
#define TILE_1  770
#define TILE_2  600
#define TILE_3  340
#define TILE_4  220
#define TILE_5  165
#define TILE_6  60

#define VOLTAGE_DEADBAND 30 // Two-sided deadband

int tile1 = NO_TILE;
int tile2 = NO_TILE;

int gameId = 0;
int score = 0;
int numFlips = 0;
unsigned long gameStartTime;
String prefix = "";

//      Stored game state     //
#define NUM_TILES 12
int tilePins[] = {0, 1, 2, 3, 4, 5, 6, 7, 9, 13, 14, 15};
// Board store IDs
int originalBoard[NUM_TILES];
int prevBoard[NUM_TILES];
int currBoard[NUM_TILES];


//////////////////////////////////////
//            PROTOTYPES            //
//////////////////////////////////////
void readBoardState(int board[NUM_TILES]);


//////////////////////////////////////
//              SETUP               //
//////////////////////////////////////
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);

  //  EEPROM.write(0,0);
  gameId = EEPROM.read(0);

  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(ANALOG_IN, INPUT);

  setupState = STATE_WAIT_FOR_START;
  matchState = STATE_WAIT_FOR_1;

  display.init();
  displayLogo(true);

  // Connect to WiFi and io.adafruit.com
  io.connect();
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi and Adafruit IO initialized");
  Serial.println(io.statusText());

  delay(1000);
  displayStartScreen();
}


//////////////////////////////////////
//                LOOP              //
//////////////////////////////////////
void loop() {
  io.run(); // Stay connected to Adafruit IO

  // When select button is pressed, start game (will be replaced by more complex display logic later)
  if (setupState == STATE_WAIT_FOR_START && readMuxChannel(SELECT_BUTTON) > 1000) {
    score = 0;
    numFlips = 0;
    gameId++;
    gameStartTime = millis();
    prefix = "gameId: "; // TODO: REPLACE WITH LESS JANK
    prefix += gameId;
    reportGameStart();
    updateDisplayScore();

    // Prepare for game
    readBoardState(originalBoard);
    copyBoard(originalBoard, prevBoard);
    EEPROM.write(0, gameId);
    EEPROM.commit();

    // Play game
    while (playGame());

    // Game over
    reportGameEnd(score);
    setupState = STATE_WAIT_FOR_START;

    displayEndGame();
    delay(2000);
    displayLogo(false);
    delay(3000);
    displayStartScreen();
  }

  delay(100);
}


//////////////////////////////////////
//              HELPERS             //
//////////////////////////////////////
bool playGame() {
  //  Serial.print("match state: "); Serial.println(matchState);

  int numUnmatched = 0;

  // Update state
  for (int i = 0; i < NUM_TILES; i++) {
    // Read tile ID at current position
    currBoard[i] = tileId(readMuxChannel(tilePins[i]));
    //    Serial.print(tilePins[i]);
    //    Serial.print(" ");
    Serial.print("p "); Serial.print(prevBoard[i]);  Serial.print(" c ");
    Serial.print(currBoard[i]);
    Serial.print(" ");
    Serial.print(readMuxChannel(tilePins[i]));
    Serial.print(" \t");

    if (currBoard[i] != NO_TILE) numUnmatched++;

    // Tile hasn't changed
    if (currBoard[i] == prevBoard[i]) continue;

    // Tile flipped face down
    if (prevBoard[i] == NO_TILE && currBoard[i] > NO_TILE) {
      reportTileFlipDown(currBoard[i]);

      if (matchState == STATE_WAIT_FOR_2) {
        tile1 = NO_TILE;
        matchState = STATE_WAIT_FOR_1;
      }

      // Tile flipped face up
    } else if (prevBoard[i] > NO_TILE && currBoard[i] == NO_TILE) {
      int tileId = originalBoard[i];
      reportTileFlipUp(tileId);

      if (matchState == STATE_WAIT_FOR_1) {
        tile1 = tileId;
        matchState = STATE_WAIT_FOR_2;
        continue;
      } else if (matchState == STATE_WAIT_FOR_2) {
        tile2 = tileId;
        checkForTileMatch();
        matchState = STATE_WAIT_FOR_1;
      }
    }
  }
  Serial.println();

  // Prepare for next loop
  copyBoard(currBoard, prevBoard);

  return (numUnmatched > 0); // Continue game if still tiles left to match
}

void copyBoard(int from[NUM_TILES], int to[NUM_TILES]) {
  for (int i = 0; i < NUM_TILES; i++) {
    to[i] = from[i];
  }
}

void checkForTileMatch() {
  numFlips++;

  if (tile1 == tile2) {
    score++;
    updateDisplayScore();
    reportTileMatch(tile1, score);
  } else {
    reportTileMismatch(tile1, tile2, score);
  }

  tile1 = NO_TILE;
  tile2 = NO_TILE;
}

int tileId(int reading) {
  if (abs(reading - EMPTY) < VOLTAGE_DEADBAND) return 0;
  if (abs(reading - TILE_1) < VOLTAGE_DEADBAND) return 1;
  if (abs(reading - TILE_2) < VOLTAGE_DEADBAND) return 2;
  if (abs(reading - TILE_3) < VOLTAGE_DEADBAND) return 3;
  if (abs(reading - TILE_4) < VOLTAGE_DEADBAND) return 4;
  if (abs(reading - TILE_5) < VOLTAGE_DEADBAND) return 5;
  if (abs(reading - TILE_6) < VOLTAGE_DEADBAND) return 6;
  return -1;
}

void readBoardState(int board[NUM_TILES]) {
  for (int i = 0; i < NUM_TILES; i++) {
    board[i] = tileId(readMuxChannel(tilePins[i]));
    //    Serial.print(board[i]); Serial.print(" ");
  }
  //  Serial.println();
}

int readMuxChannel(int chan) {
  digitalWrite(MUX_S0, (chan & 0b0001) >> 0);
  digitalWrite(MUX_S1, (chan & 0b0010) >> 1);
  digitalWrite(MUX_S2, (chan & 0b0100) >> 2);
  digitalWrite(MUX_S3, (chan & 0b1000) >> 3);
  delay(100);
  return analogRead(ANALOG_IN);
}



void displayLogo(bool showLoading) {
  display.fillScreen(GxEPD_WHITE);
  display.setRotation(3);
  display.drawBitmap(bitmapLogo, 25, 30, 80, 72, GxEPD_BLACK);

  display.setTextColor(GxEPD_BLACK);
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(120, 75);
  display.println("Synapse");

  if (showLoading) {
    display.setFont(&FreeSans9pt7b);
    display.setCursor(115, 120);
    display.println("Loading...");
  }

  display.update();
}

void displayStartScreen() {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(3);

  display.setFont(&FreeSans12pt7b);
  display.setCursor(20, 75);
  display.println("Start 1 player game?");

  display.update();
}

void displayEndGame() {
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(3);

  display.setFont(&FreeSans9pt7b);
  display.setCursor(110, 120);
  display.println("End of game");

  display.update();
}

void updateDisplayScore() {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.setRotation(3);

  display.setFont(&FreeSans12pt7b);
  display.setCursor(30, 60);
  display.println("NUMBER OF");
  display.setCursor(30, 90);
  display.println("MATCHES");

  display.setFont(&FreeSansBold24pt7b);
  display.setCursor(230, 82);
  String scoreStr = "";
  scoreStr += score;
  display.println(scoreStr);
  display.update();
}



void reportGameStart() {
  Serial.print("reporting: GAME STARTED ");
  Serial.println(gameId);
  String str = prefix;
  str += ", event: game start";
  if (SHOULD_REPORT) feed->save(str);
}

void reportGameEnd(int score) {
  Serial.println("reporting: GAME EN DED");
  String str = prefix;
  str += ", event: game end, score: ";
  str += score;
  if (SHOULD_REPORT) {
    feed->save(str);
    flipsFeed->save(numFlips);
    secondsFeed->save( (millis() - gameStartTime) / 1000 );
  }
}

void reportTileFlipUp(int tileId) {
  Serial.println("reporting: TILE FLIPPED UP");
  String str = prefix;
  str += ", event: tile flipped up, tileId: ";
  str += tileId;
  if (SHOULD_REPORT) feed->save(str);
}

void reportTileFlipDown(int tileId) {
  Serial.println("reporting: TILE FLIPPED DOWN");
  String str = prefix;
  str += ", event: tile flipped down, tileId: ";
  str += tileId;
  if (SHOULD_REPORT) feed->save(str);
}

void reportTileMatch(int tileId, int score) {
  Serial.println("reporting: TILE MATCH");
  Serial.print("Match found! Score: "); Serial.println(score);
  String str = prefix;
  str += ", event: tile match, tileId: ";
  str += tileId;
  if (SHOULD_REPORT) feed->save(str);
}

void reportTileMismatch(int tile1, int tile2, int score) {
  Serial.println("reporting: TILE MISMATCH");
  Serial.print("Not a match. Score: "); Serial.println(score);
  String str = prefix;
  str += ", event: tile mismatch, tileId1: ";
  str += tile1;
  str += ", tileId2: ";
  str += tile2;
  if (SHOULD_REPORT) feed->save(str);
}
