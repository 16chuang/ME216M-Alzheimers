#define MUX_S0 16
#define MUX_S1 14
#define MUX_S2 12
#define MUX_S3 13

#define ANALOG_IN A0

#define VOLTAGE_DEADBAND 2

void setup() {
  Serial.begin(115200);
  
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  pinMode(ANALOG_IN, INPUT);
}

void loop() {
  for (int i = 0; i < 4; i++) {
    Serial.print(readMuxChannel(i)); Serial.print('\t');
    delay(100);
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

