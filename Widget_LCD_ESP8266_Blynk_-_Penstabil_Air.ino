/* ------------------------------------------------------------------ HEADER FILE ATAU LIBRARY */
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include<SoftwareSerial.h>

/* ------------------------------------------------------------------ KOMUNIKASI SERIAL RX TX di PIN D1, D2 */
SoftwareSerial mySerial(D1, D2);

/* ------------------------------------------------------------------ WIDGET LCD BLYNK MENGGUNAKAN VIRTUAL V0 */
#define BLYNK_PRINT mySerial
WidgetLCD lcd(V0);
BlynkTimer timer;

/* ------------------------------------------------------------------ KONEKSI WIFI DAN AUTH */
char auth[] = "eIkziWIMbDlyxZiPP86yK2K2SjuG8G3L";
char ssid[] = "AndroidAP";
char pass[] = "frendix123";

/* ------------------------------------------------------------------ VARIABLE DATA SUHU DAN NTU */
float TempC,
      NTU;

/* ------------------------------------------------------------------ VARIABLE DATA TINGGI AIR DAN PESAN */
int Tinggi,
    PESAN;

/* ------------------------------------------------------------------ VARIABLE PEMPROSESAN DATA YANG DITERIMA DARI ARDUINO */
String data;
char CharData;
String StringData, dataSubs;
int index1, index2;

/* ------------------------------------------------------------------ PROSES MENYAMBUNG WIFI */
void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to AP");
}

/* ------------------------------------------------------------------ KIRIM DATA SENSOR KE VIRTUAL BLYNK */
void sendSensor()
{
  Blynk.virtualWrite(V1, TempC);
  Blynk.virtualWrite(V2, NTU);
  Blynk.virtualWrite(V3, Tinggi);
}

/* ------------------------------------------------------------------ VOID SETUP() */
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  InitWiFi();
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  timer.setInterval(1000L, sendSensor);
}

/* ------------------------------------------------------------------ VOID LOOP() */
void loop() {

  /* BACA DATA DARI ARDUINO */
  while (mySerial.available() > 0)
  {
    delay(10);
    CharData = mySerial.read();
    StringData += CharData;

    /* PARSING DATA SUHU */
    if (StringData.length() > 0 && CharData == '!')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('!', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data1 = atof(buf);

      /* DATA SUHU */
      TempC = Data1;
      Serial.print("Data Suhu : ");
      Serial.println(TempC);
    }

    /*PARSING DATA NTU */
    else if (StringData.length() > 0 && CharData == '$')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('$', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data2 = atof(buf);

      /* DATA NTU */
      NTU = Data2;
      Serial.print("Data NTU : ");
      Serial.println(NTU);
    }

    /*PARSING DATA TINGGI AIR */
    else if (StringData.length() > 0 && CharData == '@')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('@', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data3 = atof(buf);

      /* DATA TINGGI AIR */
      Tinggi = Data3;
      Serial.print("Tinggi Air : ");
      Serial.println(Tinggi);
    }

    /*PARSING DATA PESAN */
    else if (StringData.length() > 0 && CharData == '?')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('?', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data4 = atof(buf);

      /* DATA PESAN */
      PESAN = Data4;
      Serial.print("PESAN : ");
      Serial.println(PESAN);
    }
  }

  /* JIKA PESAN == 5, Maka Proses Pengisian Air */
  if (PESAN == 5) lcd.print(0, 0, " PENGISIAN AIR  ");
  /* JIKA PESAN == 10, Maka Proses Pergantian Air */
  else if (PESAN == 10) lcd.print(0, 0, " PERGANTIAN AIR  ");
  /* JIKA PESAN == 1, Maka Air Dalam Kondisi Normal */
  else if (PESAN == 1) lcd.print(0, 0, "     STABIL     ");

  /* JIKA Suhu 0 - 25, Maka Air Dalam Kondisi JERNIH */
  if (TempC >= 0 && TempC <= 25) lcd.print(0, 1, "DINGIN  ");
  /* JIKA Suhu 24 - 30, Maka Air Dalam Kondisi SEDANG */
  else if (TempC >= 24 && TempC <= 29)  lcd.print(0, 1, "NORMAL  ");
  /* JIKA Suhu 29 - 50, Maka Air Dalam Kondisi PANAS */
  else if (TempC >= 28 && TempC <= 50) lcd.print(0, 1, "PANAS   ");

  /* JIKA NTU 0 - 450, Maka Air Dalam Kondisi JERNIH */
  if (NTU >= 0 && NTU <= 450) lcd.print(10, 1, "JERNIH  ");
  /* JIKA NTU 400 - 650, Maka Air Dalam Kondisi CUKUP JERNIH */
  else if (NTU >= 400 && NTU <= 650)  lcd.print(10, 1, "CUKUP  ");
  /* JIKA NTU 600 - 1024, Maka Air Dalam Kondisi KERUH */
  else if (NTU >= 600 && NTU <= 1024) lcd.print(10, 1, "KERUH  ");



  /* JALANKAN BLYNK */
  Blynk.run();
  /* JALANKAN TIMER BLYNK */
  timer.run();
}
