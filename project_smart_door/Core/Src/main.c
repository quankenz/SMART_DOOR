/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "rc522.h"
#include "lcd.h"
#include "string.h"
#include "stdio.h"
#include "keypad.h"
#include "stdbool.h"
#include "flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Cac mode cua the
#define MODE_CHECK_CARD 0
#define MODE_ADD_CARD 1
#define MODE_DEL_CARD 2
// Cac mode cua password
#define MODE_RUN 0
#define MODE_CHANGE 1
#define MODE_INPUT_PASS 2
#define MODE_PASS_MASTER 3
// Trang thai
#define OK 0
#define NOT_OK 1
// Dia chi Flash
#define ADDRESS_PMASTER_STORAGE (0x08000000 + 90 * 1024)
#define ADDRESS_PASS_STORAGE (0x08000000 + 91 * 1024)
#define ADDRESS_CARD_STORAGE (0x08000000 + 92 * 1024)
#define ADDRESS_COUNT_STORAGE (0x08000000 + 93 * 1024)
#define ADDRESS_COUNT_M_STORAGE (0x08000000 + 94 * 1024)
#define LIMIT_CARD	255
#define LIMIT_PASS	6
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
// Key card default
char key_card[LIMIT_CARD*4];
char status;
char str[MFRC522_MAX_LEN]; //MFRC522_MAX_LEN = 16
char serNum[5];
int wrong_card = 0;

// Password default
char pass_master[] = {'1','1','1','1','1','1'};
char password[LIMIT_PASS];
// Cac bien su dung
uint8_t key;
int len_key_card = 0;
char i,j,tmp,x,k;
char count_key = 0;
char count_key_in = 0;
char count_enter_pass = 0;
char count_enter_pmaster = 0;
char key_pass[7];
char buffer_pass[7];
char new_pass1[7];
char new_pass2[7];
char mode_pass = 4;
char status_pass = 2;
char check_pass = 2;
char mode_card = MODE_CHECK_CARD;
char status_card = 2;
char check_card = 2;
char checking = NOT_OK;
int time = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
int get_len_keycard(void);
char check_card_RFID(void);
void check_Card(void);
void add_Card(void);
void del_Card (void);
char run_CheckPass(char pass[LIMIT_PASS]);
void run_Normal(void);
void run_ChangePass(void);
void run_InputPass(void);
void run_PassMaster(void);
void get_key(void);
void press_button(void);
void notify_audio(int s);
void open_latch(void); 
void loop_main(void);
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
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	
	Flash_Erase(ADDRESS_PMASTER_STORAGE);
	Flash_Write_Array(ADDRESS_PMASTER_STORAGE, (uint8_t*)pass_master, LIMIT_PASS);
	HAL_Delay(100);
	Flash_Erase(ADDRESS_PASS_STORAGE);
	Flash_Write_Array(ADDRESS_PASS_STORAGE, (uint8_t*)pass_master, LIMIT_PASS);
	HAL_Delay(100);
	
	Flash_Read_Array(ADDRESS_PASS_STORAGE, (uint8_t*)password, LIMIT_PASS);
	count_enter_pass = Flash_Read_Int(ADDRESS_COUNT_STORAGE);
	count_enter_pmaster = Flash_Read_Int(ADDRESS_COUNT_M_STORAGE);
	
  MFRC522_Init();
  lcd_init();
  lcd_put_cur(0, 3);
  lcd_send_string("WELLCOME");
	lcd_put_cur(1, 1);
  lcd_send_string("NHOM 9 - DACN");
  HAL_Delay(2000);
  lcd_clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    loop_main();
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
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA9 PA10 PA11
                           PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB15 PB4 PB5 PB6
                           PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

// Ham nhan gia tri tu ban phim nhap
void get_key()
{
  uint8_t key_input = read_keypad();
  if (key_input != 0xFF)
  {
    key = key_input;
    HAL_Delay(100);
  }
  // Che do doi mat khau
  if (key == 'A')
  {
    lcd_clear();
    count_key_in = 0;
    count_key = 0;
    mode_pass = MODE_CHANGE;
		mode_card = 3;
  }
	else if (key == 'B')
	{
		lcd_clear();
    count_key = 0;
		mode_card = MODE_ADD_CARD;
		mode_pass = 4;
    check_pass = 2;
	}
	else if (key == 'C')
	{
		lcd_clear();
    count_key = 0;
		mode_card = MODE_DEL_CARD;
		mode_pass = 4;
    check_pass = 2;
	}
	else if (key == 'D')
	{
		lcd_clear();
    count_key = 0;
		mode_pass = MODE_PASS_MASTER;
		mode_card = 3;
    check_pass = 2;
	}
  // Nhan * de backspace
  else if (key == '*')
  {
    if ((count_key > 0)&&(checking == OK))
    {
      count_key--;
      lcd_put_cur(1, count_key);
      lcd_send_data (' ');
    }
		else if ((count_key == 0)&&(checking == OK))
		{
			lcd_clear();
			check_pass = 2;
			mode_pass = 4;
			mode_card = MODE_CHECK_CARD;
		}
    if (count_key_in > 0&&(mode_pass==MODE_INPUT_PASS))
    {
      if(count_key_in<LIMIT_PASS)
      {
        lcd_put_cur(1, count_key_in-1);
        lcd_send_data (' ');
        count_key_in--;
      } 
      else
      {
        lcd_put_cur(1, count_key_in-(LIMIT_PASS+1));
        lcd_send_data (' ');
        count_key_in--;
      }
    }
  }
	// Nhan # de xac nhan password or cho phep nhap password
  else if (key == '#') 
  {
		lcd_clear();
    count_key_in = 0;
    count_key = 0;
    check_pass = 2;
		mode_card = MODE_CHECK_CARD;
    mode_pass = MODE_RUN;
		
  }
  else {}
  key_input = 0;
}

// Ham vong lap chuong trinh chinh
void loop_main()
{
	//Flash_Read_Array(ADDRESS_PASS_STORAGE, (uint8_t*)password, LIMIT_PASS);
	//Flash_Read_Int(ADDRESS_COUNT_STORAGE);
	//Flash_Read_Int(ADDRESS_COUNT_M_STORAGE);
	press_button();
  get_key();
	switch (mode_card)
	{
		case MODE_CHECK_CARD:
      check_Card();
      break;
    case MODE_ADD_CARD:
      add_Card();
      break;
    case MODE_DEL_CARD:
      del_Card();
      break;
    default:
      break;
	}
  switch (mode_pass)
  {
    case MODE_RUN:
      run_Normal();
      break;
    case MODE_CHANGE:
      run_ChangePass();
      break;
    case MODE_INPUT_PASS:
      run_InputPass();
      break;
		case MODE_PASS_MASTER:
			run_PassMaster();
			break;
    default:
      break;
  }
}



// Ham check password
char run_CheckPass(char pass[LIMIT_PASS])
{
  checking = OK;
  if ((key >= '0')&&(key <= '9'))
  {
    if (count_key == 0)
    {
      lcd_clear(); // Xoa man hinh
    }
    if (count_key < LIMIT_PASS)
    {
      buffer_pass[count_key] = key;
			//char buff_key[1];
			//buff_key[0] = key;
      lcd_put_cur(1,count_key);
      lcd_send_string("*"); // Hien thi '*' the hien dang nhap pass
      count_key++;
      key = 0;
    }
    if (count_key == LIMIT_PASS)
    {
      count_key = 0;
      lcd_clear(); // Xoa man hinh
      for(i = 0; i < LIMIT_PASS; i++){
        if(pass[i] != buffer_pass[i])
          break;
      }
      if (i == LIMIT_PASS)
      {
        check_pass = OK;
      }
      else
      {
        check_pass = NOT_OK;
        lcd_clear();
        lcd_put_cur(0,0);
        lcd_send_string("WRONG PASSWORD!"); // Hien thi sai mat khau
        notify_audio(4);
        HAL_Delay(3000);
        lcd_clear();
      }
    }
  }
  return check_pass;
  checking = NOT_OK;
}

// Ham run_Normal
void run_Normal()
{
  lcd_put_cur(0, 0);
  lcd_send_string("ENTER PASSWORD:");
  status_pass = run_CheckPass(password);
  if (status_pass == OK)
  {
		HAL_Delay(300);
    lcd_clear();
    lcd_put_cur(0,1);
    lcd_send_string("DOOR IS OPEN!"); // Hien thi mo cua
    notify_audio(2);
    open_latch();
    lcd_clear();
    count_enter_pass = 0;
		Flash_Erase(ADDRESS_COUNT_STORAGE);
		Flash_Write_Int(ADDRESS_COUNT_STORAGE, count_enter_pass);
		HAL_Delay(100);
    mode_pass = 4;
    check_pass = 2;
  }
  else if (status_pass == NOT_OK)
  {
    check_pass = 2;
    count_enter_pass++;
		Flash_Erase(ADDRESS_COUNT_STORAGE);
		Flash_Write_Int(ADDRESS_COUNT_STORAGE, count_enter_pass);
		HAL_Delay(100);
    if (count_enter_pass >= 3)
    {
      //count_enter_pass = 0;
			char buff_times[16];
			lcd_clear();
			sprintf(buff_times, "WRONG %d TIMES", count_enter_pass);
			notify_audio(15);
      lcd_clear();
      lcd_put_cur(0,3);
      lcd_send_string("WARNING!"); // Canh bao
			lcd_put_cur(1,1);
      lcd_send_string(buff_times);
      lcd_clear();
			time = count_enter_pass*40;
			lcd_put_cur(0,2);
      lcd_send_string("WAIT FOR");
			while(time>0)
			{
				char buff_time[16];
				sprintf(buff_time, "%d SECONDS", time);
				lcd_put_cur(1,1);
				lcd_send_string(buff_time);
				HAL_Delay(1000);
				time--;
			}
			lcd_clear();
    }
		check_pass = 2;
    status_pass = 2;
		mode_pass = 4;
    mode_card = MODE_CHECK_CARD;
  }
  else {}
}

// Ham run_ChangePass
void run_ChangePass()
{
  lcd_put_cur(0, 1);
  lcd_send_string("OLD PASSWORD:");
  status_pass = run_CheckPass(password);
  if (status_pass == OK)
  {
		HAL_Delay(300);
		lcd_put_cur(0,1);
		lcd_send_string("NEW PASSWORD:");
    mode_pass = MODE_INPUT_PASS;
  }
  else if (status_pass == NOT_OK)
  {
    lcd_clear();
    status_pass = 2;
    mode_pass = 4;
  }
  else {}
}

// Ham run_InputPass
void run_InputPass(){
  if ((key >= '0')&&(key <= '9'))
  {
    if (count_key_in < LIMIT_PASS)
    {
      new_pass1[count_key_in] = key;
      lcd_put_cur(1,count_key_in);
      lcd_send_string("*");
      count_key_in++;
      if (count_key_in == LIMIT_PASS){
        lcd_clear();
        lcd_put_cur(0,1);
        lcd_send_string("CONFIRM PASS:");
      }
      
      key = 0;
    }
    else
    {
      new_pass2[count_key_in-LIMIT_PASS] = key;
      lcd_put_cur(1,count_key_in-LIMIT_PASS);
      lcd_send_string("*");
      count_key_in++;
      key = 0;
    }
    if (count_key_in == LIMIT_PASS*2)
    {
      lcd_clear();
      count_key_in = 0;
      for(i = 0; i < LIMIT_PASS; i++)
      {
        if (new_pass1[i] != new_pass2[i])
          break;
      }
      if (i == 6)
      {
				for(i = 0; i < LIMIT_PASS; i++)
				{
					password[i] = new_pass1[i];
				}
        i=0;
				
				Flash_Erase(ADDRESS_PASS_STORAGE);
				Flash_Write_Array(ADDRESS_PASS_STORAGE, (uint8_t*)password, LIMIT_PASS);
				HAL_Delay(100);
				//Flash_Read_Array(ADDRESS_PASS_STORAGE, (uint8_t*)password, LIMIT_PASS);
        
        lcd_clear();
        lcd_put_cur(0,4);
        lcd_send_string("CHANGE");
				lcd_put_cur(1,2);
        lcd_send_string("SUCCESSFULLY");
				notify_audio(2);
        check_pass = 2;
      }
      else
      {
        lcd_clear();
        lcd_put_cur(0,4);
        lcd_send_string("CHANGE");
				lcd_put_cur(1,0);
        lcd_send_string("UNSUCCESSFULLY");
				notify_audio(4);
        check_pass = 2;
      }
      HAL_Delay(2000);
      lcd_clear();
			check_pass = 2;
      status_pass = 2;
			mode_pass = 4;
      mode_card = MODE_CHECK_CARD;
    }
  }
}

//Ham nhap pass master
void run_PassMaster()
{
	lcd_put_cur(0, 0);
  lcd_send_string("PASSWORD MASTER:");
  status_pass = run_CheckPass(pass_master);
  if(status_pass == OK)
  {
		check_pass = 2;
		status_pass = 2;
		lcd_put_cur(0,0);
		lcd_send_string("PASSWORD MASTER");
		lcd_put_cur(1,3);
		lcd_send_string("IS CORRECT");
		notify_audio(2);
		Flash_Erase(ADDRESS_COUNT_M_STORAGE);
		count_enter_pmaster = 0;
		Flash_Write_Int(ADDRESS_COUNT_M_STORAGE, count_enter_pmaster);
		HAL_Delay(100);
    HAL_Delay(1000);
		while(1)
		{
			get_key();
			lcd_put_cur(0,0);
			lcd_send_string("1:RESET PASSWORD");
			lcd_put_cur(1,0);
			lcd_send_string("2:RESET SYSTEM");
			if ((key >= '0')&&(key <= '9'))
			{
				lcd_clear(); // Xoa man hinh
				if(key == '1')
				{
					notify_audio(1);
					lcd_put_cur(0,0);
					lcd_send_string("RESET PASSWORD");
					lcd_put_cur(1,1);
					lcd_send_string("1:YES     2:NO");
					key = 0;
					while(1)
					{
						get_key();
						if ((key >= '0')&&(key <= '9'))
						{
							if(key == '1')
							{
								lcd_clear();
								Flash_Erase(ADDRESS_PASS_STORAGE);
								Flash_Write_Array(ADDRESS_PASS_STORAGE, (uint8_t*)pass_master, LIMIT_PASS);
								HAL_Delay(100);
								Flash_Erase(ADDRESS_COUNT_STORAGE);
								count_enter_pass = 0;
								Flash_Write_Int(ADDRESS_COUNT_STORAGE, count_enter_pass);
								HAL_Delay(100);
								Flash_Read_Array(ADDRESS_PASS_STORAGE, (uint8_t*)password, LIMIT_PASS);
								HAL_Delay(2000);
								lcd_put_cur(0,0);
								lcd_send_string("RESET PASSWORD");
								lcd_put_cur(1,0);
								lcd_send_string("SUCCESSFULLY");
								notify_audio(2);
								lcd_clear();
								mode_pass = 4;
								mode_card = MODE_CHECK_CARD;
								break;
							}
							if(key == '2')
							{
								lcd_clear();
								break;
							}
						}
					}
					if(key == '1' | key == '2')
					{
						key = 0;
						mode_pass = 4;
						mode_card = MODE_CHECK_CARD;
						break;
					}
				}
				else if(key == '2')
				{
					notify_audio(1);
					lcd_put_cur(0,0);
					lcd_send_string("RESET SYSTEM");
					lcd_put_cur(1,1);
					lcd_send_string("1:YES     2:NO");
					key = 0;
					while(1)
					{
						get_key();
						if ((key >= '0')&&(key <= '9'))
						{
							if(key == '1')
							{
								lcd_clear();
								Flash_Erase(ADDRESS_PASS_STORAGE);
								Flash_Write_Array(ADDRESS_PASS_STORAGE, (uint8_t*)pass_master, LIMIT_PASS);
								HAL_Delay(100);
								Flash_Erase(ADDRESS_COUNT_STORAGE);
								count_enter_pass = 0;
								Flash_Write_Int(ADDRESS_COUNT_STORAGE, count_enter_pass);
								HAL_Delay(100);
								Flash_Erase(ADDRESS_COUNT_M_STORAGE);
								count_enter_pmaster = 0;
								Flash_Write_Int(ADDRESS_COUNT_M_STORAGE, count_enter_pmaster);
								HAL_Delay(100);
								Flash_Erase(ADDRESS_CARD_STORAGE);
								Flash_Read_Array(ADDRESS_PASS_STORAGE, (uint8_t*)password, LIMIT_PASS);
								HAL_Delay(2000);
								lcd_put_cur(0,0);
								lcd_send_string("RESET SYSTEM");
								lcd_put_cur(1,2);
								lcd_send_string("SUCCESSFULLY");
								notify_audio(2);
								lcd_clear();
								mode_pass = 4;
								mode_card = MODE_CHECK_CARD;
								break;
							}
							if(key == '2')
							{
								lcd_clear();
								break;
							}
						}
					}
					if(key == '1' | key == '2')
					{
						key = 0;
						mode_pass = 4;
						mode_card = MODE_CHECK_CARD;
						break;
					}
				}
			}
		}
  }
  else if (status_pass == NOT_OK)
  {
    check_pass = 2;
    count_enter_pmaster++;
		Flash_Erase(ADDRESS_COUNT_M_STORAGE);
		Flash_Write_Int(ADDRESS_COUNT_M_STORAGE, count_enter_pmaster);
		HAL_Delay(100);
    if (count_enter_pmaster >= 3)
    {
			lcd_clear();
			char buff_times[16];
			sprintf(buff_times, "WRONG %d TIMES", count_enter_pmaster);
			notify_audio(15);
			lcd_clear();
      lcd_put_cur(0,3);
      lcd_send_string("WARNING!"); // Canh bao
			lcd_put_cur(1,1);
      lcd_send_string(buff_times);
      lcd_clear();
			time = count_enter_pmaster*40;
			lcd_put_cur(0,2);
      lcd_send_string("WAIT FOR");
			while(time>0)
			{
				char buff_time[16];
				sprintf(buff_time, "%d SECONDS", time);
				lcd_put_cur(1,1);
				lcd_send_string(buff_time);
				HAL_Delay(1000);
				time--;
			}
			lcd_clear();
    }
		check_pass = 2;
		status_pass = 2;
		mode_pass = 4;
		mode_card = MODE_CHECK_CARD;
  }
  else {}
}

int get_len_keycard()
{
	int n = 0;
	Flash_Read_Array(ADDRESS_CARD_STORAGE, (uint8_t*)key_card, LIMIT_CARD*4);
	for(i = 0; i<sizeof(key_card)/key_card[0]; i++)
	{
		if(key_card[i] != 0xFF)
		{
			n++;
		}
		else
		{
			break;
		}
	}
	return n;
}

// Ham check card
char check_card_RFID()
{
	bool is_true_card = false;
	char buff_uID[10];
	status = MFRC522_Request(PICC_REQIDL, str);
	status = MFRC522_Anticoll(str);
	memcpy(serNum, str, 4);
	if(status == MI_OK)
	{
		notify_audio(1);
		for(i = 0; i < len_key_card/4; i++)
		{
			bool match = true;
			for(j = 4*i; j < 4*i+4; j++)
			{
				if(key_card[j] != serNum[j%4])
				{
					match = false;
					break;
				}
			}
			if(match)
			{
				is_true_card = true;
				break;
			}
		}
		if(is_true_card == false)
		{
				check_card = NOT_OK;
		}
		else
		{
			check_card = OK;
		}
	}
	return check_card;
}

void check_Card()
{
	len_key_card = get_len_keycard();
	status_card = check_card_RFID();
	if(status_card == OK)
	{
		check_card = 2;
		lcd_clear();
		lcd_put_cur(0, 1);
		lcd_send_string("DOOR IS OPEN!");
		notify_audio(2);
		open_latch();
		mode_pass = 2;
		lcd_clear();
	}
	else if (status_card == NOT_OK)
	{
		if(len_key_card <= 0)
		{
			lcd_clear();
			lcd_put_cur(0,2);
			lcd_send_string("THE CARD'S");
			lcd_put_cur(1,0);
			lcd_send_string("MEMORY IS EMPTY");
			notify_audio(4);
			HAL_Delay(2000);
			lcd_clear();
			mode_pass = 4;
			check_card = 2;
			status_card = 2;
		} 
		else	
		{
			check_card = 2;
			lcd_clear();
			lcd_put_cur(0, 1);
			lcd_send_string("WRONG CARD!");
			notify_audio(4);
			HAL_Delay(3000);
			lcd_clear();
			mode_pass = 4;
			check_card = 2;
			status_card = 2;
		}
	}
}
// Ham them the
void add_Card()
{
	if(len_key_card == LIMIT_CARD*4)
	{
		lcd_clear();
		lcd_put_cur(0,2);
		lcd_send_string("THE CARD'S");
		lcd_put_cur(1,1);
		lcd_send_string("MEMORY IS FULL");
		mode_card = MODE_CHECK_CARD;
	}
	else
	{
		lcd_put_cur(0, 0);
		lcd_send_string("PASSWORD MASTER:");
		status_pass = run_CheckPass(pass_master);
		if(status_pass == OK)
		{
			lcd_clear();
			check_pass = 2;
			status_pass = 2;
			lcd_put_cur(0,1);
			lcd_send_string("CORRECT PASS!");
			notify_audio(2);
			HAL_Delay(1000);
			lcd_clear();
			lcd_put_cur(0,1);
			lcd_send_string("PUT NEW CARD");
			lcd_put_cur(1,1);
			lcd_send_string("ON NFC READER");
			while(1)
			{
				status_card = check_card_RFID();
				if(status_card == OK)
				{
					lcd_clear();
					lcd_put_cur(0,1);
					lcd_send_string("THE CARD");
					lcd_put_cur(1,5);
					lcd_send_string("HAS EXISTED");
					notify_audio(4);
					HAL_Delay(2000);
					lcd_clear();
					mode_card = MODE_CHECK_CARD;
					check_card = 2;
					status_card = 2;
					break;
				}
				else if(status_card == NOT_OK)
				{
					for(i=len_key_card; i<len_key_card+4; i++)
						{
							key_card[i] = serNum[i%4];
						}
					Flash_Erase(ADDRESS_CARD_STORAGE);
					Flash_Write_Array(ADDRESS_CARD_STORAGE, (uint8_t*)key_card, LIMIT_CARD*4);
					HAL_Delay(100);
					
					lcd_clear();
					lcd_put_cur(0,1);
					lcd_send_string("ADD NEW CARD");
					lcd_put_cur(1,1);
					lcd_send_string("SUCCESSFULLY");
					notify_audio(2);
					HAL_Delay(2000);
					lcd_clear();
					mode_card = MODE_CHECK_CARD;
					check_card = 2;
					status_card = 2;
					break;
				}
				HAL_Delay(1000);
				k++;
				if(k == 4)
				{
					lcd_clear();
					mode_card = MODE_CHECK_CARD;
					k = 0;
					break;
				}
			}
		}
		else if (status_pass == NOT_OK)
		{
			lcd_clear();
			mode_card = MODE_CHECK_CARD;
			mode_pass = 4;
			check_pass = 2;
			status_pass = 2;
		}
	}
}

// Ham xoa the
void del_Card()
{
	if(len_key_card <= 0)
	{
		lcd_clear();
		lcd_put_cur(0,2);
		lcd_send_string("THE CARD'S");
		lcd_put_cur(1,0);
		lcd_send_string("MEMORY IS EMPTY");
		notify_audio(4);
		HAL_Delay(2000);
		lcd_clear();
		mode_card = MODE_CHECK_CARD;
		check_card = 2;
		status_pass = 2;
		status_card = 2;
	}
	else
	{
		lcd_put_cur(0, 0);
		lcd_send_string("PASSWORD MASTER:");
		status_pass = run_CheckPass(pass_master);
		if(status_pass == OK)
		{
			lcd_clear();
			check_pass = 2;
			status_pass = 2;
			lcd_put_cur(0,1);
			lcd_send_string("CORRECT PASS!");
			notify_audio(2);
			HAL_Delay(1000);
			lcd_clear();
			lcd_put_cur(0,1);
			lcd_send_string("PUT YOUR CARD");
			lcd_put_cur(1,1);
			lcd_send_string("ON NFC READER");
			while(1)
			{
				status_card = check_card_RFID();
				if(status_card == OK)
				{
					for(x = i*4; x<len_key_card; x++)
					{
						key_card[x] = key_card[x+4];
						if(x >= len_key_card-4)
						{
							key_card[x] = 0xFF;
						}
					}
					Flash_Erase(ADDRESS_CARD_STORAGE);
					Flash_Write_Array(ADDRESS_CARD_STORAGE, (uint8_t*)key_card, LIMIT_CARD*4);
					HAL_Delay(100);
					
					lcd_clear();
					lcd_put_cur(0,1);
					lcd_send_string("DEL THIS CARD");
					lcd_put_cur(1,1);
					lcd_send_string("SUCCESSFULLY");
					notify_audio(2);
					HAL_Delay(2000);
					lcd_clear();
					mode_card = MODE_CHECK_CARD;
					check_card = 2;
					status_card = 2;
					break;
				} 
				else if (status_card == NOT_OK)
				{
					lcd_clear();
					lcd_put_cur(0,2);
					lcd_send_string("NO EXIST");
					lcd_put_cur(1,6);
					lcd_send_string("THE CARD");
					notify_audio(4);
					HAL_Delay(2000);
					lcd_clear();
					mode_card = MODE_CHECK_CARD;
					check_card = 2;
					status_card = 2;
					break;
				}
				HAL_Delay(1000);
				k++;
				if(k == 4)
				{
					lcd_clear();
					mode_card = MODE_CHECK_CARD;
					k = 0;
					break;
				}
			}
		}
		else if (status_pass == NOT_OK)
		{
			lcd_clear();
			mode_card = MODE_CHECK_CARD;
			mode_pass = 4;
			check_pass = 2;
			status_pass = 2;
		}
	}
}	


// Ham thong bao bang am thanh
void notify_audio(int s)
{
  for(int ii=0; ii<s; ii++)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 1);
    HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, 0);
		HAL_Delay(200);
  }
	HAL_Delay(500);
}

// Ham mo khoa
void open_latch()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 1);
	HAL_Delay(5000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, 0);
}

// Ham nhan mo cua
void press_button()
{
	if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)==0)
	{
		while(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15)==0);
		lcd_clear();
		lcd_put_cur(0, 1);
		lcd_send_string("DOOR IS OPEN!");
		notify_audio(2);
		open_latch();
		lcd_clear();
		mode_pass = 4;
		check_pass = 2;
		status_pass = 2;
		check_card = 2;
		status_card = 2;
		mode_card = MODE_CHECK_CARD;
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
