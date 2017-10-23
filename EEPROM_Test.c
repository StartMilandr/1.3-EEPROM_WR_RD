#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_eeprom.h>

#define EEPROM_PAGE_SIZE             (4*1024)
//#define MAIN_EEPAGE                  5
#define LED_MASK                     (PORT_Pin_7 | PORT_Pin_8 | PORT_Pin_9)
#define LED0_MASK                    PORT_Pin_7

/* Private macro -------------------------------------------------------------*/
#define LED_TST_NUM(num)             (((num) * LED0_MASK) & LED_MASK)

/* Private variables ---------------------------------------------------------*/
static PORT_InitTypeDef PORT_InitStructure;

/* Private functions ---------------------------------------------------------*/
uint32_t Pseudo_Rand(uint32_t addr)
{
  uint32_t hash = 0;
  uint32_t i = 0;
  uint8_t* key = (uint8_t *)&addr;

  for (i = 0; i < 4; i++)
  {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  for (i = 0; i < 256; i++)
  {
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

void Delay(void)
{
  volatile uint32_t i = 0;
  for (i = 0; i < 100000; i++)
  {
  }
}

int32_t EEPROM_Test_WR_RD (int32_t testPage)
{
  uint32_t Address = 0;
  uint32_t BankSelector = 0;
  uint32_t Data = 0;
  uint32_t i = 0;
  uint32_t Leds = 0;
  uint8_t EraseErrMM = 0;
  uint8_t WriteErrMM = 0;

  /* Enables the clock on PORTD */
  RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);
  /* Enables the clock on EEPROM */
  RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);

  /* Configure PORTD pins 10..14 for output to switch LEDs on/off */
  PORT_InitStructure.PORT_Pin   = LED_MASK;
  PORT_InitStructure.PORT_OE    = PORT_OE_OUT;
  PORT_InitStructure.PORT_FUNC  = PORT_FUNC_PORT;
  PORT_InitStructure.PORT_MODE  = PORT_MODE_DIGITAL;
  PORT_InitStructure.PORT_SPEED = PORT_SPEED_SLOW;

  PORT_Init(MDR_PORTD, &PORT_InitStructure);

  /* Erase main memory page MAIN_EEPAGE */
  Address = 0x00000000 + testPage * EEPROM_PAGE_SIZE;
  BankSelector = EEPROM_Main_Bank_Select;
  EEPROM_ErasePage (Address, BankSelector);

  /* Check main memory page MAIN_EEPAGE */
  Data = 0xFFFFFFFF;
  for (i = 0; i < EEPROM_PAGE_SIZE; i += 4)
  {
    if (EEPROM_ReadWord (Address + i, BankSelector) != Data)
    {
      EraseErrMM = 1;
    }
  }

  /* Indicate status of erasing main memory page MAIN_EEPAGE */
  Leds = LED_TST_NUM(0x1);
  if (EraseErrMM == 0)
  {
    PORT_Write(MDR_PORTD, Leds);
  }
  else
  {
    while (1)
    {
      Leds ^= LED_TST_NUM(0x1);
      PORT_Write(MDR_PORTD, Leds);
      Delay();
    }
  }

  /* Fill main memory page MAIN_EEPAGE */
  Address = 0x00000000 + testPage * EEPROM_PAGE_SIZE;
  BankSelector = EEPROM_Main_Bank_Select;
  for (i = 0; i < EEPROM_PAGE_SIZE; i += 4)
  {
    Data = Pseudo_Rand (Address + i);
    EEPROM_ProgramWord (Address + i, BankSelector, Data);
  }

  /* Check main memory page MAIN_EEPAGE */
  Address = 0x00000000 + testPage * EEPROM_PAGE_SIZE;
  BankSelector = EEPROM_Main_Bank_Select;
  for (i = 0; i < EEPROM_PAGE_SIZE; i +=4 )
  {
    Data = Pseudo_Rand (Address + i);
    if (EEPROM_ReadWord (Address + i, BankSelector) != Data)
    {
      WriteErrMM = 1;
    }
  }

  /* Indicate status of writing main memory page MAIN_EEPAGE */
  Leds = LED_TST_NUM(0x2);
  if (EraseErrMM == 0)
  {
    PORT_Write(MDR_PORTD, Leds);
  }
  else
  {
    while (1)
    {
      Leds ^= LED_TST_NUM(0x2);
      PORT_Write(MDR_PORTD, Leds);
      Delay();
    }
  }

  /* Erase main memory page MAIN_EEPAGE */
  Address = 0x00000000 + testPage * EEPROM_PAGE_SIZE;
  BankSelector = EEPROM_Main_Bank_Select;
  EEPROM_ErasePage (Address, BankSelector);

  /* Check main memory page MAIN_EEPAGE */
  Data = 0xFFFFFFFF;
  for (i = 0; i < EEPROM_PAGE_SIZE; i += 4)
  {
    if (EEPROM_ReadWord (Address + i, BankSelector) != Data)
    {
      EraseErrMM = 1;
    }
  }

  /* Indicate status of erasing main memory page MAIN_EEPAGE */
  Leds = LED_TST_NUM(0x3);
  if (EraseErrMM == 0)
  {
    PORT_Write(MDR_PORTD, Leds);
  }
  else
  {
    while (1)
    {
      Leds ^= LED_TST_NUM(0x3);
      PORT_Write(MDR_PORTD, Leds);
      Delay();
    }
  }

	return EraseErrMM + WriteErrMM;
}
