#define MUX_S0 0
#define MUX_S1 4
#define MUX_S2 5
#define MUX_S3 16
#define ANALOG_IN A0
#define VOLTAGE_DEADBAND 20
#define NO_TILE 8
#define TILE_1  94
#define TILE_2  874
#define TILE_3  386
#define TILE_4  485
#define TILE_5  224
void setup() {
  Serial.begin(115200);
  
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  pinMode(ANALOG_IN, INPUT);
}
void loop() {
  for (int i = 0; i < 16; i++) {
    int raw = readMuxChannel(i);
    Serial.print(raw); Serial.print(": "); 
    Serial.print("MUX5 "); Serial.print(i); 
    Serial.print('\t');
    delay(100);
  }
  Serial.println();
}
int tileId(int reading) {
  if (abs(reading - NO_TILE) < VOLTAGE_DEADBAND) return 0;
  if (abs(reading - TILE_1) < VOLTAGE_DEADBAND) return 1;
  if (abs(reading - TILE_2) < VOLTAGE_DEADBAND) return 2;
  if (abs(reading - TILE_3) < VOLTAGE_DEADBAND) return 3;
  if (abs(reading - TILE_4) < VOLTAGE_DEADBAND) return 4;
  if (abs(reading - TILE_5) < VOLTAGE_DEADBAND) return 5;
  return 0;
}
int readMuxChannel(int chan) {
  digitalWrite(MUX_S0, (chan & 0b0001) >> 0);
  digitalWrite(MUX_S1, (chan & 0b0010) >> 1);
  digitalWrite(MUX_S2, (chan & 0b0100) >> 2);
  digitalWrite(MUX_S3, (chan & 0b1000) >> 3);
  delay(100);
  return analogRead(ANALOG_IN);
}
