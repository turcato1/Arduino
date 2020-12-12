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
#define AD_3             3 //Livre
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

