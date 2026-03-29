/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "st7789.h"
#include "fonts.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SAMPLES 320
#define GRID_SIZE 40
#define GRID_COLOR GRAY
#define CENTER_LINE_COLOR WHITE
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* USER CODE BEGIN PV */
volatile uint8_t adc_capture_complete = 0;
uint16_t adc_raw[SAMPLES];      // Filled by DMA
uint16_t old_y[SAMPLES];        // History for flicker-free erase
float v_scale = 1.0;            // Voltage zoom
int t_scale = 1;                // Time zoom
int v_offset = 120;             // Vertical position

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
void Draw_Grid(void);
void Repair_Grid_Pixel(uint16_t x, uint16_t y);
void Handle_Buttons(void);
//void Update_OSD(void);
int GetTrigger(void);
void Update_Sidebar(void);
void Handle_Buttons();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  ST7789_Init();
        ST7789_SetRotation(1);
        ST7789_Fill_Color(BLACK);
        Draw_Grid();

        // Start the very first single-shot capture
        adc_capture_complete = 0;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw, SAMPLES);

      // DRAW A BORDER AROUND THE EDGE
      ST7789_DrawLine(0, 0, 319, 0, RED);
      ST7789_DrawLine(0, 239, 319, 239, RED);
      ST7789_DrawLine(0, 0, 0, 239, RED);
      ST7789_DrawLine(319, 0, 319, 239, RED);

      HAL_Delay(2000); // Look at the red box!

      // Clear the box and restart clean
      ST7789_Fill_Color(BLACK);
      Draw_Grid();
      //Update_OSD();

      // You only need this call ONCE here at the end of setup
      // HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw, SAMPLES);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
     //float angle = 0;
     uint8_t last_sidebar_update = 0;
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  // 1. WAIT: Non-blocking wait until the DMA sets the complete flag
	        if (!adc_capture_complete) {
	            Handle_Buttons(); // Process buttons while waiting
	            continue; // Go to top of loop and keep waiting
	        }

	        // 2. PROCESS: The DMA is finished, buffer is locked.
	        // Call the trigger function and ensure the value is stored.
	        int trigger = GetTrigger();

	        // 3. DRAWING LOOP (Unchanged logic, just ensure trigger is used)
	        for (int x = 0; x < 279; x++) {

	            // --- STEP A: ERASE OLD WAVE (3 Pixels Wide) ---
	            ST7789_DrawLine(x, old_y[x], x+1, old_y[x+1], BLACK);
	            ST7789_DrawLine(x, old_y[x]-1, x+1, old_y[x+1]-1, BLACK);
	            ST7789_DrawLine(x, old_y[x]+1, x+1, old_y[x+1]+1, BLACK);

	            // --- STEP B: REPAIR GRID ---
	            if (x % 40 == 0) ST7789_DrawPixel(x, old_y[x], GRID_COLOR);
	            if (old_y[x] % 40 == 0) ST7789_DrawPixel(x, old_y[x], GRID_COLOR);
	            if (old_y[x] == 120) ST7789_DrawLine(x, 120, x+1, 120, WHITE);

	            // --- STEP C: GET DATA & MAP TO SCREEN ---
	            // Use the trigger offset to lock the wave start
	            uint16_t raw_current = adc_raw[(trigger + (x * t_scale)) % SAMPLES];
	            uint16_t raw_next    = adc_raw[(trigger + ((x+1) * t_scale)) % SAMPLES];

	            int16_t y_now  = 120 - (int16_t)((raw_current - 2048) * v_scale / 17);
	            int16_t y_next = 120 - (int16_t)((raw_next - 2048) * v_scale / 17);

	            // Safety
	            if (y_now > 237) y_now = 237;
	            if (y_now < 2) y_now = 2;
	            if (y_next > 237) y_next = 237;
	            if (y_next < 2) y_next = 2;

	            // --- STEP D: DRAW WAVE ---
	            ST7789_DrawLine(x, y_now, x + 1, y_next, GREEN);

	            old_y[x] = y_now;
	        }

	        // 4. MEASUREMENTS & HOUSEKEEPING
	        if (HAL_GetTick() - last_sidebar_update > 500) {
	            Update_Sidebar();
	            last_sidebar_update = HAL_GetTick();
	        }
	        Handle_Buttons(); // Also call here in case capture was slow

	        // 5. CAPTURE REQUEST: Start a new clean Single-Shot acquisition
	        adc_capture_complete = 0;
	        HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_raw, SAMPLES);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
 * @brief Finds the rising edge of the signal to prevent the wave from jumping.
 */
int GetTrigger() {
    for (int i = 1; i < SAMPLES/2; i++) {
        // Look for the point where the wave crosses the middle voltage (ADC 2048)
        if (adc_raw[i-1] < 2048 && adc_raw[i] >= 2048) return i;
    }
    return 0;
}

/**
 * @brief Calculates and displays the Volts and Scales on the right sidebar.
 */
void Update_Sidebar(void) {
    uint16_t v_max = 0, v_min = 4095;
    char str[15];

    // Find the peaks in the real ADC buffer
    for (int i = 0; i < SAMPLES; i++) {
        if (adc_raw[i] > v_max) v_max = adc_raw[i];
        if (adc_raw[i] < v_min) v_min = adc_raw[i];
    }

    float volt_max = (v_max * 3.3f) / 4095.0f;
    float volt_pp  = ((v_max - v_min) * 3.3f) / 4095.0f;

    // Clear and redraw Sidebar area
    ST7789_DrawFilledRectangle(281, 0, 39, 240, BLACK);
    ST7789_DrawLine(281, 0, 281, 240, WHITE); // Sidebar border

    ST7789_WriteString(285, 10, "MEAS", Font_7x10, YELLOW, BLACK);

    ST7789_WriteString(285, 40, "MAX", Font_7x10, WHITE, BLACK);
    sprintf(str, "%.1fV", volt_max);
    ST7789_WriteString(285, 52, str, Font_7x10, CYAN, BLACK);

    ST7789_WriteString(285, 85, "VPP", Font_7x10, WHITE, BLACK);
    sprintf(str, "%.1fV", volt_pp);
    ST7789_WriteString(285, 97, str, Font_7x10, GREEN, BLACK);

    ST7789_WriteString(285, 130, "ZOOM", Font_7x10, YELLOW, BLACK);
    sprintf(str, "V:%.1f", v_scale);
    ST7789_WriteString(285, 150, str, Font_7x10, WHITE, BLACK);
    sprintf(str, "T:%d", t_scale);
    ST7789_WriteString(285, 170, str, Font_7x10, WHITE, BLACK);
}

/**
 * @brief Handles button presses to adjust the scale of the wave.
 */
void Handle_Buttons() {
    // PB12: Vertical Scale Up, PB13: Vertical Scale Down
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET) { v_scale += 0.2f; if(v_scale > 10.0) v_scale = 10.0; HAL_Delay(150); }
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET) { v_scale -= 0.2f; if(v_scale < 0.2) v_scale = 0.2; HAL_Delay(150); }

    // PB14: Timebase Up (More cycles), PB15: Timebase Down (Zoom in)
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET) { t_scale++; if(t_scale > 20) t_scale = 20; HAL_Delay(150); }
    if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_RESET) { if(t_scale > 1) t_scale--; HAL_Delay(150); }
}

/**
 * @brief Draws the grid lines for the waveform area only.
 */
void Draw_Grid() {
    for (int x = 0; x <= 280; x += GRID_SIZE) ST7789_DrawLine(x, 0, x, 240, GRID_COLOR);
    for (int y = 0; y <= 240; y += GRID_SIZE) ST7789_DrawLine(0, y, 280, y, GRID_COLOR);
    ST7789_DrawLine(0, 120, 280, 120, CENTER_LINE_COLOR);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    // Only process if it's ADC1 (your configured ADC)
    if (hadc->Instance == ADC1) {
        adc_capture_complete = 1; // Signal the main loop that a new buffer is ready
    }
}
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
#ifdef USE_FULL_ASSERT
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
