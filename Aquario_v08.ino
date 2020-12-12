//###############################################
//                       PROGRAMA AQUARIO
#define VERSAO_SW        "S/W Versao 0.8"
//###############################################

//************************************
// BIBLIOTECAS
//************************************
#include <LiquidCrystal.h>
#include <Wire.h>
//#include <stdlib.h>

//************************************
// DEFINIÇÃO DOS NÚMEROS DOS PINOS
//************************************
//Pinos Digitais
#define pin_0             0  //Livre
#define pin_1             1  //Livre
#define pin_OutRelay      2  //Saída Relé
#define pin_3             3  //Livre
#define pin_lcdDB4        4  //LCD pino Data 4
#define pin_lcdDB5        5  //LCD pino Data 5
#define pin_lcdDB6        6  //LCD pino Data 6
#define pin_lcdDB7        7  //LCD pino Data 7
#define pin_lcdRS         8  //LCD pino RS
#define pin_lcdEnable     9  //LCD pino Enable
#define pin_lcdBacklight  10 //LCD pino Backlight
#define pin_11            11 //Livre
#define pin_12            12 //Livre
#define pin_13            13 //Livre

//Pinos Analógicos
#define AD_Teclado        0 //Teclado analógico do LCD DFR
#define AD_1              1 //Livre
#define AD_2              2 //Livre
#define AD_3              3 //Livre
#define I2C_SDA_RTC       4 //Interface I2C para RTC - SDA
#define I2C_SCL_RTC       5 //Interface I2C para RTC - SCL

//************************************
// DEFINIÇÃO DE PERIFÉRICOS
//************************************
LiquidCrystal lcd(pin_lcdRS, pin_lcdEnable, pin_lcdDB4, pin_lcdDB5, pin_lcdDB6, pin_lcdDB7);

//************************************
// STRUCTS
//************************************
struct DATA_HORA {
	byte segundos;
	byte minutos;
	byte horas;
	byte dia_semana;
	byte dia;
	byte mes;
	byte ano;
};

struct HORA_TIMER {
	byte minutos;
	byte horas;
	boolean ativo;
};

struct CYC_TASK {
	long Ciclo_ms;
	long Ultima_exec;
	unsigned int Cnt_ciclos;
};


//************************************
// CONSTANTES GLOBAIS
//************************************
//Teclado
#define btnRIGHT         0     //Tecla RIGHT
#define btnUP            1     //Tecla UP
#define btnDOWN          2     //Tecla DOWN
#define btnLEFT          3     //Tecla LEFT
#define btnSELECT        4     //Tecla SELECT
#define btnNONE          5     //Nenhuma tecla
//ID I2C
#define DS1307_ID        0x68  //ID I2C para o RTC DS1307 (Real Time Clock - RTC)
// Exibições do LCD
#define LCD_delay_ms     1000  // Delay para exibição de telas iniciais
// Telas LCD
#define Tela_normal      0     // Tela 0 = Tela normal (exibe hora/data e status)
#define Tela_menu        1     // Tela 1 = Tela de menu
#define Tela_AjRelogio   2     // Tela 2 = Tela de ajuste do relógio
#define Tela_TimerON     3     // Tela 3 = Tela de ajuste de timer ON
#define Tela_TimerOFF    4     // Tela 4 = Tela de ajuste de timer OFF
#define Tela_StatusRTC   5     // Tela 5 = Tela de status do relógio de tempo real (RTC) 
#define Tela_AjData      6     // Tela 6 = Tela de ajuste da data
// Tela 7 a 09 - RESERVA
#define Tela_Descanso    10    // Tela 10 = Tela de descanso
// Itens menu
#define Tamanho_menu     5     // Define o número de itens do menu
#define m_AjRelogio      0     // Item 0 do menu = Ajuste de hora
#define m_TimerON        1     // Item 1 do menu = Ajuste de timer ON
#define m_TimerOFF       2     // Item 2 do menu = Ajuste de timer OFF
#define m_StatusRTC      3     // Item 3 do menu = Status do RTC
#define m_Sair           4     // Item 4 do menu = Sair do menu
// Seleção para ajuste de relógio
#define In_horas         0     // Seleciona horas para ajuste
#define In_minutos       1     // Seleciona minutos para ajuste
#define In_segundos      2     // Seleciona segundos para ajuste
#define In_Data          3     // Seleciona opção para ajuste de data
#define In_Cancel        4     // Seleciona opção "Cancela"
#define In_OK            5     // Seleciona opção "OK"
// Seleção para ajuste de data
#define In_dia           0     // Seleciona dia para ajuste
#define In_mes           1     // Seleciona mês para ajuste
#define In_ano           2     // Seleciona ano para ajuste
#define In_dia_semana    3     // Seleciona dia da semana para ajuste
#define In_Cancel        4     // Seleciona opção "Cancela"
#define In_OK            5     // Seleciona opção "OK"
#define In_Relogio       6     // Seleciona opção para ajuste de relogio
// Seleção para ajuste do TIMER ON e OFF
#define InTIMER_horas    0     // Seleciona horas para TIMER
#define InTIMER_minutos  1     // Seleciona minutos para TIMER
#define InTIMER_Ativo    2     // Seleciona opção "Ativo/Inativo"
#define InTIMER_Cancel   3     // Seleciona opção "Cancela"
#define InTIMER_OK       4     // Seleciona opção "OK"
// Ajuste tempo backlight off
#define TmpBcklghtOFF    20    // Tempo para apagar backlight (seg)

//************************************
// VARIÁVEIS GLOBAIS
//************************************
int tecla_lcd = 0;
char char_temp[20];
byte seq_exibicao = 0;
byte tela_exibida = Tela_normal;
byte tecla_apertada = btnNONE;
byte selec_input = In_horas;
byte selecao_menu = 0;
byte contador_bcklght_off = 0;
boolean clock_1s = false;
boolean tecla_travada = false;
boolean OutRelay = LOW;
boolean LcdBacklight = HIGH;
boolean RTC_OK = false;
boolean Exec_Prim_Loop = false;
String Menu_Itens[6] = { "Ajustar hora... ", "Timer ON...     ", "Timer OFF...    ", "Status relogio  ", "Sair            ", "                " };
DATA_HORA dt_hr_atual = { 0,0,0,0,0,0,0 };
DATA_HORA dt_hr_mudar = { 0,0,0,0,0,0,0 };
HORA_TIMER temp_timer = { 0,0,false };
HORA_TIMER hr_timer_ON = { 0,0,false };
HORA_TIMER hr_timer_OFF = { 0,0,false };
CYC_TASK Task[20];

//************************************
// PROCEDURES/FUNCTIONS/MACROS
//************************************ 
// MACROS
#define readIIC() Wire.read()

// ** Converte BCD->DEC **
byte bcd2dec(byte num_bcd){
  return ((num_bcd/16 * 10) + (num_bcd % 16));
}

// ** Converte DEC->BCD **
byte dec2bcd(byte num_dec){
  return ((num_dec/10 * 16) + (num_dec % 10));
}

// ** Adiciona zeros a esquerda se o número for < 10 **
String str_zero_esq(byte numero){
  if (numero < 10){
    return "0" + (String)numero;
  }else{
    return (String)numero;
  }
}

// ** Converte valor de dia semana por texto equivalente **
String status_timer(boolean timer_ativo){
  if (timer_ativo){
    return "Ativo";
  }else{
    return "Canc ";
  }
}

// ** Converte valor de dia semana por texto equivalente **
String dia_semana_str(byte dia_semana_byt){
  switch (dia_semana_byt){                       // Recebe dia da semana e converte em texto
    case 0:
      return "SEG";
      break;
    case 1:
      return "TER";
      break;
    case 2:
      return "QUA";
      break;
    case 3:
      return "QUI";
      break;
    case 4:
      return "SEX";
      break;
    case 5:
      return "SAB";
      break;
    case 6:
      return "DOM";
      break;
    default:
      return "ERR";
      break;
  }
}

// ** Leitura do teclado **
int read_teclado(){
  int AD_tecla_in;
  AD_tecla_in = analogRead(AD_Teclado);     // Lê valor analógico para determinar tecla apertada 
  if (AD_tecla_in > 1000) return btnNONE;   // Nenhuma tecla apertada, Valor analógico = 1023
  if (AD_tecla_in < 50)   return btnRIGHT;  // Valor analógico = 0   -> Tecla RIGHT
  if (AD_tecla_in < 150)  return btnUP;     // Valor analógico = 99  -> Tecla UP
  if (AD_tecla_in < 350)  return btnDOWN;   // Valor analógico = 255 -> Tecla DOWN
  if (AD_tecla_in < 450)  return btnLEFT;   // Valor analógico = 407 -> Tecla LEFT 
  if (AD_tecla_in < 750)  return btnSELECT; // Valor analógico = 639 -> Tecla LEFT 
  return btnNONE;                           // Se não houver outra opção, considerar nenhuma tecla apertada
}

// ** Exibição de telas rapidas **
void LCD_flash(char msg_display[16]){
  lcd.setCursor(0,0);            // Posiciona o cursor na linha superior
  lcd.clear();                   // Limpa o display
  lcd.print(msg_display);        // Exibe mensagem por "LCD_delay_ms" segundos
  delay(LCD_delay_ms);
}

// ** Inicia e testa I2C. Coloca ponteiro na posição "ponteiro" **
boolean Inicia_I2C(byte ponteiro){
  Wire.beginTransmission(DS1307_ID);
  Wire.write(ponteiro);
  if (Wire.endTransmission() == 0){
    return true;
  }else{
    return false;
  }
}

// ** Escreve na memoria não-volátil NV do DS1307  **
void write_mem(byte horas, byte minutos, boolean ativo, byte endereco_mem){
  Wire.beginTransmission(DS1307_ID);   // Inicia transmissão de comando via I2C p/ RTC
  Wire.write(endereco_mem);            // Ajusta ponteiro de dados da memória para posição de memoria NV
  Wire.write(minutos);                 // Grava minutos
  Wire.write(horas);                   // Grava horas
  Wire.write(ativo);                   // Grava ativo/inativo  
  Wire.endTransmission();
}

// ** Lê da memoria não-volátil NV do DS1307  **
boolean read_mem(byte endereco_mem){
  boolean Read_OK = false;

  Read_OK = Inicia_I2C(endereco_mem);
  Wire.requestFrom(DS1307_ID, 3);    // Requisita 1 byte a partir da posição do ponteiro de memória (0x00) (dados de data e hora)
  temp_timer.minutos = Wire.read();  // Recebe minutos
  temp_timer.horas = Wire.read();    // Recebe horas
  temp_timer.ativo = Wire.read();    // Recebe status ativo/inativo
  return Read_OK;                    // Retorna status se a leitura da memória foi bem sucedida  
}

// ** Modifica e grava hora no RTC **
void write_RTC(byte horas, byte minutos, byte dia_semana, byte dia, byte mes, byte ano){
  byte segundos;
  // Pára relógio e ajusta horário/data
  Wire.beginTransmission(DS1307_ID);   // Inicia transmissão de comando via I2C p/ RTC
  Wire.write(0x00);                    // Ajusta ponteiro de dados da memória para posição de dados do relógio
  Wire.write(0x00 | 0x80);             // Pára o relógio (lógica OR c/ 0x80) e ajusta segundos
  Wire.write(dec2bcd(minutos));        // Ajusta minutos
  Wire.write(dec2bcd(horas) & 0x3f);   // Ajusta horas em formato 24h
  Wire.write(dec2bcd(dia_semana));     // Ajusta dia da semana, 0x00 = segunda ~ 0x06 = domingo
  Wire.write(dec2bcd(dia));            // Ajusta dia do mês
  Wire.write(dec2bcd(mes));            // Ajusta mês
  Wire.write(dec2bcd(ano));            // Ajusta ano, 2000 = 00 ~ 2099 = 99
  Wire.endTransmission();
  // Reinicia o relógio
  RTC_OK = Inicia_I2C(0x00);
  Wire.requestFrom(DS1307_ID, 1);      // Requisita 1 byte a partir da posição do ponteiro de memória (0x00)
  segundos = Wire.read();              // Lê byte requisitado
  Wire.beginTransmission(DS1307_ID);   // Inicia transmissão de comando via I2C p/ RTC
  Wire.write(0x00);                    // Ajusta ponteiro de dados da memória para posição de dados do relógio
  Wire.write(segundos & 0x7f);         // Pára o relógio (lógica OR c/ 0x80) e ajusta segundos, pois estão na mesma posição de memória
  Wire.endTransmission();
}

// ** Lê relógio e atualiza variável global "dt_hr_atual" **
void read_RTC(){
    RTC_OK = Inicia_I2C(0x00);
    Wire.requestFrom(DS1307_ID, 7);                       // Requisita 1 byte a partir da posição do ponteiro de memória (0x00) (dados de data e hora)
    dt_hr_atual.segundos = bcd2dec(Wire.read() & 0x7f);   // Recebe segundos
    dt_hr_atual.minutos = bcd2dec(Wire.read());           // Recebe minutos
    dt_hr_atual.horas = bcd2dec(Wire.read());             // Recebe horas
    dt_hr_atual.dia_semana = bcd2dec(Wire.read());        // Recebe dia da semana e converte em texto
    dt_hr_atual.dia = bcd2dec(Wire.read());
    dt_hr_atual.mes = bcd2dec(Wire.read());
    dt_hr_atual.ano = bcd2dec(Wire.read());
}

// ** Exibe a hora atual no LCD **
void exibe_hora_LCD(int linha){
  String hr_str = "";
  
  hr_str = str_zero_esq(dt_hr_atual.horas) + ":" + str_zero_esq(dt_hr_atual.minutos) + ":" + str_zero_esq(dt_hr_atual.segundos);
  lcd.setCursor(4,linha);
  lcd.print(hr_str);
}

// ** Exibe a data atual **
void exibe_data_LCD(int linha){
  String dt_str = "";

  dt_str = str_zero_esq(dt_hr_atual.dia) + "/" + str_zero_esq(dt_hr_atual.mes) + "/" + str_zero_esq(dt_hr_atual.ano);
  dt_str += " ";
  dt_str += dia_semana_str(dt_hr_atual.dia_semana);
  lcd.setCursor(2,linha);
  lcd.print(dt_str);
}

// ** Troca de tela **
void troca_tela(byte tela){
  lcd.clear();
  tela_exibida = tela;
  tecla_travada = true; 
}

// ** Pisca campo **
void campo_pisca(byte campo_selec, byte campo_valor, String str_OFF, String str_ON){
  if ((campo_selec == campo_valor) && clock_1s){
    lcd.print(str_ON);
  }else{
    lcd.print(str_OFF);
  }
}

// ** Verifica se executa task **
boolean Exec_Task(int NumTask, boolean Habilita_task){
  if (Habilita_task){
    if ( millis() >= (Task[NumTask].Ultima_exec + Task[NumTask].Ciclo_ms) ){
      Task[NumTask].Ultima_exec = millis();
      Task[NumTask].Cnt_ciclos++;
      return true;
    }else{
      return false;
    }
  }else{ 
    return false;
  }
}
 
//************************************
// ROTINA "SETUP"
//************************************ 
void setup(){
  // ** Inicialização dos pinos digitais, saídas **
  pinMode(pin_lcdBacklight, OUTPUT);     // Pino backlight = output
  digitalWrite(pin_lcdBacklight, HIGH);  // Inicia com backlight = ON
  pinMode(pin_OutRelay, OUTPUT);         // Pino da saída a relé = output
  digitalWrite(pin_OutRelay, LOW);       // Inicia com relé = OFF
  
  // ** Inicialização do LCD ** 
  lcd.begin(16, 2);                      // Inicialização do LCD

  // ** Exibe versão do software ** 
  LCD_flash(" ><> Aquario ><>");
  LCD_flash(VERSAO_SW);
   
  // ** Inicialização I2C **
  LCD_flash("Lendo relogio...");
  delay(2000);                           // Delay para comunicação RTC ficar pronta, após alimentação ligada
  Wire.begin();                          // Inicialização da comunicação I2C p/ RTC
  RTC_OK = Inicia_I2C(0x00);             // Transmite inicialização do ponteiro em 0x00 e testa comunicação
  read_RTC();
  // Verifica se a leitura do RTC está OK. Se NOK, RTC_OK = false.
  if ( dt_hr_atual.segundos < 0 || dt_hr_atual.segundos > 59 ) RTC_OK = false;
  if ( dt_hr_atual.minutos < 0 || dt_hr_atual.minutos > 59 ) RTC_OK = false;
  if ( dt_hr_atual.horas < 0 || dt_hr_atual.horas > 23 ) RTC_OK = false;
  if ( dt_hr_atual.dia_semana < 0 || dt_hr_atual.dia_semana > 6 ) RTC_OK = false;
  if ( dt_hr_atual.dia < 1 || dt_hr_atual.dia > 31 ) RTC_OK = false;
  if ( dt_hr_atual.mes < 1 || dt_hr_atual.mes > 12 ) RTC_OK = false;
  if ( dt_hr_atual.ano < 0 || dt_hr_atual.ano > 99 ) RTC_OK = false;
  // Caso a leitura do relogio esteja incorreta, iniciar relógio
  if (!RTC_OK) write_RTC(0, 0, 0, 1, 1, 00);
 
  if (RTC_OK){
    LCD_flash("   Relogio OK   ");
  }else{
    LCD_flash("Falha no relogio");
  }
  LCD_flash("Lendo memoria...");
  if(read_mem(0x08)){
    hr_timer_ON = temp_timer;
    LCD_flash("Mem Timer ON OK");
  }else{
    LCD_flash("Falha Mem Tmr ON");
  }
  if(read_mem(0x10)){
    hr_timer_OFF = temp_timer;
    LCD_flash("Mem Timer OFF OK");
  }else{
    LCD_flash("Falha Mem TmrOFF");
  }
  lcd.clear();
  Exec_Prim_Loop = true;
  tela_exibida = Tela_normal;
}

//************************************
// ROTINA "LOOP"
//************************************   
void loop(){
  
  // TASK  00~09: Tasks de operação
  // TASKS 10~24: Tasks de exibição no LCD

  // **** TASKS DE OPERAÇÃO ****
  
  // ** TASK 0: Atualiza data/hora em dt_hr_atual **    
  Task[0].Ciclo_ms = 200;
  if ( (Exec_Task(0, true)) || Exec_Prim_Loop ){
    read_RTC();                           // Atualiza variável hora e data corrente
  }
  
  // ** TASK 1: Verifica teclado em função da tela exibida **    
  Task[1].Ciclo_ms = 0;
  if ( Exec_Task(1, true) ){
    tecla_apertada = read_teclado();
    if (tecla_apertada == btnNONE){
      tecla_travada = false;
    }else{
      contador_bcklght_off = 0;
    }
    if ( (tecla_apertada != btnNONE)&&( !tecla_travada ) ){
      switch(tela_exibida){
        case Tela_normal:
          switch (tecla_apertada){
            case btnSELECT:
              selecao_menu = 0;
              troca_tela(Tela_menu);
            break;
            case btnUP:
              OutRelay = true;
            break;
            case btnDOWN:
              OutRelay = false;
            break;            
          }
        break; 
        case Tela_menu:   
          switch (tecla_apertada){
            case btnSELECT:
              switch (selecao_menu){
                case m_AjRelogio:
                  selec_input = In_horas;
                  dt_hr_mudar = dt_hr_atual;
                  troca_tela(Tela_AjRelogio);
                break;
                case m_TimerON:
                  temp_timer = hr_timer_ON;
                  selec_input = InTIMER_horas;
                  troca_tela(Tela_TimerON);
                break;
                case m_TimerOFF:
                  temp_timer = hr_timer_OFF;
                  selec_input = InTIMER_horas;
                  troca_tela(Tela_TimerOFF);
                break;
                case m_StatusRTC:
                  troca_tela(Tela_StatusRTC);
                break;
                case m_Sair:
                  troca_tela(Tela_normal);
                break;
              }
            break;
            case btnUP:
              if (!tecla_travada & selecao_menu > 0 ) selecao_menu--;
              tecla_travada = true;
            break;
            case btnDOWN:
              if (!tecla_travada & selecao_menu < (Tamanho_menu-1) ) selecao_menu++;
              tecla_travada = true;
            break;
          }
        break;
        case Tela_StatusRTC:
          switch (tecla_apertada){
            case btnSELECT:
              troca_tela(Tela_menu);
            break;
          }
        break;
        case Tela_AjRelogio:
          switch (tecla_apertada){
            case btnSELECT:
              switch (selec_input){
                case In_Data:
                  selec_input = In_dia;
                  troca_tela(Tela_AjData);
                break;
                case In_Cancel:
                  troca_tela(Tela_menu);
                break;
                case In_OK:
                  write_RTC(dt_hr_mudar.horas, dt_hr_mudar.minutos, dt_hr_mudar.dia_semana, dt_hr_mudar.dia, dt_hr_mudar.mes, dt_hr_mudar.ano);
                  troca_tela(Tela_menu);
                break;
              }
            break;
            case btnUP:
              switch (selec_input){
                case In_segundos:
                  if (!tecla_travada & dt_hr_mudar.segundos < 59 ) dt_hr_mudar.segundos++;
                  tecla_travada = true;
                break;
                case In_minutos:
                  if (!tecla_travada & dt_hr_mudar.minutos < 59 ) dt_hr_mudar.minutos++;
                  tecla_travada = true;
                break;
                case In_horas:
                  if (!tecla_travada & dt_hr_mudar.horas < 23 ) dt_hr_mudar.horas++;
                  tecla_travada = true;
                break;
              }
            break;
            case btnDOWN:
              switch (selec_input){
                case In_segundos:
                  if (!tecla_travada & dt_hr_mudar.segundos > 0 ) dt_hr_mudar.segundos--;
                  tecla_travada = true;
                break;
                case In_minutos:
                  if (!tecla_travada & dt_hr_mudar.minutos > 0 ) dt_hr_mudar.minutos--;
                  tecla_travada = true;
                break;
                case In_horas:
                  if (!tecla_travada & dt_hr_mudar.horas > 0 ) dt_hr_mudar.horas--;
                  tecla_travada = true;
                break;
              }
            break;
            case btnRIGHT:
              if (!tecla_travada & selec_input < 5 ) selec_input++;
              tecla_travada = true;
            break;
            case btnLEFT:
              if (!tecla_travada & selec_input > 0 ) selec_input--;
              tecla_travada = true;
            break;
          }
        break;
        case Tela_AjData:
          switch (tecla_apertada){
            case btnSELECT:
              switch (selec_input){
                case In_Relogio:
                  selec_input = In_horas;
                  troca_tela(Tela_AjRelogio);
                break;
                case In_Cancel:
                  troca_tela(Tela_menu);
                break;
                case In_OK:
                  write_RTC(dt_hr_mudar.horas, dt_hr_mudar.minutos, dt_hr_mudar.dia_semana, dt_hr_mudar.dia, dt_hr_mudar.mes, dt_hr_mudar.ano);
                  troca_tela(Tela_menu);
                break;
              }
            break;
            case btnUP:
              switch (selec_input){
                case In_dia:
                  if (!tecla_travada & dt_hr_mudar.dia < 31 ) dt_hr_mudar.dia++;
                  tecla_travada = true;
                break;
                case In_mes:
                  if (!tecla_travada & dt_hr_mudar.mes < 12 ) dt_hr_mudar.mes++;
                  tecla_travada = true;
                break;
                case In_ano:
                  if (!tecla_travada & dt_hr_mudar.ano < 99 ) dt_hr_mudar.ano++;
                  tecla_travada = true;
                break;
                case In_dia_semana:
                  if (!tecla_travada & dt_hr_mudar.dia_semana < 6 ) dt_hr_mudar.dia_semana++;
                  tecla_travada = true;
                break;
              }
            break;
            case btnDOWN:
              switch (selec_input){
                case In_dia:
                  if (!tecla_travada & dt_hr_mudar.dia > 1 ) dt_hr_mudar.dia--;
                  tecla_travada = true;
                break;
                case In_mes:
                  if (!tecla_travada & dt_hr_mudar.mes > 1 ) dt_hr_mudar.mes--;
                  tecla_travada = true;
                break;
                case In_ano:
                  if (!tecla_travada & dt_hr_mudar.ano > 0 ) dt_hr_mudar.ano--;
                  tecla_travada = true;
                break;
                case In_dia_semana:
                  if (!tecla_travada & dt_hr_mudar.dia_semana > 0 ) dt_hr_mudar.dia_semana--;
                  tecla_travada = true;
                break;
              }
            break;
            case btnRIGHT:
              if (!tecla_travada & selec_input < 6 ) selec_input++;
              tecla_travada = true;
            break;
            case btnLEFT:
              if (!tecla_travada & selec_input > 0 ) selec_input--;
              tecla_travada = true;
            break;
          }
        break;
        case Tela_TimerOFF:
          switch (tecla_apertada){
            case btnSELECT:
              switch (selec_input){
                case InTIMER_Cancel:
                  temp_timer = hr_timer_OFF;
                  troca_tela(Tela_menu);
                break;
                case InTIMER_OK:
                  hr_timer_OFF = temp_timer;
                  write_mem(hr_timer_OFF.horas, hr_timer_OFF.minutos, hr_timer_OFF.ativo, 0x10);
                  troca_tela(Tela_menu);
                break;
                case InTIMER_Ativo:
                  temp_timer.ativo = !temp_timer.ativo;
                  tecla_travada = true;
                break;
              }
            break;
            case btnUP:
              switch (selec_input){
                case InTIMER_minutos:
                  if (!tecla_travada & temp_timer.minutos < 59 ) temp_timer.minutos++;
                  tecla_travada = true;
                break;
                case InTIMER_horas:
                  if (!tecla_travada & temp_timer.horas < 23 ) temp_timer.horas++;
                  tecla_travada = true;
                break;
                case InTIMER_Ativo:
                  temp_timer.ativo = true;
                  tecla_travada = true;
                break;
              }
            break;
            case btnDOWN:
              switch (selec_input){
                case InTIMER_minutos:
                  if (!tecla_travada & temp_timer.minutos > 0 ) temp_timer.minutos--;
                  tecla_travada = true;
                break;
                case InTIMER_horas:
                  if (!tecla_travada & temp_timer.horas > 0 ) temp_timer.horas--;
                  tecla_travada = true;
                break;
                case InTIMER_Ativo:
                  temp_timer.ativo = false;
                  tecla_travada = true;
                break;
              }
            break;
            case btnRIGHT:
              if (!tecla_travada & selec_input < 4 ) selec_input++;
              tecla_travada = true;
            break;
            case btnLEFT:
              if (!tecla_travada & selec_input > 0 ) selec_input--;
              tecla_travada = true;
            break;
          }
        break;
        case Tela_TimerON:
          switch (tecla_apertada){
            case btnSELECT:
              switch (selec_input){
                case InTIMER_Cancel:
                  temp_timer = hr_timer_ON;
                  troca_tela(Tela_menu);
                break;
                case InTIMER_OK:
                  hr_timer_ON = temp_timer;
                  write_mem(hr_timer_ON.horas, hr_timer_ON.minutos, hr_timer_ON.ativo, 0x08);
                  troca_tela(Tela_menu);
                break;
                case InTIMER_Ativo:
                  temp_timer.ativo = !temp_timer.ativo;
                  tecla_travada = true;
                break;
              }
            break;
            case btnUP:
              switch (selec_input){
                case InTIMER_minutos:
                  if (!tecla_travada & temp_timer.minutos < 59 ) temp_timer.minutos++;
                  tecla_travada = true;
                break;
                case InTIMER_horas:
                  if (!tecla_travada & temp_timer.horas < 23 ) temp_timer.horas++;
                  tecla_travada = true;
                break;
                case InTIMER_Ativo:
                  temp_timer.ativo = true;
                  tecla_travada = true;
                break;
              }
            break;
            case btnDOWN:
              switch (selec_input){
                case InTIMER_minutos:
                  if (!tecla_travada & temp_timer.minutos > 0 ) temp_timer.minutos--;
                  tecla_travada = true;
                break;
                case InTIMER_horas:
                  if (!tecla_travada & temp_timer.horas > 0 ) temp_timer.horas--;
                  tecla_travada = true;
                break;
                case InTIMER_Ativo:
                  temp_timer.ativo = false;
                  tecla_travada = true;
                break;
              }
            break;
            case btnRIGHT:
              if (!tecla_travada & selec_input < 4 ) selec_input++;
              tecla_travada = true;
            break;
            case btnLEFT:
              if (!tecla_travada & selec_input > 0 ) selec_input--;
              tecla_travada = true;
            break;
          }
        break;
        case Tela_Descanso:
          contador_bcklght_off = 0;
          seq_exibicao = 0;
          LcdBacklight = HIGH;
          troca_tela(Tela_normal);
        break;
        // Nova operação de tela aqui!
      }
    }
  }    
  
  // ** TASK 2: Clock 1s **    
  Task[2].Ciclo_ms = 500;
  if ( (Exec_Task(2, true)) || Exec_Prim_Loop ){
    clock_1s = !clock_1s;                  //Gera clock de 1 seg, 0,5s ON - 0,5s OFF
  }

  // ** TASK 3: Contagem tempo para backlight OFF **
  Task[3].Ciclo_ms = 1000;
  if ( (Exec_Task(3, true)) || Exec_Prim_Loop ){
    contador_bcklght_off++;
    if (contador_bcklght_off >= TmpBcklghtOFF){
    LcdBacklight = LOW;
    troca_tela(Tela_Descanso);
    }
  }
    
  // ** TASK 4: Verificação para Timer ON **
  Task[4].Ciclo_ms = 500;
  if ( (Exec_Task(4, hr_timer_ON.ativo)) || Exec_Prim_Loop ){
    // Verifica se Timer ON
    if ( dt_hr_atual.horas == hr_timer_ON.horas ){
      if ( dt_hr_atual.minutos == hr_timer_ON.minutos ){
        OutRelay = HIGH;
      }
    }
  }

  // ** TASK 5: Verificação para Timer OFF **
  Task[5].Ciclo_ms = 500;
  if ( (Exec_Task(5, hr_timer_OFF.ativo)) || Exec_Prim_Loop ){
    // Verifica se Timer OFF
    if ( dt_hr_atual.horas == hr_timer_OFF.horas ){
      if ( dt_hr_atual.minutos == hr_timer_OFF.minutos ){
        OutRelay = LOW;
      }
    }
  }

  // ** TASK 9: Atualiza I/Os **
  Task[9].Ciclo_ms = 0;
  if ( (Exec_Task(9, true)) || Exec_Prim_Loop ){
    digitalWrite(pin_OutRelay, OutRelay);
    digitalWrite(pin_lcdBacklight, LcdBacklight);
  }

  
  // **** TASKS DE EXIBIÇÃO NO LCD ****

  switch (tela_exibida){
    
    // ******* TELA NORMAL ********
    case Tela_normal:
    // ** TASK 10: Exibe hora no LCD **    
    Task[10].Ciclo_ms = 400;
    if (Exec_Task(10, true) || Exec_Prim_Loop){
      exibe_hora_LCD(0);                    // Exibe a hora atual na linha superior do LCD
    }
    // ** TASK 11: Exibe hora/status no LCD **  
    Task[11].Ciclo_ms = 1600;
    if (Exec_Task(11, true) || Exec_Prim_Loop){
      switch (seq_exibicao){
        case 0:
          lcd.setCursor(0,1);
          lcd.print("                ");
          exibe_data_LCD(1);                 // Exibe a data atual na linha inferior do LCD
          seq_exibicao++;
        break;
        case 1: // Status do Timer ON
          lcd.setCursor(0,1);
          lcd.print("Timer ON  ");
          lcd.setCursor(10,1);
          lcd.print(status_timer(hr_timer_ON.ativo));
          seq_exibicao++;
        break;
        case 2:
          if (hr_timer_ON.ativo){
            lcd.setCursor(10,1);
            lcd.print(str_zero_esq(hr_timer_ON.horas) + ":" + str_zero_esq(hr_timer_ON.minutos));
          }
          seq_exibicao++;
        break;
        case 3: // Status do Timer OFF
          lcd.setCursor(0,1);
          lcd.print("Timer OFF ");
          lcd.setCursor(10,1);
          lcd.print(status_timer(hr_timer_OFF.ativo));
          seq_exibicao++;
        break;
        case 4:
          if (hr_timer_OFF.ativo){
            lcd.setCursor(10,1);
            lcd.print(str_zero_esq(hr_timer_OFF.horas) + ":" + str_zero_esq(hr_timer_OFF.minutos));
          }
          seq_exibicao++;
        break;
        case 5:
          lcd.setCursor(0,1);
          if (RTC_OK){
            lcd.print("    Status OK   ");
          }else{
            lcd.print("  Falha relogio ");
          }
          seq_exibicao = 0;
        break;
      }
    }
    break;

    // ******* TELA MENU ********
    case Tela_menu:
    // ** TASK 12: Exibe menu **  
    Task[12].Ciclo_ms = 100;
    if (Exec_Task(12, true)){
      lcd.setCursor(1,0);
      lcd.print(Menu_Itens[selecao_menu-(selecao_menu%2)]);
      lcd.setCursor(1,1);
      lcd.print(Menu_Itens[selecao_menu-(selecao_menu%2)+1]);
      lcd.setCursor(0,(selecao_menu%2)-1);
      lcd.print(" ");
      lcd.setCursor(0,(selecao_menu%2));
      lcd.print(">");
      // Exibiçao de 2 itens por tela no menu:
      // indice do item escolhido (i) -> indice do primeiro item da tela (it) -> posição do cursor ">" (ic)
      // 0 -> 0 -> 0
      // 1 -> 0 -> 1
      // 2 -> 2 -> 0
      // 3 -> 2 -> 1
      // 4 -> 4 -> 0
      // 5 -> 4 -> 1
      // Calculo para rolagem de tela:
      // it = i - (i % numero_itens_por_tela)
      // ic = i % numero_itens_por_tela
    }
    break;
    
    // ******* TELA DE AJUSTE DO RELÓGIO ********
    //  "00:00:00    Data"
    //  " Cancela     OK "
    case Tela_AjRelogio:
    // ** TASK 13: Aj. Relógio **  
    Task[13].Ciclo_ms = 100;       
    if (Exec_Task(13, true)){
      lcd.setCursor(0,0);
      campo_pisca(selec_input, In_horas, str_zero_esq(dt_hr_mudar.horas), "  ");
      lcd.setCursor(2,0);
      lcd.print(":");
      lcd.setCursor(3,0);
      campo_pisca(selec_input, In_minutos, str_zero_esq(dt_hr_mudar.minutos), "  ");
      lcd.setCursor(5,0);
      lcd.print(":");
      lcd.setCursor(6,0);
      campo_pisca(selec_input, In_segundos, str_zero_esq(dt_hr_mudar.segundos), "  ");
      lcd.setCursor(11,0);
      campo_pisca(selec_input, In_Data, " ", ">");
      lcd.setCursor(12,0);
      lcd.print("Data");      
      lcd.setCursor(0,1);
      campo_pisca(selec_input, In_Cancel, " ", ">");
      lcd.setCursor(1,1);
      lcd.print("Cancela");
      lcd.setCursor(12,1);
      campo_pisca(selec_input, In_OK, " ", ">");
      lcd.setCursor(13,1);
      lcd.print("OK");
    }
    break;

    // ******* TELA DE AJUSTE DA DATA ********
    //  "00/00/00   TER  "
    //  " Canc  OK  Relog"
    case Tela_AjData:
    // ** TASK 14: Aj. Data **  
    Task[14].Ciclo_ms = 100;       
    if (Exec_Task(14, true)){
      lcd.setCursor(0,0);
      campo_pisca(selec_input, In_dia, str_zero_esq(dt_hr_mudar.dia), "  ");
      lcd.setCursor(2,0);
      lcd.print("/");
      lcd.setCursor(3,0);
      campo_pisca(selec_input, In_mes, str_zero_esq(dt_hr_mudar.mes), "  ");
      lcd.setCursor(5,0);
      lcd.print("/");
      lcd.setCursor(6,0);
      campo_pisca(selec_input, In_ano, str_zero_esq(dt_hr_mudar.ano), "  ");
      lcd.setCursor(11,0);
      campo_pisca(selec_input, In_dia_semana, dia_semana_str(dt_hr_mudar.dia_semana), "   ");
      lcd.setCursor(0,1);
      campo_pisca(selec_input, In_Cancel, " ", ">");
      lcd.setCursor(1,1);
      lcd.print("Canc");
      lcd.setCursor(6,1);
      campo_pisca(selec_input, In_OK, " ", ">");
      lcd.setCursor(7,1);
      lcd.print("OK");
      lcd.setCursor(10,1);
      campo_pisca(selec_input, In_Relogio, " ", ">");
      lcd.setCursor(11,1);
      lcd.print("Relog");
    }
    break;
    
    // ******* TELA DE AJUSTE TIMER ON ********
    //  "00:00 ON  Ativo "
    //  "00:00 ON  Canc  "
    //  " Cancela     OK "
    case Tela_TimerON:
    // ** TASK 15: Aj. Timer ON **  
    Task[15].Ciclo_ms = 100;       
    if (Exec_Task(15, true)){
      lcd.setCursor(0,0);
      campo_pisca(selec_input, InTIMER_horas, str_zero_esq(temp_timer.horas), "  ");
      lcd.setCursor(2,0);
      lcd.print(":");
      lcd.setCursor(3,0);
      campo_pisca(selec_input, InTIMER_minutos, str_zero_esq(temp_timer.minutos), "  ");
      lcd.setCursor(6,0);
      lcd.print("ON");
      lcd.setCursor(10,0);
      campo_pisca(selec_input, InTIMER_Ativo, status_timer(temp_timer.ativo), "     ");
      lcd.setCursor(0,1);
      campo_pisca(selec_input, InTIMER_Cancel, " ", ">");
      lcd.setCursor(1,1);
      lcd.print("Cancela");
      lcd.setCursor(12,1);
      campo_pisca(selec_input, InTIMER_OK, " ", ">");
      lcd.setCursor(13,1);
      lcd.print("OK");
    }
    break;

    // ******* TELA DE AJUSTE TIMER OFF ********
    //  "00:00 OFF Ativo "
    //  "00:00 OFF Canc  "
    //  " Cancela     OK "
    case Tela_TimerOFF:
    // ** TASK 16: Aj. Timer OFF **  
    Task[16].Ciclo_ms = 100;       
    if (Exec_Task(16, true)){
      lcd.setCursor(0,0);
      campo_pisca(selec_input, InTIMER_horas, str_zero_esq(temp_timer.horas), "  ");      
      lcd.setCursor(2,0);
      lcd.print(":");
      lcd.setCursor(3,0);
      campo_pisca(selec_input, InTIMER_minutos, str_zero_esq(temp_timer.minutos), "  ");
      lcd.setCursor(6,0);
      lcd.print("OFF");
      lcd.setCursor(10,0);
      campo_pisca(selec_input, InTIMER_Ativo, status_timer(temp_timer.ativo), "     ");
      lcd.setCursor(0,1);
      campo_pisca(selec_input, InTIMER_Cancel, " ", ">");
      lcd.setCursor(1,1);
      lcd.print("Cancela");
      lcd.setCursor(12,1);
      campo_pisca(selec_input, InTIMER_OK, " ", ">");
      lcd.setCursor(13,1);
      lcd.print("OK");
    }
    break;

    // ******* TELA DE STATUS DO RELÓGIO ********
    case Tela_StatusRTC:
    // ** TASK 17: Status Relógio **  
    Task[17].Ciclo_ms = 100;
    if (Exec_Task(17, true)){
      lcd.setCursor(0,0);
      lcd.print("Bateria: ");
      lcd.setCursor(9,0);
//      lcd.print(dtostrf((float)analogRead(AD_VBatRTC)*0.0048, 1, 2, char_temp));
      lcd.setCursor(13,0);
      lcd.print(" V");
      lcd.setCursor(0,1);
      lcd.print("Select: Voltar");

    // ******* TELA DE DESCANSO: Tela_Descanso  ********
    //       nada aparece, sem task de exibição

    }
    break;
  }

  
  // Cancela flag da execução em primeiro loop
  Exec_Prim_Loop = false;
}
