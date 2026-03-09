/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <dispcolor.h>
#include <gpio.h>
#include <hvac.h>
#include <htp.h>
#include <therm3.h>
#include <power.h>
#include <horse_frames.h>
#include "voltage_meter.h"
#include "tachometer.h"
#include "rainbow.h"
#include "mono.h"
#include "humidity_meter.h"
#include "bodmer_spiral.h"
#include "ampelmann.h"
#include "three_orbiting_rotating_yinyang.h"
#include "bodmer_single_yinyang.h"
#include "sht21_mode.h"
#include "benchmark_mode.h"
#include "watchface.h"
#include "rtc_print_mode.h"
#include "smooth_clock.h"
#include "boat_gauges.h"
#include <clock.h>
#include <compas.h>
#include <textfading.h>
#include <thermostat.h>
#include <watchface.h>
#include "font.h"
#include "w25qxx.h"
#include "gif_mode.h"

//#include "img/gauge1.h"
//#include "img/gauge2.h"
//#include "img/gauge3.h"
//#include "img/gauge4.h"
//#include "img/gauge5.h"
//#include "img/gauge6.h"



/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    EDIT_NONE = 0,
    EDIT_HH,
    EDIT_MM,
    EDIT_SS,
} EditField;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */


// Пин, к которому подключена кнопка
//#define Button_Port		GPIOA
//#define Button_Pin 		(1 << 0)
#define Button_Port BUTTON_K0_GPIO_Port
#define Button_Pin  BUTTON_K0_Pin
uint8_t Button_OldState;

// Режимы работы
#define ModesNum 	37

#define ModeBlack	0
#define ModeRed		1
#define ModeGreen	2
#define ModeBlue	3
#define ModeWhite	4
#define ModeText	5
#define ModeClock1	6
#define ModeClock2	7
#define ModeClock3	8
#define ModeClock4	9
#define ModeClock5  10
#define ModeCompas1	11	// Компас
#define ModeCompas2	12	// Компас
#define ModeHTP		13	// Температура-влажность-давление
#define ModeTherm1	14	// Термостат теплого пола
#define ModeTherm2	15	// Термостат теплого пола
#define ModeTherm3  16    // Ring meter
#define ModeHvac	17	// Кондиционер
#define ModeHvac2	18	// Что нибудь с иммитацией ЖК
#define ModePower   19  //Мощность
#define ModeHorse   20   // Новый режим: Muybridge horse
#define ModeVoltage_meter 21
#define ModeTachometer    22
#define ModeRainbow       23
#define ModeMono          24
#define Modehumidity_meter 25
#define ModeBodmer_spiral 26
#define Modeampelmann_sprite_nn 27
#define Modethree_orbiting_rotating_yinyang   28
#define ModeBodmer_16_bit_single_yin_yang   29
#define ModeSHT21    30
#define ModeBenchmark   31
#define ModeWatchface  32
#define ModeRTCPrint 33
#define ModeSmoothClock 34
#define ModeBoatGauges 35
#define ModeGif 36



// Долгое нажатие K1 для входа/выхода из настройки времени
#define K1_LONGPRESS_MS 800u

#define RTC_BKP_MAGIC 0x32F2u

#define ARR_LEN(a) (uint8_t)(sizeof(a) / sizeof((a)[0]))


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim11;

/* USER CODE BEGIN PV */
// Текущий режим работы
static uint8_t Mode = ModeBlack;

// time edit
static EditField edit = EDIT_NONE;
static uint8_t ed_hh = 0, ed_mm = 0, ed_ss = 0;

// buttons (PULLUP: 1 released, 0 pressed)
static uint8_t k0_prev = 1; // K0 PE4
static uint8_t k1_prev = 1; // K1 PE3

// K1 long-press indicator logic
static uint8_t  k1_armed = 0;
static uint8_t  k1_led_shown = 0;
static uint32_t k1_press_tick = 0;

// blink for active field
static uint8_t blink = 1;
static uint32_t blink_tick = 0;

static uint32_t k0_last_press = 0;
static uint32_t k1_last_release = 0;

uint8_t brightness = 100;        // текущая яркость 0..100
//static const uint8_t br_steps[] = {10, 25, 40, 60, 80, 100};
//static uint8_t br_idx = 5;              // индекс в br_steps (100)

static const uint8_t br_base[]  = {10, 25, 40, 60, 80, 100};   // обычные режимы
static const uint8_t br_clock[] = {20, 40, 60, 80, 100};       // часы
static const uint8_t br_hvac[]  = {10, 30, 50, 70, 90};        // HVAC

static uint8_t br_base_idx  = 5;  // старт 100
static uint8_t br_clock_idx = 4;  // старт 100
static uint8_t br_hvac_idx  = 3;  // старт 70

static uint8_t hvac_sel = 0;// 0=setpoint, 1=mode, 2=fan

#define K0_LONGPRESS_MS 700u

static uint8_t  k0_armed = 0;
static uint32_t k0_press_tick = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM11_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// ======================= ModeHorse =======================

static uint8_t horse_inited = 0;
static uint8_t horse_idx = 0;
static uint32_t horse_tick = 0;

static const uint16_t HORSE_FG = GREY; // GREEN
static const uint16_t HORSE_BG = BLACK; // BLACK

static const int16_t horse_x = 48;
static const int16_t horse_y = 72;
static const uint32_t horse_frame_ms = 50;

static const uint8_t* const horse_frames[10] = {
  horse_01, horse_02, horse_03, horse_04, horse_05,
  horse_06, horse_07, horse_08, horse_09, horse_10
};

static void Horse_DrawBitmap1bpp_MSB(int16_t x0, int16_t y0, const uint8_t *bmp, int16_t w, int16_t h, uint16_t fg, uint16_t bg)
{
  const int16_t bpr = (int16_t)((w + 7) / 8);

  for (int16_t yy = 0; yy < h; yy++) {
    const uint8_t* row = bmp + (yy * bpr);
    for (int16_t xx = 0; xx < w; xx++) {
      uint8_t b = row[xx >> 3];
      uint8_t mask = (uint8_t)(0x80u >> (xx & 7));
      dispcolor_DrawPixel((int16_t)(x0 + xx), (int16_t)(y0 + yy), (b & mask) ? fg : bg);
    }
  }
}

static void ModeHorse_Reset(void)
{
  horse_inited = 1;
  horse_idx = 0;
  horse_tick = HAL_GetTick();

  dispcolor_FillScreen(BLACK);

  dispcolor_DrawString(23, 175, FONTID_6X8M, "Muybridge - Sallie Gardner (1878)", WHITE);
  dispcolor_DrawLine(10, 170, 230, 170, YELLOW);
  dispcolor_DrawCircle(120, 120, 117, MAGENTA, 0);

  dispcolor_Update();
}

static void ModeHorse_Draw(void)
{
  if (!horse_inited) ModeHorse_Reset();

  uint32_t now = HAL_GetTick();
  if ((now - horse_tick) < horse_frame_ms) return;
  horse_tick = now;

  Horse_DrawBitmap1bpp_MSB(horse_x, horse_y, horse_frames[horse_idx], HORSE_W, HORSE_H, HORSE_FG, HORSE_BG);
  dispcolor_Update();

  horse_idx++;
  if (horse_idx >= 10) horse_idx = 0;
}



// Процедура переключения текущего режима на следующий
static inline uint8_t BTN_K0(void) { return (uint8_t)HAL_GPIO_ReadPin(BUTTON_K0_GPIO_Port, BUTTON_K0_Pin); } // PE4
static inline uint8_t BTN_K1(void) { return (uint8_t)HAL_GPIO_ReadPin(BUTTON_K1_GPIO_Port, BUTTON_K1_Pin); } // PE3

// LED1: VCC -> LED -> PA6, active LOW
static inline void LED1_On(void)  { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); }
static inline void LED1_Off(void) { HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); }

static uint8_t IsClockMode(uint8_t m)
{
    return (m == ModeClock1 || m == ModeClock2 || m == ModeClock3 ||
            m == ModeClock4 || m == ModeClock5);
}

static uint8_t IsHvacMode(uint8_t m)
{
    return (m == ModeHvac || m == ModeHvac2);
}

static void ApplyBrightnessForMode(uint8_t m)
{
    uint8_t br;

    if (m == ModeBlack) {
        br = br_base[0];
    } else if (IsClockMode(m)) {
        br = br_clock[br_clock_idx];
    } else if (IsHvacMode(m)) {
        br = br_hvac[br_hvac_idx];
    } else {
        br = br_base[br_base_idx];
    }

    dispcolor_SetBrightness(br);
}

static void Brightness_NextForCurrentMode(void)
{
    if (Mode == ModeBlack) return;

    if (IsClockMode(Mode)) {
        br_clock_idx++;
        if (br_clock_idx >= ARR_LEN(br_clock)) br_clock_idx = 0;
    } else if (IsHvacMode(Mode)) {
        br_hvac_idx++;
        if (br_hvac_idx >= ARR_LEN(br_hvac)) br_hvac_idx = 0;
    } else {
        br_base_idx++;
        if (br_base_idx >= ARR_LEN(br_base)) br_base_idx = 0;
    }

    ApplyBrightnessForMode(Mode);
}

static void SwitchMode(void)
{
    if (++Mode >= ModesNum) Mode = 0;

    if (IsHvacMode(Mode)) { hvac_sel = 0;}

    if (Mode == ModeClock5) { Clock5_Reset();}

    if (Mode == ModeHTP) { HTP_ResetView();}

    if (Mode == ModePower) Power_ResetView();

    if (Mode == ModeHorse) { horse_inited = 0; }

    if (Mode == ModeVoltage_meter) VoltageMeter_ResetView();

    if (Mode == ModeTachometer) Tachometer_ResetView();

    if (Mode == ModeRainbow) Rainbow_ResetView();

    if (Mode == ModeMono) Mono_ResetView();

    if (Mode == Modehumidity_meter) HumidityMeter_ResetView();

    if (Mode == ModeBodmer_spiral) BodmerSpiral_ResetView();

    if (Mode == Modeampelmann_sprite_nn) Ampelmann_ResetView();

    if (Mode == Modethree_orbiting_rotating_yinyang) ThreeYY_ResetView();

    if (Mode == ModeBodmer_16_bit_single_yin_yang) BodmerSingleYY_ResetView();

    if (Mode == ModeSHT21) SHT21Mode_ResetView();

    if (Mode == ModeBenchmark) Benchmark_ResetView();

    if (Mode == ModeWatchface) Watchface_Reset();

    if (Mode == ModeRTCPrint) RTCPrint_ResetView();

    if (Mode == ModeSmoothClock) SmoothClock_ResetView();

    if (Mode == ModeBoatGauges) BoatGauges_ResetView();

    if (Mode == ModeGif) GifMode_Reset();


    ApplyBrightnessForMode(Mode);
}

//static void Brightness_Next(void)
//{
//    br_idx++;
//    if (br_idx >= (sizeof(br_steps) / sizeof(br_steps[0]))) br_idx = 0;
//    dispcolor_SetBrightness(br_steps[br_idx]);
//}

static void RTC_ApplyTime(uint8_t hh, uint8_t mm, uint8_t ss)
{
    RTC_TimeTypeDef t = {0};
    t.Hours = hh;
    t.Minutes = mm;
    t.Seconds = ss;
    t.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    t.StoreOperation = RTC_STOREOPERATION_RESET;
    HAL_RTC_SetTime(&hrtc, &t, RTC_FORMAT_BIN);
}

static void Blink_Update(void)
{
    if (HAL_GetTick() - blink_tick > 400u) {
        blink_tick = HAL_GetTick();
        blink ^= 1u;
    }
}

//Предложение по UX (без K0+K1)
//В HVAC:

//K0 короткое: менять уставку (циклично по 0.5°C)
//K1 короткое: переключать “что редактируем” (MODE / FAN)
//K0 удержание (например >600 мс): инкремент выбранного параметра быстрее / или уменьшение
//(опционально) K1 двойной клик: переключение POWER ON/OFF
//Самый простой: K1 короткое = переключить MODE↔FAN, K0 короткое = изменить выбранное.

static void Buttons_Process(void)
{
    uint8_t k0_now = (uint8_t)HAL_GPIO_ReadPin(BUTTON_K0_GPIO_Port, BUTTON_K0_Pin); // PE4, PULLUP (1=up,0=down)
    uint8_t k1_now = (uint8_t)HAL_GPIO_ReadPin(BUTTON_K1_GPIO_Port, BUTTON_K1_Pin); // PE3, PULLUP

    uint32_t now = HAL_GetTick();

    uint8_t isHvac   = (uint8_t)((Mode == ModeHvac) || (Mode == ModeHvac2));
    uint8_t isTherm3 = (uint8_t)(Mode == ModeTherm3);

    // ===================== K1 (PE3) =====================

    // press
    if (k1_now == 0 && k1_prev == 1) {
        k1_press_tick = now;
        k1_armed = 1;
        k1_led_shown = 0;
        LED1_Off();
    }

    // hold -> after 800ms show LED as "release now"
    if (k1_armed && k1_now == 0 && !k1_led_shown) {
        if ((now - k1_press_tick) > K1_LONGPRESS_MS) {
            LED1_On();
            k1_led_shown = 1;
        }
    }

    // release
    if (k1_now == 1 && k1_prev == 0) {

        // антидребезг отпускания
        if (now - k1_last_release >= 150u) {
            k1_last_release = now;

            uint32_t dt = now - k1_press_tick;
            k1_armed = 0;

            LED1_Off();

            if (dt > K1_LONGPRESS_MS) {
                // long press: enter/exit edit mode
                if (edit == EDIT_NONE) {
                    RTC_TimeTypeDef tnow;
                    RTC_DateTypeDef dnow;
                    HAL_RTC_GetTime(&hrtc, &tnow, RTC_FORMAT_BIN);
                    HAL_RTC_GetDate(&hrtc, &dnow, RTC_FORMAT_BIN);

                    ed_hh = tnow.Hours;
                    ed_mm = tnow.Minutes;
                    ed_ss = tnow.Seconds;

                    edit = EDIT_HH;
                } else {
                    RTC_ApplyTime(ed_hh, ed_mm, ed_ss);
                    edit = EDIT_NONE;
                }
            } else {
                // short press
                if (edit != EDIT_NONE) {
                    // в настройке: переключаем поле HH/MM/SS
                    if (edit == EDIT_HH)      edit = EDIT_MM;
                    else if (edit == EDIT_MM) edit = EDIT_SS;
                    else                      edit = EDIT_HH;
                } else {
                    // не в настройке
                    if (isHvac) {
                        hvac_sel = (uint8_t)((hvac_sel + 1u) % 3u);
                    } else if (isTherm3) {
                        Therm3_OnK1();                 // next color scheme
                    } else {
                        Brightness_NextForCurrentMode();
                    }
                }
            }
        }
    }

    // ===================== K0 (PE4) =====================

    // press: arm for long-press detection
    if (k0_now == 0 && k0_prev == 1) {
        // антидребезг нажатия K0
        if (now - k0_last_press >= 150u) {
            k0_last_press = now;
            k0_press_tick = now;
            k0_armed = 1;
        }
    }

    // release: decide short/long
    if (k0_now == 1 && k0_prev == 0) {
        if (k0_armed) {
            k0_armed = 0;

            uint32_t dt0 = now - k0_press_tick;

            if (dt0 > K0_LONGPRESS_MS) {
                // long press K0: next main mode (disabled during time edit)
                if (edit == EDIT_NONE) {
                    SwitchMode();
                }
            } else {
                // short press K0
                if (edit != EDIT_NONE) {
                    // в настройке: увеличиваем активное поле
                    if (edit == EDIT_HH)      ed_hh = (uint8_t)((ed_hh + 1u) % 24u);
                    else if (edit == EDIT_MM) ed_mm = (uint8_t)((ed_mm + 1u) % 60u);
                    else                      ed_ss = (uint8_t)((ed_ss + 1u) % 60u);
                } else {
                    // не в настройке
                    if (isHvac) {
                        if (hvac_sel == 0) {
                            Hvac_OnK0();      // +0.5C setpoint
                        } else if (hvac_sel == 1) {
                            Hvac_OnK1();      // next mode
                        } else {
                            Hvac_OnCombo();   // next fan
                        }
                    } else if (isTherm3) {
                        Therm3_OnK0();        // +0.5°C setpoint (Therm3)
                    } else if (Mode == ModeBoatGauges) {
                        BoatGauges_OnK0();
                    } else {
                        SwitchMode();
                    }
                }
            }
        }
    }

    k0_prev = k0_now;
    k1_prev = k1_now;
}

static void GetTimeForClock(uint8_t *hh, uint8_t *mm, uint8_t *ss)
{
    if (edit == EDIT_NONE) {
        RTC_TimeTypeDef tnow;
        RTC_DateTypeDef dnow;
        HAL_RTC_GetTime(&hrtc, &tnow, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &dnow, RTC_FORMAT_BIN); // обязательно после GetTime
        if (tnow.Hours >= 24) tnow.Hours = 0;
        *hh = tnow.Hours;
        *mm = tnow.Minutes;
        *ss = tnow.Seconds;
    } else {
        *hh = ed_hh;
        *mm = ed_mm;
        *ss = ed_ss;
    }

    uint8_t editField = 0;
    if (edit == EDIT_HH)      editField = 1;
    else if (edit == EDIT_MM) editField = 2;
    else if (edit == EDIT_SS) editField = 3;

    Clock_SetEdit(editField, blink);
}






/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

//  LED1_On();
//  HAL_Delay(200);
//  LED1_Off();
//  HAL_Delay(200);

  MX_DMA_Init();
  MX_SPI1_Init();

//  HAL_PWR_EnableBkUpAccess();
//  __HAL_RCC_BACKUPRESET_FORCE();
//  __HAL_RCC_BACKUPRESET_RELEASE();один раз прошивается для очистки RTC
  MX_RTC_Init();
  if (HAL_RTC_WaitForSynchro(&hrtc) != HAL_OK) {
      Error_Handler();
  }

  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */

  	HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
  __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, __HAL_TIM_GET_AUTORELOAD(&htim11)); // 100%

  LED1_Off(); // по умолчанию LED не горит (горит только в режиме настройки)

//	gpio_SetGPIOmode_In(Button_Port, Button_Pin, gpio_PullUp);

//	RTC_Init();


  //// Вставьте это перед main() Запись поочерёдно 6 файлов
  //Порядок действий:
  //
  //Откройте main.c.
  //Вверху подключите только одну картинку. Например: #include "img/gauge1.h".
  //Внутри main(), после W25qxx_Init(), вставьте код записи (см. ниже).
  //В коде записи укажите Адрес и Имя массива для текущей картинки (см. таблицу).
  //Прошейте плату. Дождитесь, пока светодиод моргнет 5 раз (значит записалось).
  //Повторите это для gauge2, gauge3... gauge6.
  //Таблица адресов:
  //
  //Файл (Include)	Адрес (hex)	Массив данных
  //gauge1.h	0x000000	gauge1
  //gauge2.h	0x020000	gauge2
  //gauge3.h	0x040000	gauge3
  //gauge4.h	0x060000	gauge4
  //gauge5.h	0x080000	gauge5
  //gauge6.h	0x0A0000	gauge6

  // =============================================================
    // БЛОК ЗАПИСИ КАРТИНОК ВО ВНЕШНЮЮ ФЛЕШКУ (SOFT SPI)
    // =============================================================


  // === ТЕСТ ЗАПИСИ И ВЕРИФИКАЦИИ ===
//    const uint8_t* SRC_DATA = (const uint8_t*)gauge1; // Исходный массив
//    uint32_t       START_ADDR = 0x000000;             // Адрес во флешке
//    uint32_t       DATA_SIZE  = 115200;               // Размер (240*240*2)
//    // =================================
//
//    // --- 1. НАСТРОЙКА SOFT SPI (Прямое управление) ---
//    // Гарантируем, что дисплей отключен!
//    HAL_GPIO_WritePin(GPIOA, TFT_CS_Pin, GPIO_PIN_SET);
//
//    #define SCS_L  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
//    #define SCS_H  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
//    #define SCLK_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET)
//    #define SCLK_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET)
//    #define SMOSI_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)
//    #define SMOSI_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)
//    #define SMISO  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)
//
//    // Функция обмена байтом (с задержками)
//    uint8_t SPI_Transfer(uint8_t out) {
//        uint8_t in = 0;
//        for(int i=0; i<8; i++) {
//            if(out & 0x80) SMOSI_H; else SMOSI_L;
//            out <<= 1;
//            SCLK_L; for(volatile int k=0;k<50;k++); // Delay
//            SCLK_H; for(volatile int k=0;k<50;k++); // Delay
//            in <<= 1; if(SMISO) in |= 1;
//        }
//        SCLK_L;
//        return in;
//    }
//
//    void SPI_Wren() { SCS_L; SPI_Transfer(0x06); SCS_H; HAL_Delay(1); }
//    void SPI_WaitBusy() {
//        SCS_L; SPI_Transfer(0x05);
//        while(SPI_Transfer(0xFF) & 0x01);
//        SCS_H;
//    }
//
//    // --- 2. СНЯТИЕ ЗАЩИТЫ ---
//    SPI_Wren();
//    SCS_L; SPI_Transfer(0x01); SPI_Transfer(0x00); SCS_H;
//    SPI_WaitBusy();
//
//    // --- 3. СТИРАНИЕ (2 блока по 64КБ) ---
//    LED1_On(); // Горит = Занят
//    uint32_t addr = START_ADDR;
//    for(int k=0; k<2; k++) {
//        SPI_Wren();
//        SCS_L; SPI_Transfer(0xD8);
//        SPI_Transfer((addr>>16)&0xFF); SPI_Transfer((addr>>8)&0xFF); SPI_Transfer(addr&0xFF);
//        SCS_H;
//        SPI_WaitBusy();
//        addr += 0x10000;
//    }
//
//    // --- 4. ЗАПИСЬ ---
//    uint32_t curr = START_ADDR;
//    uint32_t rem = DATA_SIZE;
//    const uint8_t* p = SRC_DATA;
//
//    while(rem > 0) {
//        uint32_t chunk = (rem > 256) ? 256 : rem;
//        SPI_Wren();
//        SCS_L;
//        SPI_Transfer(0x02); // Page Program
//        SPI_Transfer((curr>>16)&0xFF); SPI_Transfer((curr>>8)&0xFF); SPI_Transfer(curr&0xFF);
//        for(uint32_t i=0; i<chunk; i++) SPI_Transfer(*p++);
//        SCS_H;
//        SPI_WaitBusy();
//        curr += chunk; rem -= chunk;
//    }
//    LED1_Off(); // Запись окончена
//
//    // --- 5. ВЕРИФИКАЦИЯ (СРАВНЕНИЕ) ---
//    // Читаем всю картинку обратно и сравниваем с массивом
//    int verify_error = 0;
//    p = SRC_DATA; // Сброс указателя на начало массива
//
//    SCS_L;
//    SPI_Transfer(0x03); // Read
//    SPI_Transfer((START_ADDR>>16)&0xFF);
//    SPI_Transfer((START_ADDR>>8)&0xFF);
//    SPI_Transfer(START_ADDR&0xFF);
//
//    for(uint32_t i=0; i<DATA_SIZE; i++) {
//        uint8_t val = SPI_Transfer(0x00);
//        if (val != p[i]) {
//            verify_error = 1; // ОШИБКА! Не совпало!
//            break; // Прерываем проверку
//        }
//    }
//    SCS_H;
//
//    // --- 6. ИНДИКАЦИЯ РЕЗУЛЬТАТА ---
//    if (verify_error) {
//        // ОШИБКА: Быстрое истеричное мигание
//        while(1) {
//            LED1_On(); HAL_Delay(50);
//            LED1_Off(); HAL_Delay(50);
//        }
//    } else {
//        // УСПЕХ: Спокойное мигание
//        while(1) {
//            LED1_On(); HAL_Delay(500);
//            LED1_Off(); HAL_Delay(500);
//        }
//    }






  // === НАСТРОЙКИ ===
//  const uint8_t* DATA = (const uint8_t*)gauge1;
//  uint32_t       ADDR = 0x000000;
//  uint32_t       SIZE = 115200;
//  // =================
//
//  // МАКРОСЫ SOFT SPI
//  #define CS_L  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
//  #define CS_H  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)
//  #define CLK_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET)
//  #define CLK_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET)
//  #define MOSI_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)
//  #define MOSI_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)
//  #define MISO  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)
//
//  // Функция обмена
//  uint8_t S_Rx(void) {
//      uint8_t in = 0;
//      for(int i=0; i<8; i++) {
//          CLK_L; for(volatile int k=0;k<50;k++);
//          CLK_H; for(volatile int k=0;k<50;k++);
//          in <<= 1; if(MISO) in |= 1;
//      }
//      CLK_L;
//      return in;
//  }
//
//  // Отправка команды
//  void S_Tx(uint8_t b) {
//      for(int i=0; i<8; i++) {
//          if(b & 0x80) MOSI_H; else MOSI_L;
//          b <<= 1;
//          CLK_L; for(volatile int k=0;k<50;k++);
//          CLK_H; for(volatile int k=0;k<50;k++);
//      }
//      CLK_L;
//  }
//
//  // --- ВЕРИФИКАЦИЯ ---
//  int error = 0;
//  const uint8_t* p = DATA;
//
//  CS_L;
//  S_Tx(0x03); // Read Command
//  S_Tx((ADDR>>16)&0xFF);
//  S_Tx((ADDR>>8)&0xFF);
//  S_Tx(ADDR&0xFF);
//
//  for(uint32_t i=0; i<SIZE; i++) {
//      uint8_t val = S_Rx(); // Читаем байт
//      if (val != p[i]) {
//          error = 1;
//          break; // Ошибка!
//      }
//  }
//  CS_H;
//
//  // --- РЕЗУЛЬТАТ ---
//  if (error) {
//      // ОШИБКА: Быстрое мигание
//      while(1) { LED1_On(); HAL_Delay(50); LED1_Off(); HAL_Delay(50); }
//  } else {
//      // УСПЕХ: Медленное мигание
//      while(1) { LED1_On(); HAL_Delay(500); LED1_Off(); HAL_Delay(500); }
//  }











	// Начальное время для часов
	RTC_TimeTypeDef timeNow;
	RTC_DateTypeDef dateNow;
//	timeNow.Hours = 19;
//	timeNow.Minutes = 23;
//	timeNow.Seconds = 45;
//	RTC_SetTime(&timeNow);
//
//	HAL_Delay(1);
	HAL_RTC_GetTime(&hrtc, &timeNow, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &dateNow, RTC_FORMAT_BIN);
//	GC9A01A_Simple_Init(240, 240);


	// Инициализация рандомайзера
	srand(timeNow.SubSeconds);

	// Инициализация дисплея
	dispcolor_Init(240, 240);

	ApplyBrightnessForMode(Mode);

	Hvac_Init();
	HTP_Init();
	Therm3_Init();
	Power_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//		// Если была нажата кнопка - переключаемся на следующий режим
//		uint8_t ButtonState = HAL_GPIO_ReadPin(Button_Port, Button_Pin);
//		if (!ButtonState && Button_OldState) {
//			SwitchMode();
//		}
//		Button_OldState = ButtonState;


	  	    Blink_Update();
	        Buttons_Process();

	        switch (Mode)
	        {
	        case ModeBlack:
	            dispcolor_FillScreen(GREY);
	            dispcolor_Update();
	            HAL_Delay(100);
	            break;

	        case ModeRed:
	            dispcolor_FillScreen(RED);
	            dispcolor_Update();
	            HAL_Delay(100);
	            break;

	        case ModeGreen:
	            dispcolor_FillScreen(GREEN);
	            dispcolor_Update();
	            HAL_Delay(100);
	            break;

	        case ModeBlue:
	            dispcolor_FillScreen(BLUE);
	            dispcolor_Update();
	            HAL_Delay(100);
	            break;

	        case ModeWhite:
	            dispcolor_FillScreen(WHITE);
	            dispcolor_Update();
	            HAL_Delay(100);
	            break;

	        case ModeClock1: {
	            uint8_t hh, mm, ss;
	            GetTimeForClock(&hh, &mm, &ss);
	            DrawClock(hh, mm, ss, 0, 0);
	            break;
	        }

	        case ModeClock2: {
	            uint8_t hh, mm, ss;
	            GetTimeForClock(&hh, &mm, &ss);
	            DrawClock(hh, mm, ss, 0, 1);
	            break;
	        }

	        case ModeClock3: {
	            uint8_t hh, mm, ss;
	            GetTimeForClock(&hh, &mm, &ss);
	            DrawClock(hh, mm, ss, 1, 0);
	            break;
	        }

	        case ModeClock4: {
	            uint8_t hh, mm, ss;
	            GetTimeForClock(&hh, &mm, &ss);
	            DrawClock(hh, mm, ss, 1, 1);
	            break;
	        }

	        case ModeClock5: {
	            uint8_t hh, mm, ss;
	            GetTimeForClock(&hh, &mm, &ss);
	            DrawClock5(hh, mm, ss);
	            break;
	        }

	        case ModeCompas1:
	            DrawCompas(0);
	            break;

	        case ModeCompas2:
	            DrawCompas(1);
	            break;

	        case ModeTherm1:
	            Test_Therm(0);
	            break;

	        case ModeTherm2:
	            Test_Therm(1);
	            break;

	        case ModeTherm3:
	            Therm3_Draw(0);     // 0=dark, 1=light
	            HAL_Delay(37);
	            break;

	        case ModeText:
	            Test_TextFading("Электроника в объективе", 25, 110);
	            SwitchMode();
	            break;

	        case ModeHTP:
	        	HTP_Draw_Round(0);// or (1)
	        	    HAL_Delay(37);
	        	break;

	        case ModeHvac:
	        	Hvac_Draw_Round(0);      // 0=dark theme, 1=light theme
	        	    HAL_Delay(37);
	        	break;

	        case ModeHvac2:
	        	Hvac_Draw_Round(1);
	        	    HAL_Delay(37);
	        	break;

	        case ModePower:
	            Power_Draw(0);      // 0=dark, 1=light
	            HAL_Delay(37);
	            break;

	        case ModeHorse:
	            ModeHorse_Draw();
	            // небольшой sleep можно, но не обязательно, т.к. у нас есть тайминг внутри
	            HAL_Delay(1);
	            break;

	        case ModeVoltage_meter:
	        	VoltageMeter_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeTachometer:
	            Tachometer_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeRainbow:
	            Rainbow_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeMono:
	            Mono_Draw();
	            HAL_Delay(1);
	            break;

	        case Modehumidity_meter:
	            HumidityMeter_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeBodmer_spiral:
	            BodmerSpiral_Draw();
	            HAL_Delay(1);
	            break;

	        case Modeampelmann_sprite_nn:
	            Ampelmann_Draw();
	            HAL_Delay(1);
	            break;

	        case Modethree_orbiting_rotating_yinyang:
	            ThreeYY_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeBodmer_16_bit_single_yin_yang:
	            BodmerSingleYY_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeSHT21:
	            SHT21Mode_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeBenchmark:
	            Benchmark_Draw();   // возвращаемое значение можно игнорировать
	            HAL_Delay(1);
	            break;

	        case ModeWatchface: {
	            uint8_t hh, mm, ss;
	            GetTimeRTC(&hh, &mm, &ss);
//	            if (hh >= 24) hh = 0;
	            Watchface_Draw(hh, mm, ss);
	            HAL_Delay(1);
	            break;
	        }

	        case ModeRTCPrint:
	            RTCPrint_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeSmoothClock:
	            SmoothClock_Draw();
	            HAL_Delay(1);
	            break;

	        case ModeBoatGauges:
	            BoatGauges_Draw();
	            HAL_Delay(10);
	            break;

	        case ModeGif:
	            GifMode_Draw();
	            // Задержка не нужна, так как внутри GifMode_Draw есть цикл воспроизведения
	            break;

	        default:
	            HAL_Delay(50);
	            break;
	        }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if (HAL_RTC_Init(&hrtc) != HAL_OK) {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
//  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0);// разкоментировать, чтобы настроить время
  // Если RTC уже был настроен раньше (есть VBAT) — НЕ сбрасываем время/дату
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) == RTC_BKP_MAGIC) {
    goto rtc_done;
  }

  // Первый запуск: выставляем дефолтные дату/время один раз
  // Лучше BIN, но можно оставить BCD — главное единообразие.
  sTime.Hours = 19;
  sTime.Minutes = 48;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
    Error_Handler();
  }

  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month   = RTC_MONTH_JANUARY;
  sDate.Date    = 5;
  sDate.Year    = 26;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
    Error_Handler();
  }

  // Ставим метку "RTC настроен"
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_BKP_MAGIC);

  /* USER CODE END Check_RTC_BKUP */

rtc_done:

  if (HAL_RTCEx_SetCalibrationOutPut(&hrtc, RTC_CALIBOUTPUT_1HZ) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM11_Init(void)
{

  /* USER CODE BEGIN TIM11_Init 0 */
////
  /* USER CODE END TIM11_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM11_Init 1 */
////
  /* USER CODE END TIM11_Init 1 */
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 0;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 4999;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim11) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim11, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM11_Init 2 */
//
  /* USER CODE END TIM11_Init 2 */
  HAL_TIM_MspPostInit(&htim11);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* --- НАСТРОЙКА УРОВНЕЙ ПО УМОЛЧАНИЮ --- */

  // TFT и LED (Port A)
  HAL_GPIO_WritePin(GPIOA, TFT_RES_Pin|TFT_DC_Pin|TFT_CS_Pin|LED1_Pin, GPIO_PIN_SET);

  // FLASH (Port B)
  // CS (PB0) -> High (отключено)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
  // CLK (PB3) и MOSI (PB5) -> Low (0)
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);


  /* --- КОНФИГУРАЦИЯ ПИНОВ --- */

  /* КНОПКИ (Port E) */
  GPIO_InitStruct.Pin = BUTTON_K1_Pin|BUTTON_K0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* TFT ДИСПЛЕЙ (Port A) */
  GPIO_InitStruct.Pin = TFT_RES_Pin|TFT_DC_Pin|TFT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* LED (Port A) */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  /* FLASH ПАМЯТЬ (SOFT SPI - Port B) */
  // 1. Выходы: CS (PB0), CLK (PB3), MOSI (PB5)
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // Максимальная скорость для Soft SPI
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // 2. Вход: MISO (PB4)
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL; // Флешка сама тянет пин, подтяжка не обязательна
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
