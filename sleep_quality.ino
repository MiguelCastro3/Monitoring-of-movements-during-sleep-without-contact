// Sleep Quality Monitor VSL - Monitor da qualidade do sono
// Charles - August/2016 - Version 3 - Jumbo
#include <Keypad.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>    // Sensor de pressão atmosférica e temperatura
Adafruit_BMP085 bmp; //Vcc=3.3V, UNO: SCL=A5, SDA=A4 MEGA: SCL=21, SDA=20
#include <EEPROM.h>
#include <DS1307.h>     // rtc
DS1307 rtc(A2, A3);  //RTC A2=CLK A3=SDA VCC=5V
#include <Adafruit_GFX.h>          // para LCD Nokia
#include <Adafruit_PCD8544.h>      // para LCD Nokia
#include <Keypad.h>
Adafruit_PCD8544 lcd = Adafruit_PCD8544(13, 12, 11, 10, 9); //CLK DIN DC CE RST  

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'},
};

byte rowPins[ROWS] = {A10, A11, A12, A13}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 24, 28}; //connect to the column pinouts of the keypad
char key;
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

int i, led=3,bot_luz=7,est_bot_luz,lights,cc=1,cc8,cc2,cc3,cc5,cc4,mov_cont, lastEEP, buzz=14, hora, minuto, bot_monit=8, sbot_monit;
int tempoluz,vatual,tempo_sem_mov,tempo_mov_ant, tempo_sem_mov_max, hora_tot, minuto_tot,temp_tot;
int timeout,timeout_val=150, hora_maior_mov, minuto_maior_mov, tempo_mov_atual, tempo_mov_max_h, tempo_mov_max_m;
int func=1,pres,dhora, dminuto, acerto_hora, acerto_desp, desp=0, minuto_ant, minuto_ant1, mov=A0, smov, c6, pres_max, pres_min;
int hpres_cont, hpres[13], smonit=0, tempo_sono, tempo_sono_inic, tempo_sono_final, hora_pos, cc7=80, h[20], mov_cont_ant;
int tempo_sono_parcial, hora_parcial, minuto_parcial, conf=1, desp_tipo, relat[7], relat_num, max_val,mov_cont_atual,luz=4,sluz;
unsigned int cont, umid, cont2, cont3;
float temp, temp_ant;
String sono_tipo="Sleep Ql", sp;
Time t;
const unsigned char PROGMEM af[] = {B00001, B00011, B00101, B11001, B11001, B00101, B00011, B00001}; //auto falante
const unsigned char PROGMEM gr[] = {B00100, B01010, B01010, B00100}; //grau

void setup() {
  //Serial.begin(9600);
  //rtc.halt(false);
  //rtc.setDOW(FRIDAY);      //Remover as linhas após a primeira execução
  //rtc.setTime(06, 30, 00);
  //rtc.setDate(19, 8, 2016);   //Define o dia, mes e ano
  //rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);
  bmp.begin();
  //   Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  //  while (1) {}
  // }
  digitalWrite(buzz, HIGH); delay(40); digitalWrite(buzz, LOW);
  delay(100);
  pinMode(led, OUTPUT);
  pinMode(luz, OUTPUT);
  pinMode(mov, INPUT);
  pinMode(bot_luz, INPUT);
  pinMode(bot_monit, INPUT);
  pinMode(buzz, OUTPUT);

  lastEEP = EEPROM.read(0); if (lastEEP == 255) lastEEP = 0;      //lê a última qt de medição na posição 0
  tempoluz = EEPROM.read(200); if (tempoluz == 255) tempoluz = 0; //lê o tempo de luz acesa
  desp = EEPROM.read(201); if (desp == 255) desp = 0;             //0=despertador desligado, 1=despertador ativo
  desp_tipo = EEPROM.read(202); if (desp_tipo == 255) desp_tipo = 0; //0=despertador desligado, 1=despertador ativo
  dhora = EEPROM.read(203); if (dhora == 255) dhora = 0;          //lê a hora do despertador
  dminuto = EEPROM.read(204); if (dminuto == 255) dminuto = 0;    //lê o minuto do despertador
  relat_num = lastEEP; ledados_relat();

  lcd.begin();
  lcd.setContrast(50); //Ajusta o contraste do display

  logo();     // Chama a tela de logo
  delay(200); digitalWrite(led,HIGH); delay(300);digitalWrite(led,LOW);
  
  pres = bmp.readPressure() / 100;
  temp = bmp.readTemperature();
  pres_max = pres; pres_min = pres;
  desp = 1;       //1=ativo, 0=false = mudar para eeprom
}

void loop() {
  sbot_monit = digitalRead(bot_monit);
  if (sbot_monit == HIGH) {
    func = 20;
    //disp_monit();
  }

  if (acerto_hora <= 0) gettime();   //a hora não está sendo alterada. Atualiza a hora
  if (desp == 1) checa_desp();       //despertador ativo. Está na hora ?
  letemp();
  if (smonit == 1) monit();

  key = keypad.getKey();
  if (key) {
    timeout = 0; timeout_val = 200;      // houve atividade, zera o timeout
    if (key == '1') {func = 1;}
    if (key == '2') {func = 2;}
    if (key == '3') {func = 3;}
    if (key == '4') {func = 4;}
    if (key == '5') {func = 5;}
    if (key == '6') {func = 6;}
    if (key == '7') {func = 7;}
    if (key == '8') {func = 8;}
    if (key == '9') {func = 9;}
    if (key == '0') {func = 10;}
  }
  if (func == 1) {disp_hora();relat_num = lastEEP;max_val = 0;acerto_desp = 0;}
  if (func == 2) {disp_desp();acerto_hora = 0;}
  if (func == 3) {disp_prev1();acerto_desp = 0;acerto_hora = 0;}
  if (func == 4) {disp_tempo_sono();acerto_desp = 0;acerto_hora = 0;}
  if (func == 5) {disp_qt_mov();acerto_desp = 0;acerto_hora = 0;}
  if (func == 6) {disp_temp_inerte();acerto_desp = 0;acerto_hora = 0;}
  if (func == 7) {relat_sono();acerto_desp = 0;acerto_hora = 0;}
  if (func == 8) {graf_sono();acerto_desp = 0;acerto_hora = 0;}
  if (func == 9) {disp_config();acerto_desp = 0;acerto_hora = 0;}
  if (func == 10) {logo();acerto_desp = 0;acerto_hora = 0;}
  if (func == 20) {disp_monit();acerto_desp = 0;acerto_hora = 0;}

  timeout = timeout + 1;
  if (timeout >= timeout_val) {     //tempo de inatividade, volta para tela principal
    if (smonit == 0) func = 1;
    if (smonit == 1) func = 20;
    timeout = 0;
  }

  //Botão liga ou desliga o led, após cerca de 10 minutos começa o fade out do led
  est_bot_luz = digitalRead(bot_luz);
  if (est_bot_luz == HIGH) {
    ligaluz(3);
  }
  else {
    cont3 = 0; //  Desativa contador do SQ
  }
  if (tempoluz > 0 && lights >= 1) {
    esmaece(); //Se tempoluz=0 não apaga, caso contrário, começa a esmaecer
  }

  if (acerto_hora == 0 && key == '*' && func != 7 && func != 9) ligaluz(0);

}

void disp_config(){
  timeout_val=2000;             //aumenta o timeout nessa tela
  lcd.clearDisplay();lcd.setTextColor(WHITE);lcd.fillRect(0,0,84,10,BLACK);
  lcd.setTextSize(1);lcd.setCursor(3,1); lcd.print("Configuracoes");lcd.setTextColor(BLACK);
  
  if (key == '*'){conf = conf + 1;cc2=0;cc3=0;}
  lcd.drawRect(0,10*conf,84,10,BLACK);

  lcd.setCursor(0,11);lcd.print("Tempo luz:");if (tempoluz == 0) {lcd.print("--");} else {lcd.print(tempoluz);}
  lcd.setCursor(0,21);lcd.print("Qtd.beeps:");lcd.print(desp_tipo);
  if (cc2 == 0){lcd.setCursor(0,31);lcd.print("DEL ult monit");}
  if (cc2 == 1){lcd.setCursor(0,31);lcd.print(" # Confirma  ");}
  if (cc2 == 2){
    for (i=20;i<=179;i=i+20){EEPROM.write(i+lastEEP,0);} 
    lastEEP = lastEEP -1; EEPROM.write(0,lastEEP);
    lcd.setCursor(0,31); lcd.print("Monit. APAGADA");lcd.display();delay(1500);cc2 = 0;}
  if (cc3 == 0){lcd.setCursor(0,41);lcd.print("Apagar memoria");}
  if (cc3 == 1){lcd.setCursor(0,41);lcd.print(" # Confirma  ");}
  if (cc3 == 2){
    lastEEP = 0; for (i=0;i<=255;i++){EEPROM.write(i,0);}
    lcd.setCursor(0,41); lcd.print("Memor APAGADA");lcd.display();delay(1500);cc3 = 0;}

  if (conf == 1 && key == '#'){tempoluz = tempoluz + 5; if (tempoluz >= 32) {tempoluz=0;} EEPROM.write(200,tempoluz);}
  if (conf == 2 && key == '#'){desp_tipo = desp_tipo + 1; if (desp_tipo > 10) {desp_tipo = 0;} EEPROM.write(202,desp_tipo);}
  if (conf == 3 && key == '#'){cc2 = cc2 + 1;}
  if (conf == 4 && key == '#'){cc3 = cc3 + 1;}
  if (conf >= 5) conf = 1;
  
  lcd.display();  
}

void disp_desp(){
  lcd.clearDisplay();lcd.setTextColor(BLACK);
  lcd.drawRoundRect(0,0,84,26,2,BLACK);  //moldura da hora
  lcd.setTextSize(3);lcd.setCursor(4,2);  // posição da hora
  if (dhora <= 9) {lcd.print("0");lcd.print(dhora);}
  if (dhora >  9) {lcd.print(dhora);}
  lcd.setCursor(33,2);lcd.print(":");
  lcd.setCursor(44,2);    // posição do minuto
  if (dminuto <= 9) {lcd.print("0");lcd.print(dminuto);}
  if (dminuto >  9) {lcd.print(dminuto);}
  lcd.setTextSize(1);
  
  //acerto da hora do despertador
  if (key == '#') acerto_desp = acerto_desp + 1;
  if (acerto_desp == 1) {
    lcd.setCursor(0,30);lcd.print("* Muda HORA");
    lcd.setCursor(0,40);lcd.print("# Confirma");
  }
  if (acerto_desp == 2) {
    lcd.setCursor(0,30);lcd.print("* Muda MINUTO");
    lcd.setCursor(0,40);lcd.print("# Confirma");
  }
  if (acerto_desp == 3 & desp == 1) {
    lcd.setCursor(0,30);lcd.print("* ALARME Ligad");
    lcd.setCursor(0,40);lcd.print("# Confirma");
  }
  if (acerto_desp == 3 & desp == 0) {
    lcd.setCursor(0,30);lcd.print("* ALARME Deslg");
    lcd.setCursor(0,40);lcd.print("# Confirma");
  }
if (acerto_desp == 4) {
    lcd.setCursor(0,30);
    if (desp == 1) {lcd.print("Despert.LIGADO");}
    if (desp == 0) {lcd.print("Despert. DELIG");}
    EEPROM.write(203,dhora);EEPROM.write(204,dminuto);EEPROM.write(201,desp);  //salva hora, minuto e situação do alarme
    lcd.display(); delay(2000); acerto_desp = 0;
  }
  if (acerto_desp == 1 & key == '*'){dhora = dhora + 1;if (dhora >= 24){dhora = 0;}}
  if (acerto_desp == 2 & key == '*'){dminuto = dminuto + 1;if (dminuto >= 60){dminuto = 0;}}
  if (acerto_desp == 3 & key == '*'){if (desp == 0){desp = 1;} else {desp = 0;}}
      
  if (acerto_desp == 0){disp_hora_peq(); disp_desp_peq();}   //hora não está sendo alterada. Mostra demais informações
  lcd.display();
}

void disp_hora(){
  lcd.clearDisplay();lcd.setTextColor(BLACK);
  lcd.drawRoundRect(0,0,84,26,2,BLACK);  //moldura da hora
  lcd.setTextSize(3);lcd.setCursor(4,2);  // posição da hora
  if (hora <= 9) {lcd.print("0");lcd.print(hora);}
  if (hora >  9) {lcd.print(hora);}
  lcd.setCursor(33,2);
  cc5 = cc5 + 1;
  if (cc5 < 10) lcd.print(" ");
  if (cc5 > 10 && cc5 < 20){lcd.print(":");}
  if (cc5 > 20) cc5 = 0;
  lcd.setCursor(44,2);    // posição do minuto
  if (minuto <= 9) {lcd.print("0");lcd.print(minuto);}
  if (minuto >  9) {lcd.print(minuto);}
  lcd.setTextSize(1);
  
  //acerto da hora
  if (key == '#') acerto_hora = acerto_hora + 1;
  if (acerto_hora == 1) {
    lcd.setCursor(0,30);lcd.print("* Muda HORA");
    lcd.setCursor(0,40);lcd.print("# Confirma");
  }
  if (acerto_hora == 2) {
    lcd.setCursor(0,30);lcd.print("* Muda MINUTO");
    lcd.setCursor(0,40);lcd.print("# Confirma");
  }
  if (acerto_hora == 3) {
    lcd.setCursor(0,30);lcd.print("Hora alterada");
    rtc.setTime(hora, minuto, 00);
    lcd.display(); delay(1500); acerto_hora = 0;
  }
  if (acerto_hora == 1 & key == '*'){hora = hora + 1;if (hora >= 24){hora = 0;}}
  if (acerto_hora == 2 & key == '*'){minuto = minuto + 1;if (minuto >= 60){minuto = 0;}}
      
  if (acerto_hora == 0){disp_prev(); disp_temp_peq();}   //hora não está sendo alterada. Mostra demais informações
  lcd.display();
}

void disp_monit(){
  lcd.clearDisplay();lcd.setTextColor(BLACK);
  lcd.drawRoundRect(0,0,84,26,2,BLACK);  //moldura da hora
  lcd.setTextSize(3);lcd.setCursor(4,2);  // posição da hora
  if (hora <= 9) {lcd.print("0");lcd.print(hora);}
  if (hora >  9) {lcd.print(hora);}
  lcd.setCursor(33,2);
  cc5 = cc5 + 1;
  if (cc5 < 20) lcd.print(" ");
  if (cc5 > 20 && cc5 < 40){lcd.print(":");}
  if (cc5 > 40) cc5 = 0;
  lcd.setCursor(44,2);    // posição do minuto
  if (minuto <= 9) {lcd.print("0");lcd.print(minuto);}
  if (minuto >  9) {lcd.print(minuto);}

  lcd.setTextSize(1);             
  if (sbot_monit == HIGH && smonit == 0 ){      // Monitoração iniciada  *********
    cc7 = 80;ligaluz(1);gettime();smonit = 1;sbot_monit = LOW;mov_cont = 0;
    tempo_sono_inic = hora * 60 + minuto;hora_pos = hora + 1;if (hora_pos == 24) hora_pos = 0;limpa_grafsono();
    lcd.setCursor(0,28);lcd.print("  Monitoracao");
    lcd.setCursor(0,38);lcd.print("   Iniciada");lcd.display(); delay(1500);}
  else if (sbot_monit == HIGH && smonit == 1 ){    // Monitaração terminada  **********
    ligaluz(1);smonit = 0; tempo_sono_final = hora * 60 + minuto;salva_monit();relat_num = lastEEP;func = 7;
    lcd.setCursor(0,28);lcd.print("  Monitoracao");
    lcd.setCursor(0,38);lcd.print("   Concluida"); lcd.display();delay(1500);}

  tempo_sono_parcial = hora * 60 + minuto;    
  if (tempo_sono_parcial < tempo_sono_inic) tempo_sono_parcial = tempo_sono_parcial + 1440;  //passou da meia noite, add 1 dia 
  hora_parcial = (tempo_sono_parcial - tempo_sono_inic) / 60;     
  minuto_parcial = (tempo_sono_parcial - tempo_sono_inic) - (hora_parcial * 60);
    
  lcd.fillRoundRect(0,25,36,23,1,BLACK);
  lcd.setCursor(1,30);lcd.setTextColor(WHITE);lcd.setTextSize(2);
  if (mov_cont <=9) {lcd.print(" ");}
  lcd.print(mov_cont);lcd.setTextSize(1);lcd.setTextColor(BLACK);
  lcd.drawRoundRect(35,25,49,23,1,BLACK);            //box
  lcd.setCursor(37,27);lcd.print("T.Sono");lcd.setCursor(37,37);
  cc8 = cc8 + 1;  if (cc8 <= 5){lcd.fillRoundRect(0,25,36,23,1,BLACK);} if (cc8 > 15 ) cc8 = 0;   //pisca qt de movimentos
  lcd.print(hora_parcial);lcd.print("h");if (minuto_parcial <=9 ){lcd.print("0");}lcd.print(minuto_parcial);
    if (desp == 1) lcd.drawBitmap(70,37,af,8,8,BLACK);
  lcd.display();
}

void disp_prev1(){
  letemp();
  lcd.clearDisplay();lcd.setTextColor(BLACK);
  lcd.drawRoundRect(0,0,64,26,1,BLACK);  //moldura da hora
  for (i=1;i<=84;i=i+2){lcd.drawPixel(i,13,BLACK);}
  lcd.drawRoundRect(0,25,28,23,1,BLACK);
  lcd.drawRoundRect(27,25,28,23,1,BLACK);
  lcd.drawRoundRect(54,25,30,23,1,BLACK);
  lcd.setTextSize(1);
  lcd.setCursor(66,4);lcd.print("Sol");
  lcd.setCursor(66,15);lcd.print("Chu");
  lcd.setCursor(4,28);lcd.print("Max");lcd.setCursor(4,38);lcd.print(pres_max);
  lcd.setCursor(32,28);lcd.print("Min");lcd.setCursor(32,38);lcd.print(pres_min);
  lcd.setCursor(60,28);lcd.print("Atu"); lcd.setCursor(60,38);lcd.print(pres);
  
  for (i=1;i<=hpres_cont;i++){lcd.fillRect(i*5-2,25,3,map(hpres[i],924,940,0,-25),BLACK);}  //pressão varia de 890 a 940 em sp
  lcd.display();
}

void disp_qt_mov(){
  timeout_val = 2000;      // Aumenta o timeout nesta tela
  max_val = 0;
  lcd.clearDisplay();lcd.setTextColor(WHITE);lcd.fillRect(0,0,84,10,BLACK);
  lcd.setCursor(1,1);lcd.print("Movimentos");lcd.setTextColor(BLACK);
  int boxinit = -2;
  for (i=1;i<=20;i++){                     // captura o maior valor para referência no gráfico
    vatual = EEPROM.read(i+20); if (vatual == 255) {vatual = 0;}    
    if (vatual > max_val) {max_val = vatual;} }
  for (i=1;i<=20;i++){
    boxinit = boxinit + 4;
    int boxsize = map(EEPROM.read(i+20),0,max_val,0,34);      // 21 é a posição da memória dos movimentos
    lcd.fillRect(boxinit,48-boxsize,3,boxsize,BLACK);
  }
  lcd.display();
}

void disp_temp_inerte(){
  timeout_val = 2000;      // Aumenta o timeout nesta tela
  max_val = 0;
  lcd.clearDisplay();lcd.setTextColor(WHITE);lcd.fillRect(0,0,84,10,BLACK);
  lcd.setCursor(1,1);lcd.print("Tempo Inerte");lcd.setTextColor(BLACK);
  int boxinit = -2;
  for (i=101;i<=120;i++){if ((EEPROM.read(i)*60+EEPROM.read(i+20)) > max_val) max_val = EEPROM.read(i)*60+EEPROM.read(i+20);}
  for (i=1;i<=20;i++){
    boxinit = boxinit + 4;
    int t = EEPROM.read(i+100)* 60 + EEPROM.read(i+120);     // 101 é a posição da memória do tempo inerte
    int boxsize = map(t,0,max_val,0,34);    // 480 é o máximo de 8 horas sem movimentos durante o sono
    lcd.fillRect(boxinit,48-boxsize,3,boxsize,BLACK);
  }
  lcd.display();  
}

void disp_tempo_sono(){
  timeout_val = 2000;      // Aumenta o timeout nesta tela
  max_val = 0;
  lcd.clearDisplay();lcd.setTextColor(WHITE);lcd.fillRect(0,0,84,10,BLACK);
  lcd.setCursor(1,1);lcd.print("Tempo Sono");lcd.setTextColor(BLACK);
  int boxinit = -2;
  for (i=41;i<=60;i++){if ((EEPROM.read(i)*60+EEPROM.read(i+20)) > max_val) max_val = EEPROM.read(i)*60+EEPROM.read(i+20);}
  for (i=1;i<=20;i++){
    boxinit = boxinit + 4;
    int t = EEPROM.read(i+40)* 60 + EEPROM.read(i+60);      // 41 é a posição da memória do tempo de sono
    int boxsize = map(t,0,max_val,0,34);    // 960 é o máximo de 16 horas de sono
    lcd.fillRect(boxinit,48-boxsize,3,boxsize,BLACK);
  }
  lcd.display();
}

void graf_sono(){
  timeout_val = 2000;      // Aumenta o timeout nesta tela
  max_val = 0;
  lcd.clearDisplay();lcd.setTextColor(WHITE);lcd.fillRect(0,0,84,10,BLACK);
  lcd.setCursor(1,1);lcd.print("Grafico Sono");lcd.setTextColor(BLACK);
  int boxinit = -2;
  for (i=81;i<=100;i++){if (EEPROM.read(i) > max_val) max_val = EEPROM.read(i); }    //checa maior valor
  for (i=81;i<=100;i++){
    boxinit = boxinit + 4;
    int boxsize = map(EEPROM.read(i),0,max_val,0,34);
    lcd.fillRect(boxinit,48-boxsize,3,boxsize,BLACK);
  }
  lcd.display(); 
}

void letemp(){
  cc4 = cc4 + 1;
  temp_tot = temp_tot + bmp.readTemperature();
  if (cc4 >= 10) {cc4 = 0;temp = temp_tot / 10;temp_tot = 0;}     // média da temperatura das última 10 medições
  pres = bmp.readPressure()/100;
  if (pres > pres_max) pres_max = pres;
  if (pres < pres_min) pres_min = pres;
  if ((minuto==0||minuto==10||minuto==20||minuto==30||minuto==40||minuto==50) && minuto != minuto_ant1) {
    hpres_cont = hpres_cont + 1;
    if (hpres_cont >= 12){for (i=1;i<=11;i++){hpres[i]=hpres[i+1];} hpres[12]=pres; hpres_cont = 11;}
    hpres[hpres_cont] = pres;
    minuto_ant1 = minuto;
  }
}

void monit(){
   smov = digitalRead(mov);
   if (smov == HIGH) {
     c6 = c6 + 1;
     if (c6 >= 30) {                 // Opa! mexeu. 30 é o ajuste de movimentos
       mov_cont = mov_cont + 1;      // soma 1 movimento
       c6 = 0;                       
       if (tempo_mov_ant > 0){       // a primeira vez não calcula
         tempo_mov_atual = hora * 60 + minuto;        // captura tempo atual
         if (tempo_mov_atual < tempo_mov_ant) tempo_mov_atual = tempo_mov_atual + 1440;     //passou da meia noite
         tempo_sem_mov = tempo_mov_atual - tempo_mov_ant;
         if (tempo_sem_mov > tempo_sem_mov_max) {
           tempo_sem_mov_max = tempo_sem_mov;    // 
           hora_maior_mov = hora;
           minuto_maior_mov = minuto;
         }
       }
       tempo_mov_ant = (hora * 60) + minuto;
     }
   }     
   
   //captura quantidade de movimentos realizados em cada hora
   if (hora == hora_pos) {
     cc7 = cc7 + (1 && cc7 <= 100);
     mov_cont_atual = mov_cont - mov_cont_ant;
     EEPROM.write(cc7,mov_cont_atual);
     mov_cont_ant = mov_cont;
     hora_pos = hora_pos + 1; if (hora_pos == 24) hora_pos = 0;
   }
}

void relat_sono(){
  timeout_val=2000;             //aumenta o timeout nessa tela
  if (key == '7'){relat_num = lastEEP;}
  if (key == '*'){relat_num = relat_num - 1;if (relat_num <= 0){relat_num = lastEEP;}}
  if (key == '#'){relat_num = relat_num + 1;if (relat_num > lastEEP){relat_num = 1;}}
  
  ledados_relat();
  
  lcd.clearDisplay();lcd.setTextColor(WHITE);lcd.fillRect(0,0,84,10,BLACK);
  lcd.setTextSize(1);lcd.setCursor(1,1); lcd.print("Rel:SONO");lcd.setCursor(72,1);lcd.print(relat_num - lastEEP);
  lcd.setTextColor(BLACK);
  lcd.setCursor(0,11);lcd.print("T.sono :");lcd.print(relat[1]); lcd.print("h"); if (relat[2] <= 9 ){lcd.print("0");} lcd.print(relat[2]);
  lcd.setCursor(0,21);lcd.print("Qt.mov :");lcd.print(relat[3]);
  lcd.setCursor(0,31);lcd.print("T.inert:");lcd.print(relat[4]);lcd.print("h");if (relat[5] <= 9 ){lcd.print("0");}lcd.print(relat[5]);
  lcd.setCursor(0,41);lcd.print("     As:");lcd.print(relat[6]);lcd.print(":");if (relat[7] <= 9 ){lcd.print("0");}lcd.print(relat[7]);
  lcd.display();
}

void salva_monit(){
  lastEEP = lastEEP + 1;
  if (lastEEP >= 20){    // número máximo atingido
    for (i=21;i<40;i++)  {EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    for (i=41;i<60;i++)  {EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    for (i=61;i<80;i++)  {EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    for (i=101;i<120;i++){EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    for (i=121;i<140;i++){EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    for (i=141;i<160;i++){EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    for (i=161;i<180;i++){EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    for (i=181;i<200;i++){EEPROM.write(i,EEPROM.read(i+1));}   //move cada atividade para posição anterior
    lastEEP = 20;         // mantém última posição como 20
  }

  if (tempo_sono_final <= tempo_sono_inic) tempo_sono_final = tempo_sono_final + 1440;  //passou da meia noite, add 1 dia 
  hora_tot = (tempo_sono_final - tempo_sono_inic) / 60;     
  minuto_tot = (tempo_sono_final - tempo_sono_inic) - (hora_tot * 60);
  if (hora_tot >= 24) hora_tot = 0;
  tempo_mov_max_h = tempo_sem_mov_max / 60;                       // Maior quantidade de tempo sem movimento (horas)
  tempo_mov_max_m = tempo_sem_mov_max - (tempo_mov_max_h * 60);   // Maior qt de tempo sem movimento (minutos)
  //Salva posições atuais
  EEPROM.write(0,lastEEP);             
  EEPROM.write(lastEEP+20,mov_cont);
  EEPROM.write(lastEEP+40,hora_tot);
  EEPROM.write(lastEEP+60,minuto_tot);
  EEPROM.write(lastEEP+100,tempo_mov_max_h);
  EEPROM.write(lastEEP+120,tempo_mov_max_m);
  EEPROM.write(lastEEP+140,hora_maior_mov);
  EEPROM.write(lastEEP+160,minuto_maior_mov);

}

void checa_desp(){
  if (hora == dhora & minuto == dminuto & minuto_ant != minuto) {
    digitalWrite(led,HIGH);delay(500);digitalWrite(led,LOW);delay(100);
    digitalWrite(led,HIGH);delay(500);digitalWrite(led,LOW);delay(100);
    digitalWrite(led,HIGH);delay(500);digitalWrite(led,LOW);delay(100);
    lights = 1;digitalWrite(led,HIGH);
    if (desp_tipo > 0){for (i=1;i<=desp_tipo;i++){digitalWrite(buzz, HIGH);delay(40);digitalWrite(buzz, LOW);delay(400);}}
  }
  minuto_ant = minuto;
}

void disp_desp_peq(){
  lcd.setTextSize(1);             
  lcd.drawRoundRect(35,25,49,23,1,BLACK);            //box
  lcd.setCursor(37,27);lcd.print("Despert");lcd.setCursor(37,37);sdesp();
}

void disp_hora_peq(){
  lcd.setTextSize(1);             
  lcd.drawRoundRect(0,25,36,23,1,BLACK);            //boxes
  lcd.setCursor(2,27);lcd.print("Hora");lcd.setCursor(2,37);shora();
}

void disp_prev(){
  lcd.setTextSize(1);             
  lcd.drawRoundRect(0,25,36,23,1,BLACK);
  lcd.fillRoundRect(1,36,35,9,1,BLACK);
  lcd.setCursor(2,27);lcd.print("Prev");lcd.setTextColor(WHITE);lcd.setCursor(2,37);
  if (pres < 930){ lcd.print("CHUVA");} else {lcd.print(" SOL");}    // 930 é um intermediário para SP
  lcd.setTextColor(BLACK);
}

void gettime(){
  lcd.setTextColor(BLACK);
  t = rtc.getTime();
  hora = t.hour; minuto = t.min;
}

void sdesp(){
  if (desp == 1){    //se alarme está ativo
    if (dhora <= 9) {lcd.print("0");} lcd.print(dhora);lcd.print(":");
    if (dminuto <= 9) {lcd.print("0");} lcd.print(dminuto);  
    lcd.drawBitmap(70,37,af,8,8,BLACK);
  }
  if (desp == 0){lcd.print("--:--");}
}

void shora(){
  if (hora <= 9) {lcd.print("0");} lcd.print(hora);lcd.print(":");
  if (minuto <= 9) {lcd.print("0");} lcd.print(minuto);  
}

void disp_temp_peq(){
  lcd.setTextSize(1);             
  lcd.drawRoundRect(35,25,33,23,1,BLACK);lcd.drawRoundRect(67,25,17,23,1,BLACK);            //boxes
  lcd.setCursor(37,27);lcd.print("Temp");lcd.setCursor(37,37);lcd.print(temp,1);lcd.drawBitmap(58,37,gr,8,4,BLACK);
  //lcd.setCursor(70,27);lcd.print("D.");
  if (desp == 1)lcd.drawBitmap(70,37,af,8,8,BLACK);    //se alarme está ativo
}

void ligaluz(int tipo_luz){
  if (tipo_luz == 0){        // acende ou apaga somente o led do lcd
    cc = -cc;;
    if (cc == -1){lights = 1;digitalWrite(led, HIGH);sluz = 1;}
    else {lights = 0;digitalWrite(led, LOW);sluz = 0;}
    cont = 0;
  }
  if (tipo_luz == 3){         // acende ou apaga o led do lcd e a luz
    cc = -cc;
    if (cc == -1){lights = 1;digitalWrite(luz, HIGH);digitalWrite(led, HIGH);sluz = 3;}
    else {lights = 0;digitalWrite(luz, LOW);digitalWrite(led, LOW);sluz = 0;}
    cont = 0;
  }
  if (tipo_luz == 1){lights = 1;digitalWrite(led, HIGH);}
  if (tipo_luz == 2){lights = 0;digitalWrite(led, LOW);}
  delay(100);
}

void esmaece(){
  if (lights == 1) {cont = cont + 1;}
  if (cont >= 750 * tempoluz){             //tempo luz começa a esmaecer
    for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -=1) {
      analogWrite(led, fadeValue);
      if (sluz = 0) analogWrite(led, fadeValue);
      if (sluz = 3) analogWrite(luz, fadeValue);
      delay(80);
    }
    lights = 0; cont = 0;
  }
}

void ledados_relat(){
  relat[1] = EEPROM.read(40+relat_num);       // horas total
  relat[2] = EEPROM.read(60+relat_num);       // minutos total
  relat[3] = EEPROM.read(20+relat_num);       // qt de movimentos
  relat[4] = EEPROM.read(100+relat_num);      // horas tempo inerte
  relat[5] = EEPROM.read(120+relat_num);      // minutos tempo inerte
  relat[6] = EEPROM.read(140+relat_num);      // que horas foi
  relat[7] = EEPROM.read(160+relat_num);      // que minutos foi
}

void limpa_grafsono(){
  for (i=81;i<=100;i++){EEPROM.write(i,0);}
}

void logo(){
  lcd.clearDisplay();
  lcd.setCursor(2, 2); lcd.print("Sleep Quality");
  lcd.setTextSize(3); lcd.setTextColor(WHITE);
  for (i=-10;i<=20;i++){
    lcd.fillRoundRect(16, 16, 58, 28, 2, BLACK);
    lcd.drawRect(0, 0, 84, 48, BLACK);
    lcd.setCursor(i, 20); lcd.print("VSL");lcd.display();delay(20);
  }
  delay(300);func = 1;  // sai do logo
  lcd.setTextSize(1); lcd.display();
}
