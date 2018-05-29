#define MUX_S0    16
#define MUX_S1    14
#define MUX_S2    12
#define MUX_S3    13
#define ANALOG_IN A0


enum MatchState { STATE_WAIT_FOR_1, STATE_WAIT_FOR_2, STATE_ERROR };
MatchState matchState;


#define NO_TILE 8
#define TILE_1  80
#define TILE_2  874
#define TILE_3  377
#define TILE_4  476
#define TILE_5  212

#define VOLTAGE_DEADBAND 10 // Two-sided deadband

int tile1 = NO_TILE;
int tile2 = NO_TILE;


int score = 0;


#define BOARD_SIZE 2
int originalBoard[BOARD_SIZE][BOARD_SIZE];
int prevBoard[BOARD_SIZE][BOARD_SIZE];
int currBoard[BOARD_SIZE][BOARD_SIZE];


// Function prototypes
void readBoardState(int board[BOARD_SIZE][BOARD_SIZE]);

void setup() {
  Serial.begin(115200);
  
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(ANALOG_IN, INPUT);

  matchState = STATE_WAIT_FOR_1;

  readBoardState(originalBoard);
  copyBoard(originalBoard, prevBoard);
}

void loop() {
//  Serial.print("match state: "); Serial.println(matchState);

  // Update state
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      currBoard[i][j] = tileId(readMuxChannel(i * BOARD_SIZE + j));
//       Serial.print("prev "); Serial.print(prevBoard[i][j]);  Serial.print("curr"); Serial.print(currBoard[i][j]); Serial.print(" ");
      
      // No change
      if (currBoard[i][j] == prevBoard[i][j]) continue;

      if (prevBoard[i][j] == NO_TILE && currBoard[i][j] != NO_TILE) { // Tile flipped face down

      } else if (prevBoard[i][j] != NO_TILE && currBoard[i][j] == NO_TILE) { // Tile flipped face up
        if (matchState == STATE_WAIT_FOR_1) {
          tile1 = originalBoard[i][j];
          matchState = STATE_WAIT_FOR_2;
          continue;
        } else if (matchState == STATE_WAIT_FOR_2) {
          tile2 = originalBoard[i][j];
          checkForTileMatch();
          matchState = STATE_WAIT_FOR_1;
        }
      }
    }
  }
//  Serial.println();
  

  // Prepare for next loop
  copyBoard(currBoard, prevBoard);

  delay(100);
}

void copyBoard(int from[BOARD_SIZE][BOARD_SIZE], int to[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      to[i][j] = from[i][j];
    }
  }
}

void checkForTileMatch() {
  if (tile1 == tile2) {
    score += 1;
    Serial.print("Match found! Score: "); Serial.println(score);
  } else {
    Serial.print("Not a match. Score: "); Serial.println(score);
  }

  tile1 = NO_TILE;
  tile2 = NO_TILE;
}

int tileId(int reading) {
  if (abs(reading - 0) < VOLTAGE_DEADBAND) return 0;
  if (abs(reading - TILE_1) < VOLTAGE_DEADBAND) return 1;
  if (abs(reading - TILE_2) < VOLTAGE_DEADBAND) return 2;
  if (abs(reading - TILE_3) < VOLTAGE_DEADBAND) return 3;
  if (abs(reading - TILE_4) < VOLTAGE_DEADBAND) return 4;
  if (abs(reading - TILE_5) < VOLTAGE_DEADBAND) return 5;
  return 0;
}

void readBoardState(int board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = tileId(readMuxChannel(i * BOARD_SIZE + j));
      Serial.print(board[i][j]); Serial.print(" ");
    }
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
