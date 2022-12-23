/*
  ExEmple PLATINE Lily pI
  Created by Bodmer 31/12/16
  #############################################################################
  ###### IMPERATIVEMENT mettre le fichier User_Setup.h dans la librairie ######
  ######       POUR SELECTIONNER LE TYPE DE CARTE UTILISEE               ######
  #############################################################################

  ///////////  ATTENTION 2 BIBLIOTHEQUE PCF8563 possibles, une se situe dans TTGO-Watch/src/drive  ////////////

  V5.0  correction sur programmation heure enclenchement entre 24h et 0h
      ajout entree port série  pour mise à lheure et programme cde relais
  V6.0  ajout mesure température
  V7.0  correction sur envoi en BT et affichage jour
  V8.0  mise en place cde variation luminosité
  V9.0  ajout sensor temperature DS18B20 entree sur IO26
  V10   ajout mesure tension batterie sur pin IO35
  V11   ajout mesure pression temp avec sensor MS5803
  V12   mise en forme affichage data
  V 13  calibrage altitude avec envoi message "Alt=......"  en bluetooth ou serie
  V 14  mise en eeprom de l'altitude locale et affichage altitude
*/

String Version = "V14";
String nom_bt = "Lily_PI";

#include "config.h"
#include "Free_Fonts.h"
#include <FT6236.h>  // CapacitifTouch Control
#include <PCF8563.h> // horloge temps reel
#include <BluetoothSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include <MS5803_I2C.h>

// définitions  des objets et raccordement
#define led_gpio TWATCH_TFT_BL // la sortie  12 qui commande l eclairage ecran
#define bouton 0               // bouton marqué boot sur le CI
#define Cde_alim_con 4         // pour alimenter la sortie vers le DS18b20

// adresse a selectionner sur le circuit
// par defaut: ADDRESS_HIGH
//  ADDRESS_HIGH = 0x76
//  ADDRESS_LOW  = 0x77
MS5803 sensor(ADDRESS_HIGH);

TTGOClass *ttgo;
FT6236 ts = FT6236();
PCF8563 pcf;

#define EEPROM_SIZE 100 // definition de la taille utilisée

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error BlueTooth pas valide! utiliser "menuconfig" pour le valider
#endif
BluetoothSerial SerialBT;

char buf[10];
int x, y;
char _date[6];
char prgon_aff[6];
char prgoff_aff[6];
uint8_t wday;
int $annee, $mois, $jour, $heure, $min, $second;

String nomjour;
String message = "";
int etat_relais;
int old_relais = 2;
String Mem_relais, RL1;
String strh_rel_on, strh_rel_off;
int jour_ancien = 15;
int old_time = 100;
int minute_old;
uint8_t numjour;
const String nnjj[8] = {"Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi", "Dimanche"};
const String etat_RL1[3] = {"Arrêt", "Marche"};
int Pr_Hhrelais_on, Pr_Hhrelais_off;
boolean prg_on = false;
boolean etat_touchPRG;
boolean message_in;
float temperature, temp_max = -30, temp_min = 50;
String Temp_C1 = "xx.x";
int brightness; // luminosité ecran
double pressure_abs, pressure_relative, altitude_delta;
double pressure_baseline;
String alt1;
double base_altitude; // Altitude du point de mesure

void setup()
{
    ledcAttachPin(led_gpio, 0); // assigne la pin led  a un canal PWM
    // canal 0-15, resolution 1-16 bits, freq limits depend on resolution
    // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
    ledcSetup(0, 4000, 8); // canal PWM, 12 kHz PWM, 8-bit resolution

    Wire.begin();
    Serial.begin(115200); // mettre la vitesse dans platformio.ini : monitor_speed = 115200
    // Initialise Touche screen
    ts.begin();
    ///////  initialisation eeprom
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("Echec init. EEPROM");
    }

    Serial.println(" Horloge F1GHO Version " + Version);
    pinMode(bouton, INPUT_PULLUP);
    pinMode(Cde_alim_con, OUTPUT); // alimente la sonde température
    digitalWrite(Cde_alim_con, HIGH);
    sensor.reset();
    sensor.begin();

    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();
    ttgo->tft->setRotation(3);
    ttgo->tft->fillScreen(TFT_BLACK);
    jour_ancien = 10;

    ttgo->tft->setTextColor(TFT_CYAN);
    ttgo->tft->setFreeFont(FF10);
    ttgo->tft->drawString("Relais", 360, 283);
    ttgo->tft->fillRoundRect(180, 270, 80, 45, 12, TFT_DARKGREEN); // pour dessiner un rectangle arrondi
    ttgo->tft->fillRoundRect(270, 270, 80, 45, 12, TFT_RED);
    ttgo->tft->drawString("OFF", 290, 283);
    ttgo->tft->setTextColor(TFT_WHITE);
    ttgo->tft->drawString("ON", 203, 283);

    ttgo->tft->fillRoundRect(54, 270, 115, 45, 12, TFT_BLUE);
    ttgo->tft->setTextColor(TFT_WHITE);
    ttgo->tft->setFreeFont(FMBO9);
    ttgo->tft->drawString("Pr Relais", 62, 285);
    ttgo->tft->drawString(Version, 1, 300);

    Time nowTime = pcf.getTime(); // lit les data actuelles
    $second = nowTime.second;
    $min = nowTime.minute;
    $heure = nowTime.hour;
    $jour = nowTime.day;
    $mois = nowTime.month;
    $annee = nowTime.year;
    wday = nowTime.weekday;
    nomjour = nnjj[wday]; // donne le numero du jour de la semaine de 1 à 7 mais le PCF donne de 0 à 6
    minute_old = 70;

    //////  Bluetooth/////////////////////////////////////
    SerialBT.begin(nom_bt); // nom Bluetooth
    // Serial.println("Bluetooth démarré! peut être appairé...");
    // Serial.println("Pour mettre l'horloge à jour envoyer le message au format suivant en BlueTooth:");
    // Serial.println("jjmmaaaa hhmnss jour  ex.:H 27092022 163840 2"); // = 27 09 2022 16h38m40s Mardi
    // Serial.println("Pour programmer le relais envoyer le message au format suivant en BlueTooth:");
    // Serial.println("P 1423on 1430off"); // = depart 14h23  arret a 14h30
    // Serial.println("Pour calibrer le baromètre envoi de: alt xxxx  altitude locale en m");
    Lecture_eeprom();
    pressure_baseline = sensor.getPressure(ADC_4096) * 0.1; // met la référence de pression
    ledcWrite(0, brightness);                               // regle la luminosite ecran
}

void loop()
{
    x = 0;
    y = 0;
    ////////////////////////////////////////////////////////////
    if (ts.touched())
    {
        analyse_touchscreen();
    }

    etat_relais = digitalRead(RELAY_PIN); // lit l'etat du relais
    if (etat_relais != old_relais)
    {
        if (etat_relais == 1)
            ttgo->tft->fillCircle(460, 293, 8, TFT_GREEN);
        if (etat_relais == 0)
            ttgo->tft->fillCircle(460, 293, 8, TFT_RED);

        old_relais = etat_relais;
        //        Serial.println("Etat relais= " + etat_RL1[etat_relais]);
        //        SerialBT.println("Etat relais= " + etat_RL1[etat_relais]);
        Ecriture_eeprom();
    }

    ////////////////////////////////////////////////////////////////////
    Time nowTime = pcf.getTime(); // lit les data actuelles
    $second = nowTime.second;
    $min = nowTime.minute;
    $heure = nowTime.hour;
    $jour = nowTime.day;
    $mois = nowTime.month;
    $annee = nowTime.year;
    wday = nowTime.weekday; // donne le numero du jour de la semaine de 0 à 6
    nomjour = nnjj[wday];   // donne le nom du jour

    if ($min != minute_old)
    {
        ttgo->tft->fillRect(5, 15, 200, 40, TFT_BLACK); // pour effacer l'ancienne inscription
        ttgo->tft->setTextColor(TFT_WHITE);
        ttgo->tft->setFreeFont(FSI24);
        ttgo->tft->drawString(nomjour, 8, 15);

        mesure_sensor();

        //////// mesure tension Bat.  ///////////////////////
        adc1_config_width(ADC_WIDTH_BIT_12);
        adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_6);
        int val = adc1_get_raw(ADC1_CHANNEL_7);
        float volt_bat = val * 0.001051;
        ttgo->tft->setTextColor(TFT_PINK);
        ttgo->tft->setFreeFont(FF14);
        ttgo->tft->fillRect(5, 245, 150, 25, TFT_BLACK); // pour effacer l'ancienne inscription
        ttgo->tft->drawString("Vbat= " + String(volt_bat, 2), 2, 245);

        minute_old = $min;
    }

    if ($second != old_time)
    {
        old_time = $second;
        if (wday != jour_ancien)
        {
            ttgo->tft->setTextColor(TFT_WHITE, TFT_BLACK);
            ttgo->tft->setFreeFont(FSI24);
            sprintf(_date, "%02d %02d 20%02d \n", $jour, $mois, $annee);
            ttgo->tft->drawString(_date, 240, 15);

            ttgo->tft->fillRect(5, 15, 200, 40, TFT_BLACK); // pour effacer l'ancienne inscription
            ttgo->tft->setTextColor(TFT_WHITE);
            ttgo->tft->setFreeFont(FSI24);
            ttgo->tft->drawString(nomjour, 8, 15);
            jour_ancien = wday;
        }
        ttgo->tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        sprintf(buf, "%02d:%02d:%02d \n", $heure, $min, $second);
        // snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
        ttgo->tft->drawString(buf, 45, 70, FONT8);

        if (etat_touchPRG == 1) // uniquement si valide par la touche Pr Relais
        {
            String heure_actuelle = String($heure) + String($min);
            if ($min < 10)
                heure_actuelle = String($heure) + "0" + String($min);
            int heure_int = heure_actuelle.toInt();

            if (Pr_Hhrelais_on < Pr_Hhrelais_off)
            {
                if (heure_int >= Pr_Hhrelais_on && heure_int < Pr_Hhrelais_off && prg_on == false)
                {
                    //                    Serial.println("Prg a mis relais ON");
                    ttgo->turnOnRelay();
                    ttgo->tft->fillCircle(460, 293, 8, TFT_GREEN);
                    prg_on = true;
                }
                else if (heure_int >= Pr_Hhrelais_off && prg_on == true)
                {
                    //                    Serial.println("Prg a mis relais OFF");
                    ttgo->turnOffRelay();
                    ttgo->tft->fillCircle(460, 293, 8, TFT_RED);
                    prg_on = false;
                }
            }
            else if (Pr_Hhrelais_on > Pr_Hhrelais_off) // passage entre 22h.. et 00h..
            {
                if (heure_int >= Pr_Hhrelais_on && heure_int > Pr_Hhrelais_off && prg_on == false)
                {
                    //                    Serial.println("Prg a mis relais ON");
                    ttgo->turnOnRelay();
                    ttgo->tft->fillCircle(460, 293, 8, TFT_GREEN);
                    prg_on = true;
                }

                else if (heure_int >= Pr_Hhrelais_off && heure_int < Pr_Hhrelais_on && prg_on == true)
                {
                    //                    Serial.println("Prg a mis relais OFF");
                    ttgo->turnOffRelay();
                    ttgo->tft->fillCircle(460, 293, 8, TFT_RED);
                    prg_on = false;
                }
            }
        }

        if (digitalRead(bouton) == false)
        {
            mesure_sensor();
            temp_max = temperature;
            temp_min = temperature;
        }
    }

    /////////////////////////////////////////////////////////
    // pour mise à l'heure par bluetooth envoi de date et heure au format: H 25102022 094512 2  = JJMMAAAA HHMNSS jsem
    // le lundi est le jour 1, le dimanche le jour 7
    // pour programme relais par bluetooth envoi de date et heure au format:
    // P 0945on 1350off =  heure de mise ON du relais HHMN = heure de mise OFF du relais
    if (Serial.available())
    {
        lecture_Serie();
        traitement_message();
    }

    if (SerialBT.available())
    {
        lectureBT();
        traitement_message();
    }
}

///////////////////////////////////////////////////
void analyse_touchscreen()
{
    //    Serial.println("Ecran Touche aux coordonnes :");
    // récupère les coodonnees du point
    TS_Point p = ts.getPoint();
    y = p.x;
    x = p.y;
    x = 480 - x;
    // Serial.print("X: ");
    // Serial.print(x);
    // Serial.print(" Y: ");
    // Serial.println(y);

    if (y > 270 && y < 320) // controle si dans zone des boutons
    {
        if (x > 180 && x < 260)
        {
            ttgo->turnOnRelay();
        }
        else if (x > 270 && x < 350)
        {
            ttgo->turnOffRelay();
        }
        else if (x > 45 && x < 170)
        {
            if (etat_touchPRG == 0)
            {
                etat_touchPRG = 1;
            }
            else
            {
                etat_touchPRG = 0;
            }
            delay(200);
        }
        etat_relais = digitalRead(RELAY_PIN); // lit l'etat du relais
        Ecriture_eeprom();
        Lecture_eeprom();
    }
    else if (y > 90 && y < 162)
    {
        brightness = x / 1.9;
        if (brightness < 6)
            brightness = 6;
        ledcWrite(0, brightness); // // regle la luminosite ecran
        Ecriture_eeprom();
    }

    old_time = 65; // pour afficher  les heures au retour
}

////////////////////////////////////////////////
void lecture_Serie()
{
    message = "";
    while (Serial.available())
    {
        byte inChar = Serial.read();
        if (inChar == 13)
            break;
        message += char(inChar);
    }
    Serial.println(message);
}

///////////////////////////
void lectureBT()
{
    byte carac;
    message = "";
    while (SerialBT.available() && carac != '\n')
    {
        carac = SerialBT.read();
        message = message + char(carac);
    }
    Serial.println(message);
}

/////////////////////////////////////////////////////////
void traitement_message()
{
    SerialBT.println(message);
    if (message.startsWith("H") || message.startsWith("h"))
    {
        String ann = message.substring(8, 10);
        String mmm = message.substring(4, 6);
        String jjj = message.substring(2, 4);
        String hhh = message.substring(11, 13);
        String mnn = message.substring(13, 15);
        String sss = message.substring(15, 17);
        String numjour = message.substring(18, 19);

        uint8_t annee = ann.toInt();
        uint8_t mois = mmm.toInt();
        uint8_t jour = jjj.toInt();
        uint8_t heures = hhh.toInt();
        uint8_t minutes = mnn.toInt();
        uint8_t secondes = sss.toInt();
        uint8_t jsem = numjour.toInt() - 1; // les jours sont comptes de 0 à 7 par le PCF8563

        pcf.stopClock();         // arrete l'horloge
        pcf.setYear(annee);      // met l'annee
        pcf.setMonth(mois);      // met le mois
        pcf.setDay(jour);        // met le jour
        pcf.setHour(heures);     // met l'heure
        pcf.setMinut(minutes);   // met les minutes
        pcf.setSecond(secondes); // mets les secondes
        pcf.setWeekday(jsem);    // met le jour de la semaine  (1 = lundi, 7 = dimanche)
        pcf.startClock();        // demarre l'horloge
    }
    else if (message.startsWith("P") || message.startsWith("p")) // P 0945on 1350off
    {
        strh_rel_on = message.substring(2, 6);
        strh_rel_off = message.substring(9, 13);
        Serial.println("Heure de marche: " + strh_rel_on);
        Serial.println("Heure d'arret: " + strh_rel_off);
    }
    else if (message.startsWith("Alt") || message.startsWith("alt")) // alt 500
    {
        alt1 = message.substring(4, message.length());
        //////////////////////////   MISE EN MEMEOIRE ALTITUDE LOCALE ////////////////////
        Ecriture_eeprom();
        Lecture_eeprom();
        ESP.restart();
    }

    Ecriture_eeprom();
    Lecture_eeprom();
}

//////////////////////////
void Ecriture_eeprom()
{
    //////////////////////////   MISE EN MEMEOIRE ETAT RELAIS  ///////////////////////////////
    Mem_relais = "10" + String(etat_relais);
    for (int i = 0; i < 3; i++) // string suivant sa longueur
    {
        EEPROM.write(10 + i, Mem_relais[i]); // met 1 à 1 les data caracteres
    }

    String Mem_touch = String(etat_touchPRG);
    //////////////////////////   MISE EN MEMEOIRE INFOS PILOTAGE RELAIS  ////////////////////
    for (int i = 0; i < 1; i++) // string suivant sa longueur
    {
        EEPROM.write(2 + i, Mem_touch[i]); // met 1 à 1 les data caracteres
    }

    //////////////////////////   MISE EN MEMEOIRE INFOS LUMINOSITE  ////////////////////
    String led_lum = String(brightness);
    if (brightness < 100)
        led_lum = "0" + led_lum;
    for (int i = 0; i < 3; i++) // string suivant sa longueur
    {
        EEPROM.write(15 + i, led_lum[i]); // met 1 à 1 les data caracteres
    }

    //////////////////////////   MISE EN MEMEOIRE ALTITUDE LOCALE ////////////////////
    for (int i = 0; i < alt1.length(); i++) // string suivant sa longueur
    {
        EEPROM.write(45 + i, alt1[i]); // met 1 à 1 les data caracteres
    }

    //////////////////////////   MISE EN MEMOIRE HEURES PILOTAGE RELAIS  ////////////////////
    for (int i = 0; i < strh_rel_on.length(); i++)
    {
        EEPROM.write(20 + i, strh_rel_on[i]);
    }

    for (int i = 0; i < strh_rel_off.length(); i++)
    {
        EEPROM.write(30 + i, strh_rel_off[i]);
    }
    EEPROM.commit(); // Ecrit dans l' EEPROM

    Pr_Hhrelais_on = strh_rel_on.toInt();
    Pr_Hhrelais_off = strh_rel_off.toInt();
}

////////////////////////////////////////
void Lecture_eeprom()
{
    Mem_relais = "";
    for (int i = 0; i < 3; i++)
    {
        Mem_relais = Mem_relais + (char(EEPROM.read(10 + i)));
    }
    RL1 = Mem_relais.substring(0, 3); // lit 3 caracteres de 0 à 2
                                      //    Serial.println("EEPROM = " + RL1);
    ///// remise dans l'état avant coupure d'alim /// valeur 100 = OFF  101 = ON
    if (RL1 == "101")
    {
        ttgo->turnOnRelay();
        ttgo->tft->fillCircle(460, 293, 8, TFT_GREEN);
    }
    else if (RL1 == "100")
    {
        ttgo->turnOffRelay();
        ttgo->tft->fillCircle(460, 293, 8, TFT_RED);
    }

    //////////////  NIVEAU LUMINOSITE  ////////////////////////
    String led_lum = "";
    for (int i = 0; i < 3; i++)
    {
        //        led_lum = String(brightness);
        led_lum = led_lum + (char(EEPROM.read(15 + i)));
    }
    brightness = led_lum.toInt();

    //////////////  ALTITUDE LOCALE  ////////////////////////
    alt1 = "";
    for (int i = 0; i < 4; i++)
    {
        alt1 = alt1 + (char(EEPROM.read(45 + i)));
    }
    Serial.println("alt1=:" + alt1);
    base_altitude = alt1.toDouble();
    Serial.println();
    Serial.println(String(base_altitude) + " m");

    ///////////////////////lecture etat touche prog  /////////////////////
    String lec_mem_touch = "";
    for (int i = 0; i < 1; i++)
    {
        lec_mem_touch = lec_mem_touch + (char(EEPROM.read(2 + i)));
    }
    etat_touchPRG = lec_mem_touch.toInt();

    ////////////////////   lecture des heures programmees ////////////////////
    strh_rel_off = "";
    strh_rel_on = "";
    for (int i = 0; i < 4; i++)
    {
        strh_rel_on = strh_rel_on + (char(EEPROM.read(20 + i)));
    }
    for (int i = 0; i < 4; i++)
    {
        strh_rel_off = strh_rel_off + (char(EEPROM.read(30 + i)));
    }

    Pr_Hhrelais_on = strh_rel_on.toInt();
    Pr_Hhrelais_off = strh_rel_off.toInt();

    sprintf(prgon_aff, "P  ON: %s \n", strh_rel_on.substring(0, 2));
    sprintf(prgoff_aff, "P OFF: %s \n", strh_rel_off.substring(0, 2));

    ttgo->tft->fillRect(180, 235, 180, 30, TFT_BLACK); // pour effacer l'ancienne inscription
    if (etat_touchPRG == 1)
    {
        ttgo->tft->drawRect(180, 235, 80, 30, TFT_GREENYELLOW);
        ttgo->tft->drawRect(270, 235, 80, 30, TFT_RED);
        ttgo->tft->setTextColor(TFT_WHITE);
        ttgo->tft->setFreeFont(FM12);
        ttgo->tft->drawString((strh_rel_off.substring(0, 2) + "h" + strh_rel_off.substring(2, 4)), 275, 240);
        ttgo->tft->drawString((strh_rel_on.substring(0, 2) + "h" + strh_rel_on.substring(2, 4)), 185, 240);
    }
}

/////////////  affichage température  et pression ////////////////////
void affi_temp_press()
{
    Temp_C1 = String(temperature, 1);
    if (temperature < temp_min)
        temp_min = temperature;
    if (temperature > temp_max)
        temp_max = temperature;
    int xt = 327;
    ttgo->tft->setTextColor(TFT_SILVER);
    if (temperature < 0)
    {
        ttgo->tft->setTextColor(TFT_GREEN);
        xt = 305;
    }
    // affichage de la temperature ambiante
    ttgo->tft->setFreeFont(FF16);
    ttgo->tft->fillRect(0, 165, 479, 60, TFT_BLACK); // pour effacer l'ancienne inscription
    ttgo->tft->drawString(Temp_C1, xt, 165);
    ttgo->tft->setFreeFont(FF46);
    ttgo->tft->drawString("o", 450, 160);

    // affichage de la pression et variatio Alti
    ttgo->tft->setFreeFont(FF16);
    ttgo->tft->setTextColor(TFT_SILVER);
    ttgo->tft->drawString(String(pressure_relative, 1), 2, 165);
    ttgo->tft->drawString("hPa", 182, 165);

    ttgo->tft->fillRect(0, 210, 360, 22, TFT_BLACK); // pour effacer l'ancienne inscription
    ttgo->tft->setFreeFont(FF14);
    ttgo->tft->drawString("Dif:" + String(altitude_delta, 2) + "m", 1, 210);
    ttgo->tft->drawString("Alt:" + String((altitude_delta + base_altitude), 2) + "m", 180, 210);

    // affichage du mini et maxi
    ttgo->tft->setTextColor(TFT_CYAN);
    ttgo->tft->fillRect(360, 220, 119, 50, TFT_BLACK); // pour effacer l'ancienne inscription
    ttgo->tft->setFreeFont(FF18);
    ttgo->tft->drawString("Max " + String(temp_max, 1), 361, 225);
    ttgo->tft->drawString("Min " + String(temp_min, 1), 361, 250);
}
/////////////////////////////

void mesure_sensor()
{
    temperature = sensor.getTemperature(CELSIUS, ADC_512);
    pressure_abs = sensor.getPressure(ADC_4096) * 0.1;
    // Convertit la pression absolue avec l'aide de l'altitude en pression relative
    pressure_relative = niv_mer(pressure_abs, base_altitude);

    // calcule le changement équivalent d'altitude en fonction du cgt de pression
    altitude_delta = altitude(pressure_abs, pressure_baseline);

    affi_temp_press();
    // imprime les valeurs
    // Serial.print("Temperature = ");
    // Serial.println(temperature);

    // Serial.print("Pressure absolue (hPa)= ");
    // Serial.println(pressure_abs);

    // Serial.print("Pressure relative (hPa)= ");
    // Serial.println(pressure_relative);

    // Serial.print("Altitude a change de : ");
    // Serial.print(altitude_delta);
    // Serial.println("m\n");
}

double niv_mer(double P, double A)
// calcule la pression en hPa pour une altitude donnée en m
// renvoie la pression équivalente en  hPa au niveau de la mer
{
    return (P / pow(1 - (A / 44330.0), 5.255));
}

double altitude(double P, double P0)
// renvoie l'altitude pour la pression mesurée en fonction de la pression niveau mer
{
    return (44330.0 * (1 - pow(P / P0, 1 / 5.255)));
}
