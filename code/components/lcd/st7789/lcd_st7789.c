#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "lcd_st7789.h"

static void lcd_spi_pre_transfer_callback(spi_transaction_t *t);

#define LCD_WR_REG lcd_wr_reg
#define LCD_WR_DATA lcd_wr_data_16bit
#ifdef USE_INT_RAM
DRAM_ATTR uint16_t lcd_ram[RES_HOR][RES_VER];
#endif

#define LCD_HOST    HSPI_HOST
#define DMA_CHAN    2

#define PIN_NUM_MOSI 36
#define PIN_NUM_CLK  35
#define PIN_NUM_CS   34

#define PIN_NUM_DC   37
#define PIN_NUM_RST  38
#define PIN_NUM_BCKL 33

#define SPI_SPEED_MHz 26 * 1000 * 1000

esp_err_t ret;
static spi_device_handle_t spi;


//Place data into DRAM. Constant data gets placed into DROM by default, which is not accessible by DMA.
DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
    /* Memory Data Access Control, MX=MV=1, MY=ML=MH=0, RGB=0 */
    {0x36, {(1<<5)|(1<<6)}, 1},
    /* Interface Pixel Format, 16bits/pixel for RGB/MCU interface */
    {0x3A, {0x55}, 1},
    /* Porch Setting */
    {0xB2, {0x0c, 0x0c, 0x00, 0x33, 0x33}, 5},
    /* Gate Control, Vgh=13.65V, Vgl=-10.43V */
    {0xB7, {0x35}, 1},
    /* VCOM Setting, VCOM=1.175V */
    {0xBB, {0x19}, 1},
    /* LCM Control, XOR: BGR, MX, MH */
    {0xC0, {0x2c}, 1},
    /* VDV and VRH Command Enable, enable=1 */
    {0xC2, {0x01, 0xff}, 2},
    /* VRH Set, Vap=4.4+... */
    {0xC3, {0x12}, 1},
    /* VDV Set, VDV=0 */
    {0xC4, {0x20}, 1},
    /* Frame Rate Control, 60Hz, inversion=0 */
    {0xC6, {0x0f}, 1},
    /* Power Control 1, AVDD=6.8V, AVCL=-4.8V, VDDS=2.3V */
    {0xD0, {0xA4, 0xA1}, 1},
    /* Positive Voltage Gamma Control */
    {0xE0, {0xd0, 0x04, 0x0d, 0x11, 0x13, 0x2b, 0x3f, 0x54, 0x4c, 0x18, 0x0d, 0x0b, 0x1f, 0x23}, 14},
    /* Negative Voltage Gamma Control */
    {0xE1, {0xd0, 0x04, 0x0c, 0x11, 0x13, 0x2c, 0x3f, 0x44, 0x51, 0x2f, 0x1f, 0x1f, 0x20, 0x23}, 14},
    // /* Positive Voltage Gamma Control */
    // {0xE0, {0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19}, 14},
    // /* Negative Voltage Gamma Control */
    // {0xE1, {0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19}, 14},
    /* Sleep Out */
    {0x11, {0}, 0x80},
    /* Display On */
    {0x29, {0}, 0x80},
    {0, {0}, 0xff}

};





//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}



/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}
static void lcd_wr_reg(uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}



void lcd_wr_data_16bit(uint16_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    uint8_t txdata[2];
    txdata[0] = data >> 8;
    txdata[1] = (uint8_t)data;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=2*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=txdata;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}



// #define LCD_HOST    HSPI_HOST
// #define DMA_CHAN    2

// #define PIN_NUM_MOSI 36
// #define PIN_NUM_CLK  35
// #define PIN_NUM_CS   34

// #define PIN_NUM_DC   37
// #define PIN_NUM_RST  38
// #define PIN_NUM_BCKL 33
//Initialize the display
void lcd_init(void)
{
    //Initialize spi 
    spi_bus_config_t buscfg={
        .miso_io_num=-1,
        .mosi_io_num= PIN_NUM_MOSI,
        .sclk_io_num= PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=RES_VER*RES_HOR*2+8
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = SPI_SPEED_MHz, // Clock out at 26 MHz
        .mode = 0,                               // SPI mode 0
        .spics_io_num = PIN_NUM_CS,              // CS pin
        .queue_size = 7,                         // We want to be able to queue 7 transactions at a time
        .pre_cb = lcd_spi_pre_transfer_callback, // Specify pre-transfer callback to handle D/C line
    };
    devcfg.flags |= SPI_DEVICE_NO_DUMMY;
    //Initialize the SPI bus
    ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);


    
    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);



    int cmd=0;  
    //Send all the commands
    while (st_init_cmds[cmd].databytes!=0xff) {
        lcd_cmd(spi, st_init_cmds[cmd].cmd);
        lcd_data(spi, st_init_cmds[cmd].data, st_init_cmds[cmd].databytes&0x1F);
        if (st_init_cmds[cmd].databytes&0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
        // printf("cmd:%d\r\n", cmd);

    }
    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 0);

    
}



/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void lcd_address_set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	if(USE_HORIZONTAL==0)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+52);
		LCD_WR_DATA(x2+52);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+40);
		LCD_WR_DATA(y2+40);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==1)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+53);
		LCD_WR_DATA(x2+53);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+40);
		LCD_WR_DATA(y2+40);
		LCD_WR_REG(0x2c);//储存器写
	}
	else if(USE_HORIZONTAL==2)
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+40);
		LCD_WR_DATA(x2+40);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+53);
		LCD_WR_DATA(y2+53);
		LCD_WR_REG(0x2c);//储存器写
	}
	else
	{
		LCD_WR_REG(0x2a);//列地址设置
		LCD_WR_DATA(x1+40);
		LCD_WR_DATA(x2+40);
		LCD_WR_REG(0x2b);//行地址设置
		LCD_WR_DATA(y1+52);
		LCD_WR_DATA(y2+52);
		LCD_WR_REG(0x2c);//储存器写
	}
}


/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void lcd_drawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	lcd_address_set(x,y,x,y);//设置光标位置 
	lcd_wr_data_16bit(color);
} 

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void lcd_fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	lcd_address_set(xsta,ysta,xend,yend);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			lcd_wr_data_16bit(color);
		}
	} 					  	    
}




void lcd_xfer(uint8_t *buf,uint32_t px_cnt)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length = px_cnt*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer = buf;               //Data
    t.user = (void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

#ifdef USE_INT_RAM
void lcd_fill_ram(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	lcd_address_set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			lcd_ram[j][i] = color;
		}
	} 					  	    
}

void lcd_xfer_ram(void)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=80*160*2*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=lcd_ram;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}
#endif


/**
 * @brief set blk of screen
 * 
 * @param v 0:off, 1:on
 */
void lcd_set_blk(int v)
{
    gpio_set_level(PIN_NUM_BCKL, v);
}