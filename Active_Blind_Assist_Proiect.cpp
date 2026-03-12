#include <Arduino.h>
#define TRIG_STANG 7
#define ECHO_STANG 6
#define TRIG_DREPT 5
#define ECHO_DREPT 4
#define BUTON_STANG 2
#define BUTON_DREPT 3
#define LED_STANG 8
#define LED_DREPT 9
#define BUZZER 10
const float DISTANTA_PRAG = 50.0;
const unsigned long INTERVAL_SENZOR = 100;
const int MARIME_FILTRU = 5;
float distanteStang[MARIME_FILTRU];
float distanteDrept[MARIME_FILTRU];
int indexStang = 0;
int indexDrept = 0;
unsigned long ultimaCitireStang = 0;
unsigned long ultimaCitireDrept = 50;
float distantaStangFiltrata = 999.0;
float distantaDreptFiltrata = 999.0;
unsigned long ultimaSchimbareBuzzer = 0;
const unsigned long INTERVAL_BUZZER = 300;
enum StareLaterala {
  SIGUR,
  OBIECT_DETECTAT,
  AVERTISMENT_ACTIV
};
StareLaterala stareStang = SIGUR;
StareLaterala stareDrept = SIGUR;
float citesteUltrasonic(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long durata = pulseIn(echoPin, HIGH, 30000);
  if (durata == 0) return 999.0;
  return durata * 0.034 / 2;
}
float calculeazaMedie(float* arr) {
  float suma = 0;
  for (int i = 0; i < MARIME_FILTRU; i++) suma += arr[i];
  return suma / MARIME_FILTRU;
}
void actualizeazaStare(StareLaterala &stare, bool obiectDetectat, bool butonApasat) {
  switch (stare) {
    case SIGUR:
      if (obiectDetectat) stare = OBIECT_DETECTAT;
      break;
    case OBIECT_DETECTAT:
      if (!obiectDetectat) stare = SIGUR;
      else if (butonApasat) stare = AVERTISMENT_ACTIV;
      break;
    case AVERTISMENT_ACTIV:
      if (!obiectDetectat) stare = SIGUR;
      else if (!butonApasat) stare = OBIECT_DETECTAT;
      break;
  }
}
void setup() {
  pinMode(TRIG_STANG, OUTPUT);
  pinMode(ECHO_STANG, INPUT);
  pinMode(TRIG_DREPT, OUTPUT);
  pinMode(ECHO_DREPT, INPUT);
  pinMode(BUTON_STANG, INPUT_PULLUP);
  pinMode(BUTON_DREPT, INPUT_PULLUP);
  pinMode(LED_STANG, OUTPUT);
  pinMode(LED_DREPT, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  for (int i = 0; i < MARIME_FILTRU; i++) {
    distanteStang[i] = 999.0;
    distanteDrept[i] = 999.0;
  }
}
void loop() {
  unsigned long acum = millis();
  if (acum - ultimaCitireStang >= INTERVAL_SENZOR) {
    ultimaCitireStang = acum;
    distanteStang[indexStang] = citesteUltrasonic(TRIG_STANG, ECHO_STANG);
    distantaStangFiltrata = calculeazaMedie(distanteStang); 
    indexStang = (indexStang + 1) % MARIME_FILTRU;
  }  
  if (acum - ultimaCitireDrept >= INTERVAL_SENZOR) {
    ultimaCitireDrept = acum;
    distanteDrept[indexDrept] = citesteUltrasonic(TRIG_DREPT, ECHO_DREPT);
    distantaDreptFiltrata = calculeazaMedie(distanteDrept);
    indexDrept = (indexDrept + 1) % MARIME_FILTRU;
  }  
  actualizeazaStare(stareStang,
    distantaStangFiltrata < DISTANTA_PRAG,
    digitalRead(BUTON_STANG) == LOW);
  actualizeazaStare(stareDrept,
    distantaDreptFiltrata < DISTANTA_PRAG,
    digitalRead(BUTON_DREPT) == LOW);  
  digitalWrite(LED_STANG, (stareStang != SIGUR) ? HIGH : LOW);
  digitalWrite(LED_DREPT, (stareDrept != SIGUR) ? HIGH : LOW);  
  if (stareStang == AVERTISMENT_ACTIV || stareDrept == AVERTISMENT_ACTIV) {
    if (acum - ultimaSchimbareBuzzer >= INTERVAL_BUZZER) {
      ultimaSchimbareBuzzer = acum;
      digitalWrite(BUZZER, !digitalRead(BUZZER));
    }
  } else {
    digitalWrite(BUZZER, LOW);
  }
}

