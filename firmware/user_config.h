#define ip_fixo true
#define print_data false
#define self_adjust true //pegar horario do pc na hora de enviar código

#define fade_time 200

char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};

const char* ssid = "h\"(x)"; //nome da rede wifi
const char* password = "T5e5L0e9C7o7M0u2N7i4C4a0C6o4E0s"; //senha da rede wifi

IPAddress ip(192, 168, 1, 25);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

//Pinout LED
int led_red = 13,
    led_gre = 12,
    led_blu = 14;

int R = 100, G = 125, B = 120; //cor padrão

//Pinout relés
int rele_pin = 2;

float tempo_max = 3600000; //1h => 3600000
