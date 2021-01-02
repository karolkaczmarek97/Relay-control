/* Karol Kaczmarek 2020
 * 
 * Sterowanie przekaźnikami z obsługą logiki, opóźnień i portu szeregowego
 * 
 * Komendy przesyłane przez port szeregowy mają format
 * <komenda> lub <komenda>.<numer przekaznika> gdzie kropka - separator:
 * CA (close all) - zamknij wszystkie przekaźniki
 * OA (open all) - otwórz wszystkie przekaźniki
 * SA (switch all) - zmień stan wszystkich przekaźników
 * C.1 (close 1) - zamknij przekaźnik nr 1
 * O.1 (open 1) - zamknij przekaźnik nr 1
 * S.1 (switch 1) - zamknij przekaźnik nr 1
 * (itd. ...)
 */

#include <arduino-timer.h>

Timer<2, millis> t_timer;

//unsigned long time = millis();
int RELAY[4] = {5, 6, 7, 8};
int SIGNAL[2] = {9,10};
const int DEFAULT_DELAY_MS = 1000;
const int DEFAULT_STATE = HIGH;
const int DEFAULT_INPUT = INPUT_PULLUP;
const int DEFAULT_INPUT_STATE = HIGH; // Type HIGH if INPUT_PULLUP, LOW if INPUT
String SerialCommand;
String SerialReading = "";
boolean SerialEnd = false;

//----------------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(9600);
  for (int i = 1; i <= (sizeof(RELAY) + 1) / 2; i++) {
    initializeOutput(RELAY[i - 1], DEFAULT_STATE);
  }
  for (int i = 1; i <= (sizeof(SIGNAL) + 1) / 2; i++) {
    initializeInput(SIGNAL[i - 1]);
  }
  Serial.println("Nadawanie jest OK");
  Serial.write("-----------------\n");
}

//----------------------------------------------------------------------------------------------------------

void loop() {

  t_timer.tick();

  if (Serial.available() > 0)
  {
    remote();
  }


  closeRelayOnSignal(0, 0);
  openRelayOnSignal(0, 1);
  //closeRelayOnSignal_Timer(0, 3, 1000);

}

//----------------------------------------------------------------------------------------------------------

void initializeInput(int Pin) {
  pinMode(Pin, DEFAULT_INPUT);
}

void initializeOutput(int Pin, int State) {
  pinMode(Pin, OUTPUT);
  digitalWrite(Pin, State);
}

void remote() {
  while (Serial.available())
  {
    char znak = (char)Serial.read();
    SerialReading += znak;
    if (znak == '\n')
    {
      SerialEnd = true;
    }
    if (SerialEnd == true)
    {
      Serial.print("otrzymano: ");
      Serial.print(SerialReading);
      SerialEnd = false;
      SerialCommand = SerialReading;
      String komenda = SerialCommand.substring(0, SerialCommand.indexOf("."));
      String adres = SerialCommand.substring(SerialCommand.indexOf(".") + 1);
      SerialReading = "";

      if (komenda == "C") {
        closeRelay(adres.toInt() - 1);
      }

      if (komenda == "O") {
        openRelay(adres.toInt() - 1);
      }

      if (komenda == "S") {
        switchRelay(adres.toInt() - 1);
      }

      if (komenda == "CA") {
        for (int i = 0; i < sizeof(RELAY) / 2; i++) {
          closeRelay(i);
        }
      }

      if (komenda == "OA") {
        for (int i = 0; i < sizeof(RELAY) / 2; i++) {
          openRelay(i);
        }
      }

      if (komenda == "SA") {
        for (int i = 0; i < sizeof(RELAY) / 2; i++) {
          switchRelay(i);
        }
      }
    }
  }
}

void switchRelay(int Relay) {
  if (digitalRead(RELAY[Relay]) == HIGH) digitalWrite(RELAY[Relay], LOW); else if (digitalRead(RELAY[Relay]) == LOW) digitalWrite(RELAY[Relay], HIGH);
}

void closeRelay(int Relay) {
  digitalWrite(RELAY[Relay], !DEFAULT_STATE);
}

void openRelay(int Relay) {
  digitalWrite(RELAY[Relay], DEFAULT_STATE);

}

void closeRelayOnSignal(int Relay, int Signal) {
  int state = digitalRead(SIGNAL[Signal]);
  if (state != DEFAULT_INPUT_STATE) {
    closeRelay(Relay);
  }
}

void closeRelayOnSignal_Timer(int Relay, int Signal, int Duration) {
  int state = digitalRead(SIGNAL[Signal]);
  int relaystate = digitalRead(RELAY[Relay]);
  if (relaystate == DEFAULT_STATE && state != DEFAULT_INPUT_STATE) {
    //if (state != DEFAULT_INPUT_STATE) {
    closeRelay(Relay);
    t_timer.in(Duration, openRelay, Relay);

  }
}


void openRelayOnSignal(int Relay, int Signal) {
  int state = digitalRead(SIGNAL[Signal]);
  if (state != DEFAULT_INPUT_STATE) {
    openRelay(Relay);
  }
}

void closeRelayOnSignalOnly(int Relay, int Signal) {
  int state = digitalRead(SIGNAL[Signal]);
  if (state != DEFAULT_INPUT_STATE) {
    closeRelay(Relay);
  } else openRelay(Relay);
}

void openRelayOnSignalOnly(int Relay, int Signal) {
  int state = digitalRead(SIGNAL[Signal]);
  if (state != DEFAULT_INPUT_STATE) {
    openRelay(Relay);
  } else closeRelay(Relay);
}
