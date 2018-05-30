//////////////////////////////////////
//            DEFINITIONS           //
//////////////////////////////////////

//      Libraries      //
#include <Wire.h>
#include <Ticker.h>
#include <EEPROM.h>

#include "AdafruitIO_WiFi.h"
#include <ESP8266HTTPClient.h>


//      WiFi      //
#define WIFI_SSID "Claire"
#define WIFI_PASS "colebluestamp"


//      Adafruit IO      //
#define IO_USERNAME "claire_h"
#define IO_KEY "e5209e5b9fe4409eb29e6fb17d3ef8a8"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
AdafruitIO_Feed *feed = io.feed("me216m-synapse.events");


//      Multiplexer pins     //
#define MUX_S0    16
#define MUX_S1    14
#define MUX_S2    12
#define MUX_S3    13
#define ANALOG_IN A0


//      Button pins     //
#define SELECT_BUTTON 4
#define UP_BUTTON     2
#define DOWN_BUTTON   5


//      State machines     //
enum SetupState { STATE_WAIT_FOR_START, STATE_GAME_STARTED };
enum MatchState { STATE_WAIT_FOR_1, STATE_WAIT_FOR_2, STATE_ERROR };
SetupState setupState;
MatchState matchState;


//      Tile voltage readings (IDs)     //
#define NO_TILE 0
#define EMPTY   8
#define TILE_1  94
#define TILE_2  874
#define TILE_3  386
#define TILE_4  485
#define TILE_5  224

#define VOLTAGE_DEADBAND 20 // Two-sided deadband

int tile1 = NO_TILE;
int tile2 = NO_TILE;

int gameId = 0;
int score = 0;
String prefix = "";

//      Stored game state     //
#define NUM_TILES 10
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

  pinMode(SELECT_BUTTON, INPUT);
  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);

  // Connect to WiFi and io.adafruit.com
  io.connect();
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi and Adafruit IO initialized");
  Serial.println(io.statusText());

  setupState = STATE_WAIT_FOR_START;
  matchState = STATE_WAIT_FOR_1;
}


//////////////////////////////////////
//                LOOP              //
//////////////////////////////////////
void loop() {
  io.run(); // Stay connected to Adafruit IO
  
  // When select button is pressed, start game (will be replaced by more complex display logic later)
  if (setupState == STATE_WAIT_FOR_START && digitalRead(SELECT_BUTTON) == HIGH) {
    gameId++;
    prefix = "gameId: "; // TODO: REPLACE WITH LESS JANK
    prefix += gameId;
    reportGameStart();

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
    currBoard[i] = tileId(readMuxChannel(i));
       Serial.print("\tprev "); Serial.print(prevBoard[i]);  Serial.print("\tcurr"); Serial.print(currBoard[i]); Serial.print(" ");

    if (currBoard[i] != NO_TILE) numUnmatched++;
    
    // Tile hasn't changed
    if (currBoard[i] == prevBoard[i]) continue;

    // Tile flipped face down
    if (prevBoard[i] == NO_TILE && currBoard[i] != NO_TILE) { 
      reportTileFlipDown(currBoard[i]);
      
      if (matchState == STATE_WAIT_FOR_2) {
        tile1 = NO_TILE;
        matchState = STATE_WAIT_FOR_1;
      }

    // Tile flipped face up
    } else if (prevBoard[i] != NO_TILE && currBoard[i] == NO_TILE) {
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
  delay(100);

  return (numUnmatched > 0); // Continue game if still tiles left to match
}

void copyBoard(int from[NUM_TILES], int to[NUM_TILES]) {
  for (int i = 0; i < NUM_TILES; i++) {
    to[i] = from[i];
  }
}

void checkForTileMatch() {
  if (tile1 == tile2) {
    score++;
    reportTileMatch(tile1, score);
  } else {
    score--;
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
  return 0;
}

void readBoardState(int board[NUM_TILES]) {
  for (int i = 0; i < NUM_TILES; i++) {
    board[i] = tileId(readMuxChannel(i));
    Serial.print(board[i]); Serial.print(" ");
  }
  Serial.println();
}

int readMuxChannel(int chan) {
  digitalWrite(MUX_S0, (chan & 0b0001) >> 0);
  digitalWrite(MUX_S1, (chan & 0b0010) >> 1);
  digitalWrite(MUX_S2, (chan & 0b0100) >> 2);
  digitalWrite(MUX_S3, (chan & 0b1000) >> 3);
  delay(100);
  return analogRead(ANALOG_IN);
}



void reportGameStart() {
  Serial.print("reporting: GAME STARTED ");
  Serial.println(gameId);
  String str = prefix;
  str += ", event: game start";
  feed->save(str);
}

void reportGameEnd(int score) {
  Serial.println("reporting: GAME ENDED");
  String str = prefix;
  str += ", event: game end, score: ";
  str += score;
  feed->save(str);
}

void reportTileFlipUp(int tileId) {
  Serial.println("reporting: TILE FLIPPED UP");
  String str = prefix;
  str += ", event: tile flipped up, tileId: ";
  str += tileId;
  feed->save(str);
}

void reportTileFlipDown(int tileId) {
  Serial.println("reporting: TILE FLIPPED DOWN");
  String str = prefix;
  str += ", event: tile flipped down, tileId: ";
  str += tileId;
  feed->save(str);
}

void reportTileMatch(int tileId, int score) {
  Serial.println("reporting: TILE MATCH");
  Serial.print("Match found! Score: "); Serial.println(score);
  String str = prefix;
  str += ", event: tile match, tileId: ";
  str += tileId;
  feed->save(str);
}

void reportTileMismatch(int tile1, int tile2, int score) {
  Serial.println("reporting: TILE MISMATCH");
  Serial.print("Not a match. Score: "); Serial.println(score);
  String str = prefix;
  str += ", event: tile mismatch, tileId1: ";
  str += tile1;
  str += ", tileId2: ";
  str += tile2;
  feed->save(str);
}
