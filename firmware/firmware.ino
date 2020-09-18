//Bibliotecas
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include "RTClib.h"
/***end***/

//RTC
RTC_DS3231 rtc;

DateTime now;
/***end***/

int set = 4, set_old; //variaveis para seleção de funções

bool nightMode = false;
bool good_morning = false;
float tempo_salve, tempo_left;


//Wifi config
WiFiServer server(80);
String header;
/***end***/

#include "user_config.h"

void setup()
{
  Serial.begin(115200); //baud rate padrão na comunicação serial é 115200

  Wire.begin(); //começar conexão I2C
  rtc.begin(); //começar comunicação com RTC
  //RTC config
  if(rtc.lostPower())
  {
    Serial.println("DS3231 OK!");
    #if (self_adjust)
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    //January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    #endif
  }
  /***end***/

  #if (ip_fixo)
  WiFi.config(ip, gateway, subnet);    //por algum motivo isso não esta funcionando
  #endif

  //conectar na rede Wifi
  WiFi.begin(ssid, password);

  pinMode(2, OUTPUT);
  int resetar = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(2, LOW);
    delay(200);
    Serial.print(".");
    digitalWrite(2, HIGH);

    resetar = millis();
    if (resetar > 500000)
      ESP.restart();
  }

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  /***end***/

  //Pinout config
  pinMode(led_red, OUTPUT);
  pinMode(led_gre, OUTPUT);
  pinMode(led_blu, OUTPUT);
  pinMode(rele1_pin, OUTPUT);
  pinMode(rele2_pin, OUTPUT);

  digitalWrite(led_red, LOW);
  digitalWrite(led_gre, LOW);
  digitalWrite(led_blu, LOW);
  digitalWrite(rele1_pin, LOW);
  digitalWrite(rele2_pin, LOW);
  /***end***/


  // OTA config. passar código para esp pelo wifi
  ArduinoOTA.setHostname("ESP_quarto09");
  ArduinoOTA.onStart([]() {
    Serial.println("Inicio...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("nFim!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Autenticacao Falhou");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Falha no Inicio");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Falha na Conexao");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Falha na Recepcao");
    else if (error == OTA_END_ERROR) Serial.println("Falha no Fim");
  });
  ArduinoOTA.begin();
  /***end***/
}

void loop()
{
  ArduinoOTA.handle(); //verificar porta OTA
  coletar_hora(); //atualizar variaveis de horas

  wifi(); //ver se tem alguem no wifi

  if (set == 0)
    shotdowm_led();
  else if (set == 1)
    combination();
  else if (set == 2)
    fade(fade_time);
  else if (set == 3)
    vermelhao();
  else if (set == 4)
    verdao();
  else if (set == 5)
    azulao();
  else if (set == 6)
    vermelhao_claro();
  else if (set == 7)
    azul_agua();
  else if (set == 8)
    verde_agua();
  else if (set == 9)
    lilas();
  else if (set == 10)
    branco();
  else if (set == 11)
    azul_escuro();
  else if (set == 12)
    verde_escuro();
  else if (set == 13)
    amarelo();

  if (nightMode)
    shunt_down_time();

  if (good_morning)
    good_morning_funcion();

  //Serial.print ("valor desse trem aqui: ");
  //Serial.println(nightMode);
  //delay(50);
}


void coletar_hora ()
{
  now = rtc.now();

  #if (print_data)
  Serial.print("Data: "); //IMPRIME O TEXTO NO MONITOR SERIAL
  Serial.print(now.day(), DEC); //IMPRIME NO MONITOR SERIAL O DIA
  Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  Serial.print(now.month(), DEC); //IMPRIME NO MONITOR SERIAL O MÊS
  Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  Serial.print(now.year(), DEC); //IMPRIME NO MONITOR SERIAL O ANO
  Serial.print(" / Dia: "); //IMPRIME O TEXTO NA SERIAL
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]); //IMPRIME NO MONITOR SERIAL O DIA
  Serial.print(" / Horas: "); //IMPRIME O TEXTO NA SERIAL
  Serial.print(now.hour()); //IMPRIME NO MONITOR SERIAL A HORA
  Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  Serial.print(now.minute(), DEC); //IMPRIME NO MONITOR SERIAL OS MINUTOS
  Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  Serial.print(now.second(), DEC); //IMPRIME NO MONITOR SERIAL OS SEGUNDOS
  Serial.println(); //QUEBRA DE LINHA NA SERIAL
  #endif
}

void wifi ()
{
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println("New Client.");
    String currentLine = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println(all_html()); //envia o html para o cliente

            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }
      }
    }

    if (header.indexOf("GET /button0") >= 0)
    {
      set_old = set;
      set = 0;
      nightMode = false;

      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button1") >= 0)
    {
      set_old = set;
      set = 1;
      nightMode = false;

      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button2") >= 0)
    {
      set_old = set;
      set = 2;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button3") >= 0)
    {
      set_old = set;
      set = 3;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button4") >= 0)
    {
      set_old = set;
      set = 4;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button5") >= 0)
    {
      set_old = set;
      set = 5;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button6") >= 0)
    {
      set_old = set;
      set = 6;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button7") >= 0)
    {
      set_old = set;
      set = 7;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button8") >= 0)
    {
      set_old = set;
      set = 8;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /button9") >= 0)
    {
      set_old = set;
      set = 9;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /buttonN10") >= 0)
    {
      set_old = set;
      set = 10;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /buttonN11") >= 0)
    {
      set_old = set;
      set = 11;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /buttonN12") >= 0)
    {
      set_old = set;
      set = 12;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /buttonN13") >= 0)
    {
      set_old = set;
      set = 13;
      nightMode = false;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /buttonN14") >= 0)
    {
      set_old = set;
      set = 14;
      nightMode = !nightMode;
      tempo_salve = millis();
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");

      //Serial.print ("Entrou aquiiiiiiiiiii valor desse trem aqui: ");
      //Serial.println(nightMode);
    }
    else if (header.indexOf("GET /buttonN15") >= 0)
    {
      good_morning = !good_morning;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }
    else if (header.indexOf("GET /buttonN16") >= 0)
    {

    }
    else if (header.indexOf("GET /buttonN17") >= 0)
    {

    }
    else if (header.indexOf("GET /RGB(") >= 0)
    {
      int R_index = header.indexOf(",");
      int G_index = header.indexOf(",", R_index + 1);
      int B_index = header.indexOf(")", G_index);

      String R_aux = header.substring(header.indexOf("(") + 1, R_index);
      String G_aux = header.substring(R_index + 1, G_index);
      String B_aux = header.substring(G_index + 1, B_index);

      R = R_aux.toInt();
      G = G_aux.toInt();
      B = B_aux.toInt();

      Serial.print("index: ");
      Serial.print(header.indexOf("GET /RGB(")); Serial.print(";");
      Serial.print(R_index); Serial.print(";");
      Serial.print(G_index); Serial.print(";");
      Serial.println(B_index);

      Serial.print("aux: ");
      Serial.print(R_aux); Serial.print(";");
      Serial.print(G_aux); Serial.print(";");
      Serial.println(B_aux);

      Serial.print("value: ");
      Serial.print(R); Serial.print(";");
      Serial.print(G); Serial.print(";");
      Serial.println(B);

      cores_RGB(R, G, B);

      set_old = set;
      set = 99;
      client.println("<script>location.href = \"http://192.168.1.25\";</script>");
    }

    header = "";
    client.stop();
  }
}

String all_html ()
{
  String _html = "";
  //_html += "HTTP/1.1 200 OK";
  //_html += "Content-type:text/html";
  //_html += "Connection: close";
  // Display the HTML web page

  _html += "<!DOCTYPE html><html>";
  _html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  _html += "<meta http-equiv=\"refresh\" content=\"120; url=http://192.168.1.25\" >";
  _html += "<link rel=\"icon\" href=\"data:,\">";

  _html += "<style>";

  _html += ".btn-group button {";
  _html += "background-color: #4CAF50;";
  _html += "border: 1px solid black; ";
  _html += "color: black;";
  _html += "padding: 10px 24px;";
  _html += "cursor: pointer;";
  _html += "float: left;";
  _html += "}";

  _html += ".btn-group2 button {";
  _html += "background-color: RGB(";
  _html += R; _html += ","; _html += G; _html += ","; _html += B;
  _html += ");";
  _html += "border: 1px solid green; ";
  _html += "color: white;";
  _html += "padding: 10px 24px;";
  _html += "cursor: pointer;";
  _html += "}";

  _html += ".btn-group:after {";
  _html += "content: "";";
  _html += "clear: both;";
  _html += "display: table;";
  _html += "}";

  _html += ".btn-group button:not(:last-child) {";
  _html += "border-right: none;";
  _html += "}";

  _html += ".btn-group button:hover {";
  _html += "background-color: #3e8e41;";
  _html += "}";

  _html += ".slidecontainer {";
  _html += "width: 100%;";
  _html += "}";

  _html += ".slider {";
  _html += "-webkit-appearance: none;";
  _html += "width: 100%;";
  _html += "height: 25px;";
  _html += "background: #d3d3d3;";
  _html += "outline: none;";
  _html += "opacity: 0.7;";
  _html += "-webkit-transition: .2s;";
  _html += "transition: opacity .2s;";
  _html += "}";

  _html += ".slider:hover {";
  _html += "opacity: 1;";
  _html += "}";

  _html += ".slider::-webkit-slider-thumb {";
  _html += "-webkit-appearance: none;";
  _html += "appearance: none;";
  _html += "width: 25px;";
  _html += "height: 25px;";
  _html += "background: #4CAF50;";
  _html += "cursor: pointer;";
  _html += "}";

  _html += ".slider::-moz-range-thumb {";
  _html += "width: 25px;";
  _html += "height: 25px;";
  _html += "cursor: pointer;";
  _html += "background: #00ff00;";
  _html += "}";

  _html += "#myRed::-moz-range-thumb {";
  _html += "background: #ff0000;";
  _html += "}";
  _html += "#myGre::-moz-range-thumb {";
  _html += "background: #00ff00;";
  _html += "}";
  _html += "#myBlu::-moz-range-thumb {";
  _html += "background: #0000ff;";
  _html += "}";

  _html += "</style>";

  _html += "</head>";

  // Web Page Heading
  _html += "<body>";

  _html += "<a href=\"http://192.168.1.25\" style=\"text-decoration: none;color: black;\"><h1 style=\"text-align: center\">Led bedroom</h1></a>";

  _html += "<div class=\"btn-group\" style=\"width:100%\">";

  _html += "<br>";
  _html += "<p><a href=\"/button0\"><button style=\"width:100%\" >Desligar</button></a></p>";

  _html += "</div>";

  // _html += "<br>";
  // _html += "<p>Modo atual: ";
  // if (set == 0)
  //   _html += "Desligado</p>";
  // else if ((set >= 1) && (set <= 2))
  //   _html += "Funcoes</p>";
  // else if ((set >= 3) && (set <= 5))
  //   _html += "Cores solidas</p>";
  // else if ((set >= 6) && (set <= 13))
  //   _html += "Cores</p>";


  _html += "<hr>";

  _html += "<p style=\"text-align: center\">Funcoes:</p>";
  _html += "<div class=\"btn-group\" style=\"width:100%\">";

  _html += "<p><a href=\"/button1\"><button style=\"width:33.3%";
  if (set == 1)
  {
    _html += ";background-color: #6d912a;";
  }
  _html += "\" >Piscar</button></a></p>";

  _html += "<p><a href=\"/button2\"><button style=\"width:33.3%";
  if(set == 2)
  {
    _html += ";background-color: #6d912a;";
  }
  _html += "\" >Fade</button></a></p>";

  _html += "<p><a href=\"/buttonN14\"><button style=\"width:33.3%";
  if(nightMode)
  {
    _html += ";background-color: #6d912a;";
  }
  _html += "\" >Sleep LED</button></a></p>";

  if (nightMode)
  {
    _html += "<p>shunt down time in: ";

    if (tempo_max < tempo_left)
      _html += " acabou :)";
    else
    {
      int h = 0, mi = 0, s = 0, aux = 0;

      s = (tempo_max - tempo_left) / 1000;
      mi = int(s/60);

      Serial.print("mi: "); Serial.print(mi);
      Serial.print(" ss: "); Serial.print(s);

      s = (s/60 - mi)*60;

      Serial.print(" s: "); Serial.println(s);

      _html += "time left: " + String(mi) + " min ";// + String(s) + " s";

      // Serial.print("hour: ");
      // Serial.print(h);
      // Serial.print("min: ");
      // Serial.print(mi);
      // Serial.print("seg: ");
      // Serial.println(s);
    }

    _html += "</p>";
  }
  else
    _html += "<p></p>";


  _html += "<p><a href=\"/buttonN15\"><button style=\"width:33.3%";
  if(good_morning)
  {
    _html += ";background-color: #6d912a;";
  }
  _html += "\">Morning</button></a></p>";

  _html += "<p><a href=\"/buttonN16\"><button style=\"width:33.3%\" >On/OFF rele </button></a></p>";
  _html += "<p><a href=\"/buttonN17\"><button style=\"width:33.3%\" >sleep rele</button></a></p>";

  _html += "</div>";

  _html += "<hr>";
  _html += "<p style=\"text-align: center\">Color</p>";

  _html += "<div class=\"slidecontainer\">";
  _html += "<input type=\"range\" min=\"0\" max=\"255\" value=\"";
  _html += R;
  _html += "\" class=\"slider\" id=\"myRed\">";
  _html += "<input type=\"range\" min=\"0\" max=\"255\" value=\"";
  _html += G;
  _html += "\" class=\"slider\" id=\"myGre\">";
  _html += "<input type=\"range\" min=\"0\" max=\"255\" value=\"";
  _html += B;
  _html += "\" class=\"slider\" id=\"myBlu\">";
  _html += "</div>";

  _html += "<p>RGB (<a id=\"valueRed\"></a>, <span id=\"valueGre\"></span>, <span id=\"valueBlu\"></span>)</p>";

  _html += "<div class=\"btn-group2\" style=\"width:100%\">";
  _html += "<p><a><button style=\"width:100%\" Onclick=\"enviarRGB()\">Send</button></a></p>";
  _html += "</div>";



  _html += "<hr>";

  _html += "<div class=\"btn-group\" style=\"width:100%\">";

  _html += "<p><a href=\"/button6\"><button style=\"width:25%; background-color: rgb(255, 171, 171);\">Vermelho claro</button></a></p>";
  _html += "<p><a href=\"/button7\"><button style=\"width:25%; background-color: rgb(148, 255, 255);\">Azul agua</button></a></p>";
  _html += "<p><a href=\"/button8\"><button style=\"width:25%; background-color: rgb(0, 255, 255);\">Verde agua</button></a></p>";
  _html += "<p><a href=\"/button9\"><button style=\"width:25%; background-color: rgb(193, 22, 196);color: white;\">Lilas sleep</button></a></p>";

  _html += "<p><a href=\"/buttonN10\"><button style=\"width:25%; background-color: rgb(200, 200, 200);\">Branco normal</button></a></p>";
  _html += "<p><a href=\"/buttonN11\"><button style=\"width:25%; background-color: rgb(0, 88, 138);color: white;\">Azul escuro</button></a></p>";
  _html += "<p><a href=\"/buttonN12\"><button style=\"width:25%; background-color: rgb(48, 214, 55);color: white;\">Verde escuro</button></a></p>";
  _html += "<p><a href=\"/buttonN13\"><button style=\"width:25%; background-color: rgb(249, 247, 61);\">Amarelo bebe</button></a></p>";

  _html += "</div>";

  _html += "<div class=\"btn-group\" style=\"width:100%\">";

  _html += "<p><a href=\"/button3\"><button style=\"width:33.3%; background-color: rgb(245, 30, 30);\" href=\"/button3\">Vermelho</button></a></p>";
  _html += "<p><a href=\"/button4\"><button style=\"width:33.3%; background-color: rgb(30, 245, 30);\" href=\"/button4\">Verde</button></a></p>";
  _html += "<p><a href=\"/button5\"><button style=\"width:33.3%; background-color: rgb(30, 30, 245);\" href=\"/button5\">Azul</button></a></p>";

  _html += "</div>";

  _html += "<br>";
  _html += "<br>";
  _html += "<br>";
  _html += "<br>";

  _html += "<p style=\"text-align: center\">By Wesley </p>";
  /*
    _html += "<br>";
    _html += "<br>";
    _html += "<p>for dev: <br> set: ";
    _html += set;
    _html += "; set old: ";
    _html += set_old;
    _html += "</p>";*/

  _html += "</body>";

  _html += "<script>";
  _html += "var sliderRED = document.getElementById(\"myRed\");";
  _html += "var outputRED = document.getElementById(\"valueRed\");";
  _html += "outputRED.innerHTML = sliderRED.value;";

  _html += "sliderRED.oninput = function() {";
  _html += "outputRED.innerHTML = this.value;";
  _html += "};";

  _html += "var sliderGRE = document.getElementById(\"myGre\");";
  _html += "var outputGRE = document.getElementById(\"valueGre\");";
  _html += "outputGRE.innerHTML = sliderGRE.value;";

  _html += "sliderGRE.oninput = function() {";
  _html += "outputGRE.innerHTML = this.value;";
  _html += "};";

  _html += "var sliderBLU = document.getElementById(\"myBlu\");";
  _html += "var outputBLU = document.getElementById(\"valueBlu\");";
  _html += "outputBLU.innerHTML = sliderBLU.value;";

  _html += "sliderBLU.oninput = function() {";
  _html += "outputBLU.innerHTML = this.value;";
  _html += "};";

  _html += "function enviarRGB ()";
  _html += "{";
  _html += "location.href = \"http://192.168.1.25/RGB(\" + sliderRED.value + \",\" + sliderGRE.value + \",\" + sliderBLU.value + \")\";";
  _html += "}";
  _html += "</script>";

  _html += "</html>";

  return _html;
}



void fade (int del)
{
  Serial.println("primeiro for");
  for (int i = 0; (i < 255) && (!breck_for()); i++)
  {
    analogWrite(led_red, i);
    analogWrite(led_blu, 255 - i);
    Serial.println(i);

    wifi();
    delay(del);
  }

  Serial.println("segundo for");
  for (int i = 0; (i < 255) && (!breck_for()); i++)
  {
    analogWrite(led_red, 255 - i);
    analogWrite(led_gre, i);
    Serial.println(i);

    wifi();
    delay(del );
  }

  Serial.println("terceiro for");
  for (int i = 0; (i < 255) && (!breck_for()); i++)
  {
    analogWrite(led_gre, 255 - i);
    analogWrite(led_blu, i);
    Serial.println(i);

    wifi();
    delay(del );
  }
}

bool breck_for ()
{
  Serial.print("set: ");
  Serial.print(set);
  Serial.print("  set_old: ");
  Serial.println(set_old);

  if (set != set_old)
  {
    set_old = set;
    return true;
  }
  else
    return false;
}

void combination ()
{
  shotdowm_led();

  digitalWrite(led_red, HIGH);
  wifi();
  delay(500);
  digitalWrite(led_gre, HIGH);
  wifi();
  delay(500);

  digitalWrite(led_red, LOW);
  wifi();
  delay(500);

  digitalWrite(led_blu, HIGH);
  wifi();
  delay(500);

  digitalWrite(led_gre, LOW);
  wifi();
  delay(500);
}

void cores_RGB (int R_aux, int G_aux, int B_aux)
{
  R_aux = map(R_aux, 0, 255, 0, 1024);
  G_aux = map(G_aux, 0, 255, 0, 1024);
  B_aux = map(B_aux, 0, 255, 0, 1024);

  R_aux = constrain(R_aux, 0, 1024);
  G_aux = constrain(G_aux, 0, 1024);
  B_aux = constrain(B_aux, 0, 1024);

  analogWrite(led_red, R_aux);
  analogWrite(led_gre, G_aux);
  analogWrite(led_blu, B_aux);
}

void vermelhao ()
{
  cores_RGB(255, 0, 0);
}

void verdao ()
{
  cores_RGB(0, 255, 0);
}

void azulao ()
{
  cores_RGB(0, 0, 255);
}

void vermelhao_claro()
{
  cores_RGB(255, 171, 171);
}
void branco()
{
  cores_RGB(255, 255, 255);
}

void azul_agua()
{
  cores_RGB(148, 255, 255);
}
void azul_escuro()
{
  cores_RGB(0, 88, 138);
}

void verde_agua ()
{
  cores_RGB(0, 255, 255);
}
void verde_escuro ()
{
  cores_RGB(48, 214, 55);
}

void lilas ()
{
  cores_RGB(193, 22, 196);
}

void amarelo ()
{
  cores_RGB(249, 247, 61);
}

void shotdowm_led()
{
  digitalWrite(led_red, LOW);
  digitalWrite(led_gre, LOW);
  digitalWrite(led_blu, LOW);
}

void shunt_down_time ()
{
  delay(100);

  tempo_left = millis() - tempo_salve;

  Serial.print("tempo_left: ");
  Serial.println(tempo_left);

  if (tempo_max < tempo_left)
  {
    shotdowm_led();

    Serial.println("acabooouuuuuuuuuuuuuuu");
  }
  else
  {
    if((now.hour() >= 18) && (now.hour() <= 23))
    {
      int _r = map(now.hour(), 18, 23, 80, 4);
      int _g = map(now.hour(), 18, 23, 60, 3);
      int _b = map(now.hour(), 18, 23, 40, 0);

      _r = constrain(_r, 4, 80);
      _g = constrain(_g, 3, 60);
      _b = constrain(_b, 0, 40);

      cores_RGB(_r, _g, _b);
    }
  }
}

void good_morning_funcion ()
{
  if(now.hour() == 5)
  {
    int _r = map(now.minute(), 0, 59, 11, 150);
    int _g = map(now.minute(), 0, 59, 5, 150);
    int _b = map(now.minute(), 0, 59, 20, 200);

    // _r = constrain(_r, 1, 80);
    // _g = constrain(_g, 3, 60);
    // _b = constrain(_b, 0, 40);

    cores_RGB(_r, _g, _b);
  }
}
