  /* ------------------------------------------------------------------ HEADER FILE ATAU LIBRARY */
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

/* ------------------------------------------------------------------ DEKLARASI PIN INPUT ATAU OUTPUT YANG DIGUNAKAN */
#define Temp_Pin 12
#define Turbidity_Pin A0
#define Trig_Pin 5
#define Echo_Pin 4
#define PompaIn 10
#define PompaOut 11
#define Heater 8
#define Lampu 9
#define OFF 1
#define ON 0

/* ------------------------------------------------------------------ KOMUNIKASI SERIAL RX TX, di PIN 7 dan 6 Arduino */
SoftwareSerial mySerial(7, 6);

/* ------------------------------------------------------------------ PENDEFINISIAN LCD (ALAMAT DAN UKURAN) */
LiquidCrystal_I2C lcd(0x27, 16, 2);

/* ------------------------------------------------------------------ PIN SENSOR TEMPERATURE */
OneWire oneWire(Temp_Pin);
DallasTemperature sensors(&oneWire);

/* ------------------------------------------------------------------ VARIABLE DATA SUHU DAN TEGANGAN */
float suhu,
      volt,
      xx;

/* ------------------------------------------------------------------ VARIABLE DATA NTU */
int  filter_ntu,
     hasil_ntu,
     ntua,
     ntu,
     ntu_rata;

/* ------------------------------------------------------------------ VARIABLE DATA KETINGGIAN AIR */
long distance,
     duration,
     tinggiAir;

/* ------------------------------------------------------------------ SYMBOL DERAJAT */
byte Derajat = B11011111;

/* ------------------------------------------------------------------ VARIABLE LOGIC PEMBUANGAN DAN PENGISIAN AIR */
boolean buang = false,
        isi = false;


/* ------------------------------------------------------------------ Pembacaan Data NTU */
void baca_ntu()
{
  /* ------------------------------------------------------------------ FILTER Pembacaan Data NTU */
  filter_ntu = 0;
  for ( int i = 1; i <= 20; i++)
  {
    filter_ntu += analogRead(Turbidity_Pin);
  }
  /* ------------------------------------------------------------------ PROSES OLAH Data NTU */
  hasil_ntu = filter_ntu / 20;
  ntua = map(hasil_ntu, 910, 960, 1024, 0);
  ntu_rata = constrain(ntua, 0, 1024);
  ntu = ntu_rata;

  /* ------------------------------------------------------------------ KIRIM Data NTU ke NODEMCU */
  mySerial.print("#" + String(ntu) + "$");

}


/* ------------------------------------------------------------------ Tampilan Seluruh Data Sensor*/
void dataDisplay()
{
  /* ------------------------------------------------------------------ Tampilan Data Sensor di LCD */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(suhu, 1);
  lcd.setCursor(6, 0);
  lcd.write(Derajat);
  lcd.print("C");
  lcd.setCursor(9, 0);
  lcd.print("D:");

  /* ------------------------------------------------------------------ Data TINGGI AIR di LCD */
  if (tinggiAir < 10)
  {
    lcd.setCursor(11, 0);
    lcd.print(" ");
    lcd.print(tinggiAir);
    lcd.print(" cm");
  }
  else
  {
    lcd.setCursor(11, 0);
    lcd.print(tinggiAir);
    lcd.print(" cm");
  }

  /* ------------------------------------------------------------------ Data NTU di LCD */
  lcd.setCursor(0, 1);
  lcd.print("NTU:");
  lcd.print(ntu);


  lcd.setCursor(10, 1);
  /* JIKA NTU 0 - 450, Maka Air Dalam Kondisi JERNIH */
  if (ntu >= 0 && ntu <= 450) lcd.print("JERNIH ");
  /* JIKA NTU 400 - 650, Maka Air Dalam Kondisi CUKUP JERNIH */
  else if (ntu >= 400 && ntu <= 650)  lcd.print("CUKUP");
  /* JIKA NTU 600 - 1024, Maka Air Dalam Kondisi KERUH */
  else if (ntu >= 600 && ntu <= 1024) lcd.print("KERUH");

  /* ------------------------------------------------------------------ Tampilan Data Sensor di SERIAL MONITOR */
  Serial.println("");
  Serial.print("Volt: ");
  Serial.print(volt);
  Serial.print("   |  NTU: ");
  Serial.print(filter_ntu);
  Serial.print("   |  Temp: ");
  Serial.print(suhu);
  Serial.print("   |  Tinggi Air: ");
  Serial.print(tinggiAir);
}


/* ------------------------------------------------------------------ Pembacaan Data SUHU */
void baca_suhu()
{
  /* ------------------------------------------------------------------ Pembacaan Sensor TEMPERATURE */
  sensors.requestTemperatures();
  suhu = sensors.getTempCByIndex(0) + 0.3;

  /* ------------------------------------------------------------------ Kirim Data Suhu ke NODEMCU */
  mySerial.print("#" + String(suhu) + "!");

}


/* ------------------------------------------------------------------ Pembacaan Data TINGGI AIR */
void Tinggi_Air(){
/* ------------------------------------------------------------------ Pembacaan Sensor UTRASONIC */
digitalWrite(Trig_Pin, LOW);
delayMicroseconds(2);
digitalWrite(Trig_Pin, HIGH);
delayMicroseconds(10);
digitalWrite(Trig_Pin, LOW);
duration = pulseIn(Echo_Pin, HIGH);
distance = (duration / 2) / 29.1;

/* ------------------------------------------------------------------ Proses Pengolahan Data TINGGI AIR*/
tinggiAir = 19 - distance;

/* ------------------------------------------------------------------ KIRIM Data TINGGI AIR ke NOEMCU */
mySerial.print("#" + String(tinggiAir) + "@");
}


/* ------------------------------------------------------------------ VOID SETUP() */
void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
  sensors.begin();
  lcd.init(); lcd.backlight();
  pinMode(Turbidity_Pin, INPUT);
  pinMode(Echo_Pin, INPUT);
  pinMode(Trig_Pin, OUTPUT);
  pinMode(PompaIn,  OUTPUT);
  pinMode(PompaOut, OUTPUT);
  pinMode(Heater,   OUTPUT);
  pinMode(Lampu,   OUTPUT);
  digitalWrite(PompaIn, OFF);
  digitalWrite(PompaOut, OFF);
  digitalWrite(Heater, OFF);
  digitalWrite(Lampu, ON);
}


/* ------------------------------------------------------------------ VOID LOOP() */
void loop()
{
  /* ------------------------------------------------------------------ RUN FUNGSI dataDisplay() */
  dataDisplay();
  /* ------------------------------------------------------------------ RUN FUNGSI baca_ntu() */
  baca_ntu();
  /* ------------------------------------------------------------------ RUN FUNGSI baca_suhu() */
  baca_suhu();
  /* ------------------------------------------------------------------ RUN FUNGSI Tinggi_Air() */
  Tinggi_Air();


  /* -------------------------------- FUZZY LOGIC ---------------------------------- */
  /* ------------------------------------------------------------------ SUHU DINGIN */

  /* -------------------------------- SUHU DINGIN && AIR JERNIH ---------------------------------- */
  if ( (suhu >= 0 && suhu <= 25) && (ntu >= 0 && ntu <= 450) )
  {
    /*  TAMPIL SERIAL MONITOR */
    Serial.print("DINGIN");
    Serial.print("  |  AIR JERNIH");
    /* HEATER ON */
    digitalWrite(Heater, ON);
    /* Kirim KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PENGISIAN AIR Jika Air Kurang 15 CM ---------------------------------- */
    if (tinggiAir <= 15)
    {
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          delay(500);
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
    else
    {
      isi = false;
      digitalWrite(PompaOut, OFF);
      digitalWrite(PompaIn, OFF);
    }
  }

  /* -------------------------------- SUHU DINGIN && AIR CUKUP JERNIH ---------------------------------- */
  else  if ( (suhu >= 0 && suhu <= 25) && (ntu >= 400 && ntu <= 650) )
  {
    /*  TAMPIL SERIAL MONITOR */
    Serial.print("DINGIN");
    Serial.print("  |  CUKUP JERNIH");
    /*  HEATER ON */
    digitalWrite(Heater, ON);
    /*  KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PENGISIAN AIR Jika Air Kurang 15 CM ---------------------------------- */
    if (tinggiAir <= 15)
    {
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          delay(500);
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
    else
    {
      isi = false;
      digitalWrite(PompaOut, OFF);
      digitalWrite(PompaIn, OFF);
    }
  }


  /* -------------------------------- SUHU DINGIN && AIR KERUH ---------------------------------- */
  else if ( (suhu >= 0 && suhu <= 25) && (ntu >= 600 && ntu <= 1024) )
  {
    /*  TAMPIL SERIAL MONITOR */
    Serial.print("DINGIN");
    Serial.print("  |  AIR KERUH");
    /*  HEATER ON */
    digitalWrite(Heater, ON);
    /*  KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PEMBUANGAN AIR Jika Tinggi Air Mencapai Antara 11 - 18 CM ---------------------------------- */
    if (tinggiAir > 10 && tinggiAir <= 18)
    {
      buang = true;
      while (buang) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PEMBUANGAN ke NODEMCU */
          String Pesan = "10";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaOut, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PEMBUANGAN BERHENTI Jika Tinggi Air Mencapai 6 - 8 CM ---------------------------------- */
        if (tinggiAir >= 6 && tinggiAir <= 8)
        {
          delay(500);
          digitalWrite(PompaOut, OFF);
          buang  = false;
          break;
        }
      }
    }

    /* -------------------------------- PENGISIAN AIR Jika Tinggi Air Mencapai 6 - 8 CM ---------------------------------- */
    if (tinggiAir >= 6 && tinggiAir <= 8)
    {
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          delay(500);
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
  }


  /* -------------------------------- FUZZY LOGIC ---------------------------------- */
  /* ------------------------------------------------------------------ SUHU NORMAL */
  
  /* -------------------------------- SUHU NORMAL && AIR JERNIH ---------------------------------- */
  else if ( (suhu >= 24 && suhu <= 29) && (ntu >= 0 && ntu <= 450) )
  {
    /* TAMPIL SERIAL MONITOR */
    Serial.print("NORMAL");
    Serial.print("  |  AIR JERNIH");
    /* HEATER OFF && POMPA OUT OFF */
    digitalWrite(Heater, OFF);
    digitalWrite(PompaOut, OFF);
    /* KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PENGISIAN Air Jika Tinggi Air Kurang 15 CM ---------------------------------- */
    if (tinggiAir <= 15)
    {
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          delay(500);
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
    else
    {
      isi = false;
      digitalWrite(PompaOut, OFF);
      digitalWrite(PompaIn, OFF);
    }
  }

  /* -------------------------------- SUHU NORMAL && AIR CUKUP JERNIH ---------------------------------- */
  else if ( (suhu >= 24 && suhu <= 29) && (ntu >= 400 && ntu <= 650) )
  {
    /* TAMPIL SERIAL MONITOR */
    Serial.print("NORMAL");
    Serial.print("  |  CUKUP JERNIH");
    /* HEATER OFF && POMPA OUT OFF */
    digitalWrite(Heater, OFF);
    digitalWrite(PompaOut, OFF);
    /* KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PENGISIAN AIR Jika Tinggi Air Kurang 15 CM ---------------------------------- */
    if (tinggiAir <= 15)
    {
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          delay(500);
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
    else
    {
      isi = false;
      digitalWrite(PompaOut, OFF);
      digitalWrite(PompaIn, OFF);
    }
  }

  /* -------------------------------- SUHU NORMAL && AIR KERUH ---------------------------------- */
  else if ( (suhu >= 24 && suhu <= 29) && (ntu >= 600 && ntu <= 1024) )
  {
    /* TAMPIL SERIAL MONITOR */
    Serial.print("NORMAL");
    Serial.print("  |  AIR KERUH");
    /* HEATER OFF */
    digitalWrite(Heater, OFF);
    /* KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PEMBUANGAN AIR Jika Tinggi Air Antara 11 - 18 CM ---------------------------------- */
    if (tinggiAir > 10 && tinggiAir <= 18)
    {
      buang = true;
      while (buang) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PEMBUANGAN ke NODEMCU */
          String Pesan = "10";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaOut, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PEMBUANGAN BERHENTI Jika Tinggi Air Antara 6 - 8 CM ---------------------------------- */
        if (tinggiAir >= 6 && tinggiAir <= 8)
        {
          delay(500);
          digitalWrite(PompaOut, OFF);
          buang  = false;
          break;
        }
      }
    }

    /* -------------------------------- PENGISIAN AIR Jika Tinggi Air Antara 6 - 8 CM ---------------------------------- */
    if (tinggiAir >= 6 && tinggiAir <= 8)
    {
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          delay(500);
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
  }


  /* -------------------------------- FUZZY LOGIC ---------------------------------- */
  /* ------------------------------------------------------------------ SUHU PANAS */

  /* -------------------------------- SUHU PANAS && AIR JERNIH ---------------------------------- */
  else if ( (suhu >= 28 && suhu <= 50) && (ntu >= 0 && ntu <= 450) )
  {
    /* TAMPIL SERIAL MONITOR */
    Serial.print("PANAS");
    Serial.print("  |  AIR JERNIH");
    /* HEATER OFF */
    digitalWrite(Heater, OFF);
    /* KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PEMBUANGAN AIR Jika Tinggi Air Mencapai Anatara 11 - 18 CM ---------------------------------- */
    if (tinggiAir > 10 && tinggiAir <= 18)
    {
      buang = true;
      while (buang) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PEMBUANGAN ke NODEMCU */
          String Pesan = "10";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaOut, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PEMBUANGAN BERHENTI Jika Tinggi Air Mencapai Antara 6 - 8 CM ---------------------------------- */
        if (tinggiAir >= 6 && tinggiAir <= 8)
        {
          delay(500);
          digitalWrite(PompaOut, OFF);
          buang  = false;
          break;
        }
      }
    }

    /* -------------------------------- PENGISIAN AIR Jika Tinggi Air Mencapai Antara 6 - 8 CM ---------------------------------- */
    if (tinggiAir >= 6 && tinggiAir <= 8)
    {
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          delay(500);
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
  }

  /* -------------------------------- SUHU PANAS && AIR CUKUP JERNIH ---------------------------------- */
  else if ( (suhu >= 28 && suhu <= 50) && (ntu >= 400 && ntu <= 650) )
  {
    /* TAMPIL SERIAL MONITOR */
    Serial.print("PANAS");
    Serial.print("  |  CUKUP JERNIH");
    /* HEATER OFF */
    digitalWrite(Heater, OFF);
    /* KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PEMBUANGAN AIR Jika Tinggi Air Mencapai Antara 11 - 18 CM ---------------------------------- */
    if (tinggiAir > 10 && tinggiAir <= 18)
    {
      buang = true;
      while (buang) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PEMBUANGAN ke NODEMCU */
          String Pesan = "10";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaOut, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PEMBUANGAN BERHENTI Jika Tinggi Air Mencapai Antara 6 - 8 CM ---------------------------------- */
        if (tinggiAir >= 6 && tinggiAir <= 8)
        {
          digitalWrite(PompaOut, OFF);
          buang  = false;
          break;
        }
      }
    }

    /* -------------------------------- PENGISIAN AIR Jika Tinggi Air Mencapai Antara 6 - 8 CM ---------------------------------- */
    else if (tinggiAir >= 6 && tinggiAir <= 8)
    {
      digitalWrite(PompaOut, OFF);
      buang = false;
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
  }

  /* -------------------------------- SUHU PANAS && AIR KERUH ---------------------------------- */
  else if ( (suhu >= 28 && suhu <= 50) && (ntu >= 600 && ntu <= 1024) )
  {
    /* TAMPIL SERIAL MONITOR */
    Serial.print("PANAS");
    Serial.print("  |  AIR KERUH");
    /* HEATER OFF */
    digitalWrite(Heater, OFF);
    /* KIRIM KODE NORMAL ke NODEMCU */
    mySerial.print("#1?");

    /* -------------------------------- PEMBUANGAN AIR Jika Tinggi Air Mencapai Antara 11 - 18 CM ---------------------------------- */
    if (tinggiAir > 10 && tinggiAir <= 18)
    {
      buang = true;
      while (buang) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PEMBUANGAN ke NODEMCU */
          String Pesan = "10";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaOut, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PEMBUANGAN BERHENTI Jika Tinggi Air Mencapai Antara 6 - 8 CM ---------------------------------- */
        if (tinggiAir >= 6 && tinggiAir <= 8)
        {
          digitalWrite(PompaOut, OFF);
          buang  = false;
          break;
        }
      }
    }

    /* -------------------------------- PENGISIAN AIR Jika Tinggi Air Mencapai Antara 6 - 8 CM ---------------------------------- */
    else if (tinggiAir >= 6 && tinggiAir <= 8)
    {
      digitalWrite(PompaOut, OFF);
      buang = false;
      isi = true;
      while (isi) {

        int kirim = 0;
        while (kirim <= 0)
        {
          /* KIRIM KODE PENGISIAN ke NODEMCU */
          String Pesan = "5";
          mySerial.print("#" + Pesan + "?");
          kirim++;
        }

        digitalWrite(PompaIn, ON);
        dataDisplay();
        baca_ntu();
        baca_suhu();
        Tinggi_Air();

        /* -------------------------------- PENGISIAN BERHENTI Jika Tinggi Air Mencapai 17 CM ---------------------------------- */
        if (tinggiAir >= 17 && tinggiAir <= 18)
        {
          digitalWrite(PompaIn, OFF);
          isi = false;
          break;
        }
      }
    }
  }
}
