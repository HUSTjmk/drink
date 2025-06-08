#include "bsp_lcd.h"
#include "fsmc.h"

static SRAM_HandleTypeDef  SRAM_Handler;
//根据液晶扫描方向而变化的XY像素宽度
//调用NT35510_GramScan函数设置方向时会自动更改
uint16_t LCD_X_LENGTH = NT35510_MORE_PIXEL;
uint16_t LCD_Y_LENGTH = NT35510_LESS_PIXEL;

//液晶屏扫描模式，本变量主要用于方便选择触摸屏的计算参数
//参数可选值为0-7
//调用NT35510_GramScan函数设置方向时会自动更改
//LCD刚初始化完成时会使用本默认值
LCD_Scan_Mode lcd_scan_mode = LCD_Scan_Mode_6;


// static sFONT *LCD_Currentfonts = &Font16x32;  //英文字体
static uint16_t CurrentTextColor   = WHITE;//前景色
static uint16_t CurrentBackColor   = BLUE;//背景色

__inline void                 NT35510_Write_Cmd           (uint16_t usCmd);
__inline void                 NT35510_Write_Data          (uint16_t usData);
__inline uint16_t             NT35510_Read_Data           (void);
static void                   NT35510_Delay               (__IO uint32_t nCount);
static void                   NT35510_REG_Config          (void);
static void                   NT35510_SetCursor           (uint16_t usX, uint16_t usY);
static __inline void          NT35510_FillColor           (uint32_t ulAmout_Point, uint16_t usColor);
static uint16_t               NT35510_Read_PixelData      (void);

/**
  * @brief  向NT35510写入命令
  * @param  usCmd :要写入的命令（表寄存器地址）
  * @retval 无
  */	
void NT35510_Write_Cmd (uint16_t usCmd)
{
	* (__IO uint16_t *) (FSMC_Addr_NT35510_CMD) = usCmd;
}


/**
  * @brief  向NT35510写入数据
  * @param  usData :要写入的数据
  * @retval 无
  */	
 void NT35510_Write_Data (uint16_t usData)
{
	* (__IO uint16_t *) (FSMC_Addr_NT35510_DATA) = usData;
}


/**
  * @brief  从NT35510读取数据
  * @param  无
  * @retval 读取到的数据
  */	
 uint16_t NT35510_Read_Data (void)
{
	return (* (__IO uint16_t *) (FSMC_Addr_NT35510_DATA));
	
}


/**
  * @brief  用于 NT35510 简单延时函数
  * @param  nCount ：延时计数值
  * @retval 无
  */	
static void NT35510_Delay (__IO uint32_t nCount)
{
  for (; nCount != 0; nCount --);
	
}

/**
 * @brief  初始化NT35510寄存器
 * @param  无
 * @retval 无
 */
static void NT35510_REG_Config (void)
{	
#if NT35510_HSD43   //NT35510+HSD4.3


    
    NT35510_Write_Cmd(0xF000);    NT35510_Write_Data(0x0055);
    NT35510_Write_Cmd(0xF001);    NT35510_Write_Data(0x00AA);
    NT35510_Write_Cmd(0xF002);    NT35510_Write_Data(0x0052);
    NT35510_Write_Cmd(0xF003);    NT35510_Write_Data(0x0008);
    NT35510_Write_Cmd(0xF004);    NT35510_Write_Data(0x0001);

  //Set AVDD 5.2V
  NT35510_Write_Cmd(0xB000);    NT35510_Write_Data(0x000D);
  NT35510_Write_Cmd(0xB001);    NT35510_Write_Data(0x000D);
  NT35510_Write_Cmd(0xB002);    NT35510_Write_Data(0x000D);

  //Set AVEE 5.2V
  NT35510_Write_Cmd(0xB100);    NT35510_Write_Data(0x000D);
  NT35510_Write_Cmd(0xB101);    NT35510_Write_Data(0x000D);
  NT35510_Write_Cmd(0xB102);    NT35510_Write_Data(0x000D);

  //Set VCL -2.5V
  NT35510_Write_Cmd(0xB200);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xB201);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xB202);    NT35510_Write_Data(0x0000);				

  //Set AVDD Ratio
  NT35510_Write_Cmd(0xB600);    NT35510_Write_Data(0x0044);
  NT35510_Write_Cmd(0xB601);    NT35510_Write_Data(0x0044);
  NT35510_Write_Cmd(0xB602);    NT35510_Write_Data(0x0044);

  //Set AVEE Ratio
  NT35510_Write_Cmd(0xB700);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xB701);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xB702);    NT35510_Write_Data(0x0034);

  //Set VCL -2.5V
  NT35510_Write_Cmd(0xB800);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xB801);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xB802);    NT35510_Write_Data(0x0034);
        
  //Control VGH booster voltage rang
  NT35510_Write_Cmd(0xBF00);    NT35510_Write_Data(0x0001); //VGH:7~18V	

  //VGH=15V(1V/step)	Free pump
  NT35510_Write_Cmd(0xB300);    NT35510_Write_Data(0x000f);		//08
  NT35510_Write_Cmd(0xB301);    NT35510_Write_Data(0x000f);		//08
  NT35510_Write_Cmd(0xB302);    NT35510_Write_Data(0x000f);		//08

  //VGH Ratio
  NT35510_Write_Cmd(0xB900);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xB901);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xB902);    NT35510_Write_Data(0x0034);

  //VGL_REG=-10(1V/step)
  NT35510_Write_Cmd(0xB500);    NT35510_Write_Data(0x0008);
  NT35510_Write_Cmd(0xB501);    NT35510_Write_Data(0x0008);
  NT35510_Write_Cmd(0xB502);    NT35510_Write_Data(0x0008);

  NT35510_Write_Cmd(0xC200);    NT35510_Write_Data(0x0003);

  //VGLX Ratio
  NT35510_Write_Cmd(0xBA00);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xBA01);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xBA02);    NT35510_Write_Data(0x0034);

    //VGMP/VGSP=4.5V/0V
  NT35510_Write_Cmd(0xBC00);    NT35510_Write_Data(0x0000);		//00
  NT35510_Write_Cmd(0xBC01);    NT35510_Write_Data(0x0078);		//C8 =5.5V/90=4.8V
  NT35510_Write_Cmd(0xBC02);    NT35510_Write_Data(0x0000);		//01

  //VGMN/VGSN=-4.5V/0V
  NT35510_Write_Cmd(0xBD00);    NT35510_Write_Data(0x0000); //00
  NT35510_Write_Cmd(0xBD01);    NT35510_Write_Data(0x0078); //90
  NT35510_Write_Cmd(0xBD02);    NT35510_Write_Data(0x0000);

  //Vcom=-1.4V(12.5mV/step)
  NT35510_Write_Cmd(0xBE00);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xBE01);    NT35510_Write_Data(0x0064); //HSD:64;Novatek:50=-1.0V, 80  5f

  //Gamma (R+)
  NT35510_Write_Cmd(0xD100);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD101);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD102);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD103);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD104);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD105);    NT35510_Write_Data(0x003A);
  NT35510_Write_Cmd(0xD106);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD107);    NT35510_Write_Data(0x004A);
  NT35510_Write_Cmd(0xD108);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD109);    NT35510_Write_Data(0x005C);
  NT35510_Write_Cmd(0xD10A);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD10B);    NT35510_Write_Data(0x0081);
  NT35510_Write_Cmd(0xD10C);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD10D);    NT35510_Write_Data(0x00A6);
  NT35510_Write_Cmd(0xD10E);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD10F);    NT35510_Write_Data(0x00E5);
  NT35510_Write_Cmd(0xD110);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD111);    NT35510_Write_Data(0x0013);
  NT35510_Write_Cmd(0xD112);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD113);    NT35510_Write_Data(0x0054);
  NT35510_Write_Cmd(0xD114);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD115);    NT35510_Write_Data(0x0082);
  NT35510_Write_Cmd(0xD116);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD117);    NT35510_Write_Data(0x00CA);
  NT35510_Write_Cmd(0xD118);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD119);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD11A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD11B);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD11C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD11D);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD11E);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD11F);    NT35510_Write_Data(0x0067);
  NT35510_Write_Cmd(0xD120);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD121);    NT35510_Write_Data(0x0084);
  NT35510_Write_Cmd(0xD122);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD123);    NT35510_Write_Data(0x00A4);
  NT35510_Write_Cmd(0xD124);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD125);    NT35510_Write_Data(0x00B7);
  NT35510_Write_Cmd(0xD126);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD127);    NT35510_Write_Data(0x00CF);
  NT35510_Write_Cmd(0xD128);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD129);    NT35510_Write_Data(0x00DE);
  NT35510_Write_Cmd(0xD12A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD12B);    NT35510_Write_Data(0x00F2);
  NT35510_Write_Cmd(0xD12C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD12D);    NT35510_Write_Data(0x00FE);
  NT35510_Write_Cmd(0xD12E);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD12F);    NT35510_Write_Data(0x0010);
  NT35510_Write_Cmd(0xD130);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD131);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD132);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD133);    NT35510_Write_Data(0x006D);

  //Gamma (G+)
  NT35510_Write_Cmd(0xD200);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD201);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD202);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD203);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD204);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD205);    NT35510_Write_Data(0x003A);
  NT35510_Write_Cmd(0xD206);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD207);    NT35510_Write_Data(0x004A);
  NT35510_Write_Cmd(0xD208);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD209);    NT35510_Write_Data(0x005C);
  NT35510_Write_Cmd(0xD20A);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD20B);    NT35510_Write_Data(0x0081);
  NT35510_Write_Cmd(0xD20C);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD20D);    NT35510_Write_Data(0x00A6);
  NT35510_Write_Cmd(0xD20E);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD20F);    NT35510_Write_Data(0x00E5);
  NT35510_Write_Cmd(0xD210);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD211);    NT35510_Write_Data(0x0013);
  NT35510_Write_Cmd(0xD212);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD213);    NT35510_Write_Data(0x0054);
  NT35510_Write_Cmd(0xD214);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD215);    NT35510_Write_Data(0x0082);
  NT35510_Write_Cmd(0xD216);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD217);    NT35510_Write_Data(0x00CA);
  NT35510_Write_Cmd(0xD218);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD219);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD21A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD21B);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD21C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD21D);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD21E);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD21F);    NT35510_Write_Data(0x0067);
  NT35510_Write_Cmd(0xD220);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD221);    NT35510_Write_Data(0x0084);
  NT35510_Write_Cmd(0xD222);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD223);    NT35510_Write_Data(0x00A4);
  NT35510_Write_Cmd(0xD224);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD225);    NT35510_Write_Data(0x00B7);
  NT35510_Write_Cmd(0xD226);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD227);    NT35510_Write_Data(0x00CF);
  NT35510_Write_Cmd(0xD228);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD229);    NT35510_Write_Data(0x00DE);
  NT35510_Write_Cmd(0xD22A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD22B);    NT35510_Write_Data(0x00F2);
  NT35510_Write_Cmd(0xD22C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD22D);    NT35510_Write_Data(0x00FE);
  NT35510_Write_Cmd(0xD22E);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD22F);    NT35510_Write_Data(0x0010);
  NT35510_Write_Cmd(0xD230);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD231);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD232);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD233);    NT35510_Write_Data(0x006D);

  //Gamma (B+)
  NT35510_Write_Cmd(0xD300);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD301);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD302);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD303);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD304);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD305);    NT35510_Write_Data(0x003A);
  NT35510_Write_Cmd(0xD306);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD307);    NT35510_Write_Data(0x004A);
  NT35510_Write_Cmd(0xD308);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD309);    NT35510_Write_Data(0x005C);
  NT35510_Write_Cmd(0xD30A);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD30B);    NT35510_Write_Data(0x0081);
  NT35510_Write_Cmd(0xD30C);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD30D);    NT35510_Write_Data(0x00A6);
  NT35510_Write_Cmd(0xD30E);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD30F);    NT35510_Write_Data(0x00E5);
  NT35510_Write_Cmd(0xD310);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD311);    NT35510_Write_Data(0x0013);
  NT35510_Write_Cmd(0xD312);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD313);    NT35510_Write_Data(0x0054);
  NT35510_Write_Cmd(0xD314);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD315);    NT35510_Write_Data(0x0082);
  NT35510_Write_Cmd(0xD316);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD317);    NT35510_Write_Data(0x00CA);
  NT35510_Write_Cmd(0xD318);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD319);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD31A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD31B);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD31C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD31D);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD31E);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD31F);    NT35510_Write_Data(0x0067);
  NT35510_Write_Cmd(0xD320);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD321);    NT35510_Write_Data(0x0084);
  NT35510_Write_Cmd(0xD322);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD323);    NT35510_Write_Data(0x00A4);
  NT35510_Write_Cmd(0xD324);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD325);    NT35510_Write_Data(0x00B7);
  NT35510_Write_Cmd(0xD326);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD327);    NT35510_Write_Data(0x00CF);
  NT35510_Write_Cmd(0xD328);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD329);    NT35510_Write_Data(0x00DE);
  NT35510_Write_Cmd(0xD32A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD32B);    NT35510_Write_Data(0x00F2);
  NT35510_Write_Cmd(0xD32C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD32D);    NT35510_Write_Data(0x00FE);
  NT35510_Write_Cmd(0xD32E);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD32F);    NT35510_Write_Data(0x0010);
  NT35510_Write_Cmd(0xD330);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD331);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD332);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD333);    NT35510_Write_Data(0x006D);

  //Gamma (R-)
  NT35510_Write_Cmd(0xD400);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD401);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD402);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD403);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD404);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD405);    NT35510_Write_Data(0x003A);
  NT35510_Write_Cmd(0xD406);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD407);    NT35510_Write_Data(0x004A);
  NT35510_Write_Cmd(0xD408);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD409);    NT35510_Write_Data(0x005C);
  NT35510_Write_Cmd(0xD40A);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD40B);    NT35510_Write_Data(0x0081);
  NT35510_Write_Cmd(0xD40C);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD40D);    NT35510_Write_Data(0x00A6);
  NT35510_Write_Cmd(0xD40E);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD40F);    NT35510_Write_Data(0x00E5);
  NT35510_Write_Cmd(0xD410);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD411);    NT35510_Write_Data(0x0013);
  NT35510_Write_Cmd(0xD412);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD413);    NT35510_Write_Data(0x0054);
  NT35510_Write_Cmd(0xD414);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD415);    NT35510_Write_Data(0x0082);
  NT35510_Write_Cmd(0xD416);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD417);    NT35510_Write_Data(0x00CA);
  NT35510_Write_Cmd(0xD418);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD419);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD41A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD41B);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD41C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD41D);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD41E);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD41F);    NT35510_Write_Data(0x0067);
  NT35510_Write_Cmd(0xD420);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD421);    NT35510_Write_Data(0x0084);
  NT35510_Write_Cmd(0xD422);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD423);    NT35510_Write_Data(0x00A4);
  NT35510_Write_Cmd(0xD424);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD425);    NT35510_Write_Data(0x00B7);
  NT35510_Write_Cmd(0xD426);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD427);    NT35510_Write_Data(0x00CF);
  NT35510_Write_Cmd(0xD428);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD429);    NT35510_Write_Data(0x00DE);
  NT35510_Write_Cmd(0xD42A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD42B);    NT35510_Write_Data(0x00F2);
  NT35510_Write_Cmd(0xD42C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD42D);    NT35510_Write_Data(0x00FE);
  NT35510_Write_Cmd(0xD42E);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD42F);    NT35510_Write_Data(0x0010);
  NT35510_Write_Cmd(0xD430);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD431);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD432);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD433);    NT35510_Write_Data(0x006D);

  //Gamma (G-)
  NT35510_Write_Cmd(0xD500);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD501);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD502);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD503);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD504);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD505);    NT35510_Write_Data(0x003A);
  NT35510_Write_Cmd(0xD506);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD507);    NT35510_Write_Data(0x004A);
  NT35510_Write_Cmd(0xD508);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD509);    NT35510_Write_Data(0x005C);
  NT35510_Write_Cmd(0xD50A);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD50B);    NT35510_Write_Data(0x0081);
  NT35510_Write_Cmd(0xD50C);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD50D);    NT35510_Write_Data(0x00A6);
  NT35510_Write_Cmd(0xD50E);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD50F);    NT35510_Write_Data(0x00E5);
  NT35510_Write_Cmd(0xD510);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD511);    NT35510_Write_Data(0x0013);
  NT35510_Write_Cmd(0xD512);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD513);    NT35510_Write_Data(0x0054);
  NT35510_Write_Cmd(0xD514);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD515);    NT35510_Write_Data(0x0082);
  NT35510_Write_Cmd(0xD516);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD517);    NT35510_Write_Data(0x00CA);
  NT35510_Write_Cmd(0xD518);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD519);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD51A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD51B);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD51C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD51D);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD51E);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD51F);    NT35510_Write_Data(0x0067);
  NT35510_Write_Cmd(0xD520);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD521);    NT35510_Write_Data(0x0084);
  NT35510_Write_Cmd(0xD522);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD523);    NT35510_Write_Data(0x00A4);
  NT35510_Write_Cmd(0xD524);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD525);    NT35510_Write_Data(0x00B7);
  NT35510_Write_Cmd(0xD526);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD527);    NT35510_Write_Data(0x00CF);
  NT35510_Write_Cmd(0xD528);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD529);    NT35510_Write_Data(0x00DE);
  NT35510_Write_Cmd(0xD52A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD52B);    NT35510_Write_Data(0x00F2);
  NT35510_Write_Cmd(0xD52C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD52D);    NT35510_Write_Data(0x00FE);
  NT35510_Write_Cmd(0xD52E);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD52F);    NT35510_Write_Data(0x0010);
  NT35510_Write_Cmd(0xD530);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD531);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD532);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD533);    NT35510_Write_Data(0x006D);

  //Gamma (B-)
  NT35510_Write_Cmd(0xD600);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD601);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD602);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD603);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD604);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD605);    NT35510_Write_Data(0x003A);
  NT35510_Write_Cmd(0xD606);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD607);    NT35510_Write_Data(0x004A);
  NT35510_Write_Cmd(0xD608);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD609);    NT35510_Write_Data(0x005C);
  NT35510_Write_Cmd(0xD60A);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD60B);    NT35510_Write_Data(0x0081);
  NT35510_Write_Cmd(0xD60C);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD60D);    NT35510_Write_Data(0x00A6);
  NT35510_Write_Cmd(0xD60E);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD60F);    NT35510_Write_Data(0x00E5);
  NT35510_Write_Cmd(0xD610);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD611);    NT35510_Write_Data(0x0013);
  NT35510_Write_Cmd(0xD612);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD613);    NT35510_Write_Data(0x0054);
  NT35510_Write_Cmd(0xD614);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD615);    NT35510_Write_Data(0x0082);
  NT35510_Write_Cmd(0xD616);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD617);    NT35510_Write_Data(0x00CA);
  NT35510_Write_Cmd(0xD618);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD619);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0xD61A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD61B);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xD61C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD61D);    NT35510_Write_Data(0x0034);
  NT35510_Write_Cmd(0xD61E);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD61F);    NT35510_Write_Data(0x0067);
  NT35510_Write_Cmd(0xD620);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD621);    NT35510_Write_Data(0x0084);
  NT35510_Write_Cmd(0xD622);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD623);    NT35510_Write_Data(0x00A4);
  NT35510_Write_Cmd(0xD624);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD625);    NT35510_Write_Data(0x00B7);
  NT35510_Write_Cmd(0xD626);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD627);    NT35510_Write_Data(0x00CF);
  NT35510_Write_Cmd(0xD628);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD629);    NT35510_Write_Data(0x00DE);
  NT35510_Write_Cmd(0xD62A);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD62B);    NT35510_Write_Data(0x00F2);
  NT35510_Write_Cmd(0xD62C);    NT35510_Write_Data(0x0002);
  NT35510_Write_Cmd(0xD62D);    NT35510_Write_Data(0x00FE);
  NT35510_Write_Cmd(0xD62E);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD62F);    NT35510_Write_Data(0x0010);
  NT35510_Write_Cmd(0xD630);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD631);    NT35510_Write_Data(0x0033);
  NT35510_Write_Cmd(0xD632);    NT35510_Write_Data(0x0003);
  NT35510_Write_Cmd(0xD633);    NT35510_Write_Data(0x006D);

  //PAGE0
  NT35510_Write_Cmd(0xF000);    NT35510_Write_Data(0x0055);
  NT35510_Write_Cmd(0xF001);    NT35510_Write_Data(0x00AA);
  NT35510_Write_Cmd(0xF002);    NT35510_Write_Data(0x0052);
  NT35510_Write_Cmd(0xF003);    NT35510_Write_Data(0x0008);	
  NT35510_Write_Cmd(0xF004);    NT35510_Write_Data(0x0000); 

  //480x800
  NT35510_Write_Cmd(0xB500);    NT35510_Write_Data(0x0050);

  //NT35510_Write_Cmd(0x2C00);    NT35510_Write_Data(0x0006); //8BIT 6-6-6?

  //Dispay control
  NT35510_Write_Cmd(0xB100);    NT35510_Write_Data(0x00CC);	
  NT35510_Write_Cmd(0xB101);    NT35510_Write_Data(0x0000); // S1->S1440:00;S1440->S1:02

  //Source hold time (Nova non-used)
  NT35510_Write_Cmd(0xB600);    NT35510_Write_Data(0x0005);

  //Gate EQ control	 (Nova non-used)
  NT35510_Write_Cmd(0xB700);    NT35510_Write_Data(0x0077);  //HSD:70;Nova:77	 
  NT35510_Write_Cmd(0xB701);    NT35510_Write_Data(0x0077);	//HSD:70;Nova:77

  //Source EQ control (Nova non-used)
  NT35510_Write_Cmd(0xB800);    NT35510_Write_Data(0x0001);  
  NT35510_Write_Cmd(0xB801);    NT35510_Write_Data(0x0003);	//HSD:05;Nova:07
  NT35510_Write_Cmd(0xB802);    NT35510_Write_Data(0x0003);	//HSD:05;Nova:07
  NT35510_Write_Cmd(0xB803);    NT35510_Write_Data(0x0003);	//HSD:05;Nova:07

  //Inversion mode: column
  NT35510_Write_Cmd(0xBC00);    NT35510_Write_Data(0x0002);	//00: column
  NT35510_Write_Cmd(0xBC01);    NT35510_Write_Data(0x0000);	//01:1dot
  NT35510_Write_Cmd(0xBC02);    NT35510_Write_Data(0x0000); 

  //Frame rate	(Nova non-used)
  NT35510_Write_Cmd(0xBD00);    NT35510_Write_Data(0x0001);
  NT35510_Write_Cmd(0xBD01);    NT35510_Write_Data(0x0084);
  NT35510_Write_Cmd(0xBD02);    NT35510_Write_Data(0x001c); //HSD:06;Nova:1C
  NT35510_Write_Cmd(0xBD03);    NT35510_Write_Data(0x001c); //HSD:04;Nova:1C
  NT35510_Write_Cmd(0xBD04);    NT35510_Write_Data(0x0000);

  //LGD timing control(4H/4-NT35510_Delay)
  NT35510_Write_Cmd(0xC900);    NT35510_Write_Data(0x00D0);	//3H:0x50;4H:0xD0	 //D
  NT35510_Write_Cmd(0xC901);    NT35510_Write_Data(0x0002);  //HSD:05;Nova:02
  NT35510_Write_Cmd(0xC902);    NT35510_Write_Data(0x0050);	//HSD:05;Nova:50
  NT35510_Write_Cmd(0xC903);    NT35510_Write_Data(0x0050);	//HSD:05;Nova:50	;STV NT35510_Delay time
  NT35510_Write_Cmd(0xC904);    NT35510_Write_Data(0x0050);	//HSD:05;Nova:50	;CLK NT35510_Delay time

  NT35510_Write_Cmd(0x3600);    NT35510_Write_Data(0x0000);
  NT35510_Write_Cmd(0x3500);    NT35510_Write_Data(0x0000);

  NT35510_Write_Cmd(0xFF00);    NT35510_Write_Data(0x00AA);
  NT35510_Write_Cmd(0xFF01);    NT35510_Write_Data(0x0055);
  NT35510_Write_Cmd(0xFF02);    NT35510_Write_Data(0x0025);
  NT35510_Write_Cmd(0xFF03);    NT35510_Write_Data(0x0001);

  NT35510_Write_Cmd(0xFC00);    NT35510_Write_Data(0x0016);
  NT35510_Write_Cmd(0xFC01);    NT35510_Write_Data(0x00A2);
  NT35510_Write_Cmd(0xFC02);    NT35510_Write_Data(0x0026);
  NT35510_Write_Cmd(0x3A00);    NT35510_Write_Data(0x0006);

  NT35510_Write_Cmd(0x3A00);    NT35510_Write_Data(0x0055);
  //Sleep out
  NT35510_Write_Cmd(0x1100);	   //?
  NT35510_Delay(1600);

  //Display on
  NT35510_Write_Cmd(0x2900);   

 
#elif OTM8009A_HSD43  //OTM8009A+HSD4.3
    NT35510_Write_Cmd(0xFF00); NT35510_Write_Data(0x80);
    NT35510_Write_Cmd(0xFF01); NT35510_Write_Data(0x09);
    NT35510_Write_Cmd(0xFF02); NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0xFF80); NT35510_Write_Data(0x80);
    NT35510_Write_Cmd(0xFF81); NT35510_Write_Data(0x09);
    NT35510_Write_Cmd(0xFF03); NT35510_Write_Data(0x01);  

    NT35510_Write_Cmd(0xC0B4); NT35510_Write_Data(0x10);
    NT35510_Write_Cmd(0xC489); NT35510_Write_Data(0x08);
    NT35510_Write_Cmd(0xC0A3); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xC582); NT35510_Write_Data(0xA3);
    NT35510_Write_Cmd(0xC590); NT35510_Write_Data(0xD6);
    NT35510_Write_Cmd(0xC591); NT35510_Write_Data(0x87);

    NT35510_Write_Cmd(0xD800); NT35510_Write_Data(0x74);
    NT35510_Write_Cmd(0xD801); NT35510_Write_Data(0x72);

    NT35510_Write_Cmd(0xD900); NT35510_Write_Data(0x60);	

    NT35510_Write_Cmd(0xE100); NT35510_Write_Data(0x09);	
    NT35510_Write_Cmd(0xE101); NT35510_Write_Data(0x0C);	
    NT35510_Write_Cmd(0xE102); NT35510_Write_Data(0x12);	
    NT35510_Write_Cmd(0xE103); NT35510_Write_Data(0x0E);	
    NT35510_Write_Cmd(0xE104); NT35510_Write_Data(0x08);	
    NT35510_Write_Cmd(0xE105); NT35510_Write_Data(0x19);	
    NT35510_Write_Cmd(0xE106); NT35510_Write_Data(0x0C);	
    NT35510_Write_Cmd(0xE107); NT35510_Write_Data(0x0B);	
    NT35510_Write_Cmd(0xE108); NT35510_Write_Data(0x01);	
    NT35510_Write_Cmd(0xE109); NT35510_Write_Data(0x05);	
    NT35510_Write_Cmd(0xE10A); NT35510_Write_Data(0x03);	
    NT35510_Write_Cmd(0xE10B); NT35510_Write_Data(0x07);	
    NT35510_Write_Cmd(0xE10C); NT35510_Write_Data(0x0E);	
    NT35510_Write_Cmd(0xE10D); NT35510_Write_Data(0x26);	
    NT35510_Write_Cmd(0xE10E); NT35510_Write_Data(0x23);	
    NT35510_Write_Cmd(0xE10F); NT35510_Write_Data(0x1B); 

    NT35510_Write_Cmd(0xE200); NT35510_Write_Data(0x09);
    NT35510_Write_Cmd(0xE201); NT35510_Write_Data(0x0C);
    NT35510_Write_Cmd(0xE202); NT35510_Write_Data(0x12);
    NT35510_Write_Cmd(0xE203); NT35510_Write_Data(0x0E);
    NT35510_Write_Cmd(0xE204); NT35510_Write_Data(0x08);
    NT35510_Write_Cmd(0xE205); NT35510_Write_Data(0x19);
    NT35510_Write_Cmd(0xE206); NT35510_Write_Data(0x0C);
    NT35510_Write_Cmd(0xE207); NT35510_Write_Data(0x0B);
    NT35510_Write_Cmd(0xE208); NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0xE209); NT35510_Write_Data(0x05);
    NT35510_Write_Cmd(0xE20A); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xE20B); NT35510_Write_Data(0x07);
    NT35510_Write_Cmd(0xE20C); NT35510_Write_Data(0x0E);
    NT35510_Write_Cmd(0xE20D); NT35510_Write_Data(0x26);
    NT35510_Write_Cmd(0xE20E); NT35510_Write_Data(0x23);
    NT35510_Write_Cmd(0xE20F); NT35510_Write_Data(0x1B);

    NT35510_Write_Cmd(0xC181); NT35510_Write_Data(0x66);
    NT35510_Write_Cmd(0xC1A1); NT35510_Write_Data(0x08);
    NT35510_Write_Cmd(0xC481); NT35510_Write_Data(0x83);
    NT35510_Write_Cmd(0xC592); NT35510_Write_Data(0x01);  
    NT35510_Write_Cmd(0xC5B1); NT35510_Write_Data(0xA9);
    NT35510_Write_Cmd(0xCE80); NT35510_Write_Data(0x85);

    NT35510_Write_Cmd(0xCE81); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCE82); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCE83); NT35510_Write_Data(0x84);
    NT35510_Write_Cmd(0xCE84); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCE85); NT35510_Write_Data(0x00);	                       
    NT35510_Write_Cmd(0xCE86); NT35510_Write_Data(0x83);	                      
    NT35510_Write_Cmd(0xCE87); NT35510_Write_Data(0x03);	                       
    NT35510_Write_Cmd(0xCE88); NT35510_Write_Data(0x00);	                       
    NT35510_Write_Cmd(0xCE89); NT35510_Write_Data(0x82);
    NT35510_Write_Cmd(0xCE8A); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCE8B); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEA0); NT35510_Write_Data(0x38);
    NT35510_Write_Cmd(0xCEA1); NT35510_Write_Data(0x02);
    NT35510_Write_Cmd(0xCEA2); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCEA3); NT35510_Write_Data(0x21);
    NT35510_Write_Cmd(0xCEA4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEA5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEA6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEA7); NT35510_Write_Data(0x38);
    NT35510_Write_Cmd(0xCEA8); NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0xCEA9); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCEAA); NT35510_Write_Data(0x22);
    NT35510_Write_Cmd(0xCEAB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEAC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEAD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEB0); NT35510_Write_Data(0x38);
    NT35510_Write_Cmd(0xCEB1); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEB2); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCEB3); NT35510_Write_Data(0x23);
    NT35510_Write_Cmd(0xCEB4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEB5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEB6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEB7); NT35510_Write_Data(0x30);
    NT35510_Write_Cmd(0xCEB8); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEB9); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCEBA); NT35510_Write_Data(0x24);
    NT35510_Write_Cmd(0xCEBB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEBC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEBD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEC0); NT35510_Write_Data(0x30);
    NT35510_Write_Cmd(0xCEC1); NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0xCEC2); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCEC3); NT35510_Write_Data(0x25);
    NT35510_Write_Cmd(0xCEC4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEC5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEC6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEC7); NT35510_Write_Data(0x30);
    NT35510_Write_Cmd(0xCEC8); NT35510_Write_Data(0x02);
    NT35510_Write_Cmd(0xCEC9); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCECA); NT35510_Write_Data(0x26);
    NT35510_Write_Cmd(0xCECB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCECC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCECD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCED0); NT35510_Write_Data(0x30);
    NT35510_Write_Cmd(0xCED1); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCED2); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCED3); NT35510_Write_Data(0x27);
    NT35510_Write_Cmd(0xCED4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCED5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCED6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCED7); NT35510_Write_Data(0x30);
    NT35510_Write_Cmd(0xCED8); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCED9); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCEDA); NT35510_Write_Data(0x28);
    NT35510_Write_Cmd(0xCEDB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEDC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCEDD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC0); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC1); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC2); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC3); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC7); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC8); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFC9); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCFD0); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBC0); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBC1); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBC2); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBC3); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBC4); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBC5); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBC6); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBC7); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBC8); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBC9); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBCA); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBCB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBCC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBCD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBCE); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD0); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD1); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD2); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD3); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD7); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD8); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBD9); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBDA); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBDB); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBDC); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBDD); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBDE); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCBE0); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE1); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE2); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE3); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE7); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE8); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCBE9); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC80); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC81); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC82); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC83); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC84); NT35510_Write_Data(0x0C);
    NT35510_Write_Cmd(0xCC85); NT35510_Write_Data(0x0A);
    NT35510_Write_Cmd(0xCC86); NT35510_Write_Data(0x10);
    NT35510_Write_Cmd(0xCC87); NT35510_Write_Data(0x0E);
    NT35510_Write_Cmd(0xCC88); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCC89); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCC90); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC91); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC92); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC93); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC94); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC95); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC96); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC97); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC98); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC99); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC9A); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC9B); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC9C); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC9D); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCC9E); NT35510_Write_Data(0x0B);
    NT35510_Write_Cmd(0xCCA0); NT35510_Write_Data(0x09);
    NT35510_Write_Cmd(0xCCA1); NT35510_Write_Data(0x0F);
    NT35510_Write_Cmd(0xCCA2); NT35510_Write_Data(0x0D);
    NT35510_Write_Cmd(0xCCA3); NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0xCCA4); NT35510_Write_Data(0x02);
    NT35510_Write_Cmd(0xCCA5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCA6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCA7); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCA8); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCA9); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCAA); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCAB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCAC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCAD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCAE); NT35510_Write_Data(0x00);	
    NT35510_Write_Cmd(0xCCB0); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCB1); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCB2); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCB3); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCB4); NT35510_Write_Data(0x0D);
    NT35510_Write_Cmd(0xCCB5); NT35510_Write_Data(0x0F);
    NT35510_Write_Cmd(0xCCB6); NT35510_Write_Data(0x09);
    NT35510_Write_Cmd(0xCCB7); NT35510_Write_Data(0x0B);
    NT35510_Write_Cmd(0xCCB8); NT35510_Write_Data(0x02);
    NT35510_Write_Cmd(0xCCB9); NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0xCCC0); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC1); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC2); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC3); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC4); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC7); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC8); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCC9); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCCA); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCCB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCCC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCCD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCCE); NT35510_Write_Data(0x0E);
    NT35510_Write_Cmd(0xCCD0); NT35510_Write_Data(0x10);
    NT35510_Write_Cmd(0xCCD1); NT35510_Write_Data(0x0A);
    NT35510_Write_Cmd(0xCCD2); NT35510_Write_Data(0x0C);
    NT35510_Write_Cmd(0xCCD3); NT35510_Write_Data(0x04);
    NT35510_Write_Cmd(0xCCD4); NT35510_Write_Data(0x03);
    NT35510_Write_Cmd(0xCCD5); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCD6); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCD7); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCD8); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCD9); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCDA); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCDB); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCDC); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCDD); NT35510_Write_Data(0x00);
    NT35510_Write_Cmd(0xCCDE); NT35510_Write_Data(0x00);

    NT35510_Write_Cmd(0xB391); NT35510_Write_Data(0x00);
    //NT35510_Write_Cmd(0xB392); NT35510_Write_Data(0x20);//BGR

    NT35510_Write_Cmd(0xFF00); NT35510_Write_Data(0xFF); 
    NT35510_Write_Cmd(0xFF01); NT35510_Write_Data(0xFF); 
    NT35510_Write_Cmd(0xFF02); NT35510_Write_Data(0xFF); 


    NT35510_Write_Cmd(0x3500);  NT35510_Write_Data(0x00);// TE(FmArk) SignAl On
    NT35510_Write_Cmd(0x4400);  NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0x4401);  NT35510_Write_Data(0xFF);// TE(FmArk) SignAl Output Position

    NT35510_Write_Cmd(0x3600);  NT35510_Write_Data(0x08);// DisplAy DirECtion 0
    NT35510_Write_Cmd(0x3500);  NT35510_Write_Data(0x00);// TE(FmArk) SignAl On
    NT35510_Write_Cmd(0x4400);  NT35510_Write_Data(0x01);
    NT35510_Write_Cmd(0x4401);  NT35510_Write_Data(0x22);// TE(FmArk) SignAl Output Position

    NT35510_Write_Cmd(0x5100);  NT35510_Write_Data(0xFF);// BACklight LEvEl Control
    NT35510_Write_Cmd(0x5300);  NT35510_Write_Data(0x2C);// BACklight On
    NT35510_Write_Cmd(0x5500);  NT35510_Write_Data(0x00);// CABC FunCtion OFF
    NT35510_Write_Cmd(0x3600);  NT35510_Write_Data(0x00);// DisplAy DirECtion 180

    NT35510_Write_Cmd(0x3A00); NT35510_Write_Data(0x55);	
    NT35510_Write_Cmd(0x1100);
    NT35510_Delay(150);	
    NT35510_Write_Cmd(0x2900);
    NT35510_Delay(20);



#elif ILI9806_BOE397IPS      //ILI9806_BOE3.97IPS     
    NT35510_Write_Cmd(0xFF);
    NT35510_Write_Data(0xFF);
    NT35510_Write_Data(0x98);
    NT35510_Write_Data(0x06);
    NT35510_Write_Cmd(0xBA);
    NT35510_Write_Data(0x60);

    NT35510_Write_Cmd(0xBC);
    NT35510_Write_Data(0x01);
    NT35510_Write_Data(0x12);
    NT35510_Write_Data(0x61);
    NT35510_Write_Data(0xFF);
    NT35510_Write_Data(0x10);
    NT35510_Write_Data(0x10);
    NT35510_Write_Data(0x0B);
    NT35510_Write_Data(0x13);
    NT35510_Write_Data(0x32);
    NT35510_Write_Data(0x73);
    NT35510_Write_Data(0xFF);
    NT35510_Write_Data(0xFF);
    NT35510_Write_Data(0x0E);
    NT35510_Write_Data(0x0E);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x03);
    NT35510_Write_Data(0x66);
    NT35510_Write_Data(0x63);
    NT35510_Write_Data(0x01);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x00);

    NT35510_Write_Cmd(0xBD);
    NT35510_Write_Data(0x01);
    NT35510_Write_Data(0x23);
    NT35510_Write_Data(0x45);
    NT35510_Write_Data(0x67);
    NT35510_Write_Data(0x01);
    NT35510_Write_Data(0x23);
    NT35510_Write_Data(0x45);
    NT35510_Write_Data(0x67);

    NT35510_Write_Cmd(0xBE);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x21);
    NT35510_Write_Data(0xAB);
    NT35510_Write_Data(0x60);
    NT35510_Write_Data(0x22);
    NT35510_Write_Data(0x22);
    NT35510_Write_Data(0x22);
    NT35510_Write_Data(0x22);
    NT35510_Write_Data(0x22);

    NT35510_Write_Cmd(0xC7);
    NT35510_Write_Data(0x5E);
    NT35510_Write_Data(0x80);

    NT35510_Write_Cmd(0xED);
    NT35510_Write_Data(0x7F);
    NT35510_Write_Data(0x0F);
    NT35510_Write_Data(0x00);

    NT35510_Write_Cmd(0x3A);
    NT35510_Write_Data(0x55);

    NT35510_Write_Cmd(0xB5);
    NT35510_Write_Data(0x3E);
    NT35510_Write_Data(0x18);

    NT35510_Write_Cmd(0xC0);
    NT35510_Write_Data(0xAB);
    NT35510_Write_Data(0x0B);
    NT35510_Write_Data(0x0A);

    NT35510_Write_Cmd(0xFC);
    NT35510_Write_Data(0x09);

    NT35510_Write_Cmd(0xDF);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x20);

    NT35510_Write_Cmd(0xF3);
    NT35510_Write_Data(0x74);

    NT35510_Write_Cmd(0xB4);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x00);
    
    NT35510_Write_Cmd(0xF7);
    NT35510_Write_Data(0x82);
    NT35510_Write_Cmd(0xB1);
    
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x12);
    NT35510_Write_Data(0x13);
    
    NT35510_Write_Cmd(0xF2);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x59);
    NT35510_Write_Data(0x40);
    NT35510_Write_Data(0x28);
    
//NT35510_Write_Cmd(0xB7);NT35510_Write_Data(0x1);NT35510_Write_Data(0x0E);
            
    NT35510_Write_Cmd(0xC1);
    NT35510_Write_Data(0x07);
    NT35510_Write_Data(0x80);
    NT35510_Write_Data(0x80);
    NT35510_Write_Data(0x20);

    NT35510_Write_Cmd(0xE0);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x17);
    NT35510_Write_Data(0x1A);
    NT35510_Write_Data(0x0D);
    NT35510_Write_Data(0x0E);
    NT35510_Write_Data(0x0B);
    NT35510_Write_Data(0x07);
    NT35510_Write_Data(0x05);
    NT35510_Write_Data(0x05);
    NT35510_Write_Data(0x09);
    NT35510_Write_Data(0x0E);
    NT35510_Write_Data(0x0F);
    NT35510_Write_Data(0x0D);
    NT35510_Write_Data(0x1D);
    NT35510_Write_Data(0x1A);
    NT35510_Write_Data(0x00);
    
    NT35510_Write_Cmd(0xE1);
    NT35510_Write_Data(0x00);
    NT35510_Write_Data(0x06);
    NT35510_Write_Data(0x0E);
    NT35510_Write_Data(0x0D);
    NT35510_Write_Data(0x0E);
    NT35510_Write_Data(0x0D);
    NT35510_Write_Data(0x06);
    NT35510_Write_Data(0x06);
    NT35510_Write_Data(0x05);
    NT35510_Write_Data(0x09);
    NT35510_Write_Data(0x0D);
    NT35510_Write_Data(0x0E);
    NT35510_Write_Data(0x0D);
    NT35510_Write_Data(0x1F);
    NT35510_Write_Data(0x1D);
    NT35510_Write_Data(0x00);


    NT35510_Write_Cmd(0x35); //Tearing Effect ON 
    NT35510_Write_Data(0x00); 

    NT35510_Write_Cmd(0x3A);
    NT35510_Write_Data(0x55);

    NT35510_Write_Cmd(0x11);

    NT35510_Delay(160);
    NT35510_Write_Cmd(0x29);
    NT35510_Delay(20);
    
#else	
// #elif ILI9806_BOE397IPS     //ILI9806_BOE3.97IPS     
//     NT35510_Write_Cmd(0xFF);
//     NT35510_Write_Data(0xFF);
//     NT35510_Write_Data(0x98);
//     NT35510_Write_Data(0x06);
//     NT35510_Write_Cmd(0xBA);
//     NT35510_Write_Data(0x60);

//     NT35510_Write_Cmd(0xBC);
//     NT35510_Write_Data(0x01);
//     NT35510_Write_Data(0x12);
//     NT35510_Write_Data(0x61);
//     NT35510_Write_Data(0xFF);
//     NT35510_Write_Data(0x10);
//     NT35510_Write_Data(0x10);
//     NT35510_Write_Data(0x0B);
//     NT35510_Write_Data(0x13);
//     NT35510_Write_Data(0x32);
//     NT35510_Write_Data(0x73);
//     NT35510_Write_Data(0xFF);
//     NT35510_Write_Data(0xFF);
//     NT35510_Write_Data(0x0E);
//     NT35510_Write_Data(0x0E);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x03);
//     NT35510_Write_Data(0x66);
//     NT35510_Write_Data(0x63);
//     NT35510_Write_Data(0x01);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x00);

//     NT35510_Write_Cmd(0xBD);
//     NT35510_Write_Data(0x01);
//     NT35510_Write_Data(0x23);
//     NT35510_Write_Data(0x45);
//     NT35510_Write_Data(0x67);
//     NT35510_Write_Data(0x01);
//     NT35510_Write_Data(0x23);
//     NT35510_Write_Data(0x45);
//     NT35510_Write_Data(0x67);

//     NT35510_Write_Cmd(0xBE);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x21);
//     NT35510_Write_Data(0xAB);
//     NT35510_Write_Data(0x60);
//     NT35510_Write_Data(0x22);
//     NT35510_Write_Data(0x22);
//     NT35510_Write_Data(0x22);
//     NT35510_Write_Data(0x22);
//     NT35510_Write_Data(0x22);

//     NT35510_Write_Cmd(0xC7);
//     NT35510_Write_Data(0x5E);
//     NT35510_Write_Data(0x80);

//     NT35510_Write_Cmd(0xED);
//     NT35510_Write_Data(0x7F);
//     NT35510_Write_Data(0x0F);
//     NT35510_Write_Data(0x00);

//     NT35510_Write_Cmd(0x3A);
//     NT35510_Write_Data(0x55);

//     NT35510_Write_Cmd(0xB5);
//     NT35510_Write_Data(0x3E);
//     NT35510_Write_Data(0x18);

//     NT35510_Write_Cmd(0xC0);
//     NT35510_Write_Data(0xAB);
//     NT35510_Write_Data(0x0B);
//     NT35510_Write_Data(0x0A);

//     NT35510_Write_Cmd(0xFC);
//     NT35510_Write_Data(0x09);

//     NT35510_Write_Cmd(0xDF);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x20);

//     NT35510_Write_Cmd(0xF3);
//     NT35510_Write_Data(0x74);

//     NT35510_Write_Cmd(0xB4);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x00);
    
//     NT35510_Write_Cmd(0xF7);
//     NT35510_Write_Data(0x82);
//     NT35510_Write_Cmd(0xB1);
    
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x12);
//     NT35510_Write_Data(0x13);
    
//     NT35510_Write_Cmd(0xF2);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x59);
//     NT35510_Write_Data(0x40);
//     NT35510_Write_Data(0x28);
    
// //NT35510_Write_Cmd(0xB7);NT35510_Write_Data(0x1);NT35510_Write_Data(0x0E);
            
//     NT35510_Write_Cmd(0xC1);
//     NT35510_Write_Data(0x07);
//     NT35510_Write_Data(0x80);
//     NT35510_Write_Data(0x80);
//     NT35510_Write_Data(0x20);

//     NT35510_Write_Cmd(0xE0);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x17);
//     NT35510_Write_Data(0x1A);
//     NT35510_Write_Data(0x0D);
//     NT35510_Write_Data(0x0E);
//     NT35510_Write_Data(0x0B);
//     NT35510_Write_Data(0x07);
//     NT35510_Write_Data(0x05);
//     NT35510_Write_Data(0x05);
//     NT35510_Write_Data(0x09);
//     NT35510_Write_Data(0x0E);
//     NT35510_Write_Data(0x0F);
//     NT35510_Write_Data(0x0D);
//     NT35510_Write_Data(0x1D);
//     NT35510_Write_Data(0x1A);
//     NT35510_Write_Data(0x00);
    
//     NT35510_Write_Cmd(0xE1);
//     NT35510_Write_Data(0x00);
//     NT35510_Write_Data(0x06);
//     NT35510_Write_Data(0x0E);
//     NT35510_Write_Data(0x0D);
//     NT35510_Write_Data(0x0E);
//     NT35510_Write_Data(0x0D);
//     NT35510_Write_Data(0x06);
//     NT35510_Write_Data(0x06);
//     NT35510_Write_Data(0x05);
//     NT35510_Write_Data(0x09);
//     NT35510_Write_Data(0x0D);
//     NT35510_Write_Data(0x0E);
//     NT35510_Write_Data(0x0D);
//     NT35510_Write_Data(0x1F);
//     NT35510_Write_Data(0x1D);
//     NT35510_Write_Data(0x00);


//     NT35510_Write_Cmd(0x35); //Tearing Effect ON 
//     NT35510_Write_Data(0x00); 

//     NT35510_Write_Cmd(0x3A);
//     NT35510_Write_Data(0x55);

// 	NT35510_Write_Cmd(0x11);

// 	NT35510_Delay(160);
// 	NT35510_Write_Cmd(0x29);
// 	NT35510_Delay(20);
	

#endif		
}
/**
 * @brief  NT35510初始化函数，如果要用到lcd，一定要调用这个函数
 * @param  无
 * @retval 无
 */
void NT35510_Init (void)
{
	NT35510_BackLed_Control (LCD_BackLed_Enable);      //点亮LCD背光灯
	NT35510_Rst ();
	NT35510_REG_Config ();
	//设置默认扫描方向，其中 6 模式为大部分液晶例程的默认显示方向  
	NT35510_GramScan(lcd_scan_mode);
	NT35510_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* 清屏 */
}



/**
 * @brief  NT35510G背光LED控制
 * @param  enumState ：决定是否使能背光LED
  *   该参数为以下值之一：
  *     @arg ENABLE :使能背光LED
  *     @arg DISABLE :禁用背光LED
 * @retval 无
 */

void NT35510_BackLed_Control(LCD_BackLed_Cmd cmd)
{
    if(cmd == LCD_BackLed_Enable){
        HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);		
    }
    else{
        HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);		
    }
}



/**
 * @brief  NT35510G 软件复位
 * @param  无
 * @retval 无
 */
void NT35510_Rst(void)
{				 	 
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);		
	NT35510_Delay (0xAFF); 	 
	//HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);	
	NT35510_Delay (0xAFF); 						   
	 	 
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);	
	NT35510_Delay (0xAFF); 	 	
	
}


/**
 * @brief  设置NT35510的GRAM的扫描方向 
 * @param  ucOption ：选择GRAM的扫描方向 
 *     @arg 0-7 :参数可选值为0-7这八个方向
 *
 *	！！！其中0、3、5、6 模式适合从左至右显示文字，
 *				不推荐使用其它模式显示文字	其它模式显示文字会有镜像效果			
 *		
 *	其中0、2、4、6 模式的X方向像素为240，Y方向像素为320
 *	其中1、3、5、7 模式下X方向像素为320，Y方向像素为240
 *
 *	其中 6 模式为大部分液晶例程的默认显示方向
 *	其中 3 模式为摄像头例程使用的方向
 *	其中 0 模式为BMP图片显示例程使用的方向
 *
 * @retval 无
 * @note  坐标图例：A表示向上，V表示向下，<表示向左，>表示向右
					X表示X轴，Y表示Y轴

------------------------------------------------------------
模式0：				.		模式1：		.	模式2：			.	模式3：					
					A		.					A		.		A					.		A									
					|		.					|		.		|					.		|							
					Y		.					X		.		Y					.		X					
					0		.					1		.		2					.		3					
	<--- X0 o		.	<----Y1	o		.		o 2X--->  .		o 3Y--->	
------------------------------------------------------------	
模式4：				.	模式5：			.	模式6：			.	模式7：					
	<--- X4 o		.	<--- Y5 o		.		o 6X--->  .		o 7Y--->	
					4		.					5		.		6					.		7	
					Y		.					X		.		Y					.		X						
					|		.					|		.		|					.		|							
					V		.					V		.		V					.		V		
---------------------------------------------------------				
											 LCD屏示例
								|-----------------|
								|			野火Logo		|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|									|
								|-----------------|
								屏幕正面（宽240，高320）

 *******************************************************/
void NT35510_GramScan(LCD_Scan_Mode ucOption)
{	
	//参数检查，只可输入0-7
	if(ucOption >7)
		return;
	
	//根据模式更新lcd_scan_mode的值，主要用于触摸屏选择计算参数
	lcd_scan_mode = ucOption;
	
	//根据模式更新XY方向的像素宽度
	if(ucOption%2 == 0)	
	{
		//0 2 4 6模式下X方向像素宽度为240，Y方向为320
		LCD_X_LENGTH = NT35510_LESS_PIXEL;
		LCD_Y_LENGTH =	NT35510_MORE_PIXEL;
	}
	else				
	{
		//1 3 5 7模式下X方向像素宽度为320，Y方向为240
		LCD_X_LENGTH = NT35510_MORE_PIXEL;
		LCD_Y_LENGTH =	NT35510_LESS_PIXEL; 
	}

	//0x36命令参数的高3位可用于设置GRAM扫描方向	
	NT35510_Write_Cmd (0x3600); 
	NT35510_Write_Data (0x00 |(ucOption<<5));//根据ucOption的值设置LCD参数，共0-7种模式
	NT35510_Write_Cmd (CMD_SetCoordinateX); 
	NT35510_Write_Data (0x00);		/* x 起始坐标高8位 */
	NT35510_Write_Cmd (CMD_SetCoordinateX + 1); 
	NT35510_Write_Data (0x00);		/* x 起始坐标低8位 */
	NT35510_Write_Cmd (CMD_SetCoordinateX + 2); 
	NT35510_Write_Data (((LCD_X_LENGTH-1)>>8)&0xFF); /* x 结束坐标高8位 */	
	NT35510_Write_Cmd (CMD_SetCoordinateX + 3); 
	NT35510_Write_Data ((LCD_X_LENGTH-1)&0xFF);				/* x 结束坐标低8位 */

	NT35510_Write_Cmd (CMD_SetCoordinateY); 
	NT35510_Write_Data (0x00);		/* y 起始坐标高8位 */
	NT35510_Write_Cmd (CMD_SetCoordinateY + 1); 
	NT35510_Write_Data (0x00);		/* y 起始坐标低8位 */
	NT35510_Write_Cmd (CMD_SetCoordinateY + 2); 
	NT35510_Write_Data (((LCD_Y_LENGTH-1)>>8)&0xFF);	/* y 结束坐标高8位 */	 
	NT35510_Write_Cmd (CMD_SetCoordinateY + 3); 
	NT35510_Write_Data ((LCD_Y_LENGTH-1)&0xFF);				/* y 结束坐标低8位 */

	/* write gram start */
	NT35510_Write_Cmd (CMD_SetPixel);	
}



/**
 * @brief  在NT35510显示器上开辟一个窗口
 * @param  usX ：在特定扫描方向下窗口的起点X坐标
 * @param  usY ：在特定扫描方向下窗口的起点Y坐标
 * @param  usWidth ：窗口的宽度
 * @param  usHeight ：窗口的高度
 * @retval 无
 */
void NT35510_OpenWindow (uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{	
	NT35510_Write_Cmd (CMD_SetCoordinateX); 				 /* 设置X坐标 */
	NT35510_Write_Data (usX >> 8);	 /* 先高8位，然后低8位 */
	NT35510_Write_Cmd (CMD_SetCoordinateX + 1); 
	NT35510_Write_Data (usX & 0xff);	 /* 设置起始点和结束点*/
	NT35510_Write_Cmd (CMD_SetCoordinateX + 2);
	NT35510_Write_Data ((usX + usWidth - 1) >> 8);
	NT35510_Write_Cmd (CMD_SetCoordinateX + 3);
	NT35510_Write_Data ((usX + usWidth - 1) & 0xff);

	NT35510_Write_Cmd (CMD_SetCoordinateY); 			     /* 设置Y坐标*/
	NT35510_Write_Data (usY >> 8);
	NT35510_Write_Cmd (CMD_SetCoordinateY + 1);
	NT35510_Write_Data (usY & 0xff);
	NT35510_Write_Cmd (CMD_SetCoordinateY + 2);
	NT35510_Write_Data ((usY + usHeight - 1) >> 8);
	NT35510_Write_Cmd (CMD_SetCoordinateY + 3);
	NT35510_Write_Data ((usY + usHeight - 1) & 0xff);
	
}


/**
 * @brief  设定NT35510的光标坐标
 * @param  usX ：在特定扫描方向下光标的X坐标
 * @param  usY ：在特定扫描方向下光标的Y坐标
 * @retval 无
 */
static void NT35510_SetCursor (uint16_t usX, uint16_t usY)	
{
	NT35510_OpenWindow (usX, usY, 1, 1);
}


/**
 * @brief  在NT35510显示器上以某一颜色填充像素点
 * @param  ulAmout_Point ：要填充颜色的像素点的总数目
 * @param  usColor ：颜色
 * @retval 无
 */
static __inline void NT35510_FillColor (uint32_t ulAmout_Point, uint16_t usColor)
{
	uint32_t i = 0;
	
	
	/* memory write */
	NT35510_Write_Cmd (CMD_SetPixel);	
		
	for (i = 0; i < ulAmout_Point; i ++)
		NT35510_Write_Data (usColor);
		
	
}


/**
 * @brief  对NT35510显示器的某一窗口以某种颜色进行清屏
 * @param  usX ：在特定扫描方向下窗口的起点X坐标
 * @param  usY ：在特定扫描方向下窗口的起点Y坐标
 * @param  usWidth ：窗口的宽度
 * @param  usHeight ：窗口的高度
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void NT35510_Clear (uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{
	NT35510_OpenWindow (usX, usY, usWidth, usHeight);

	NT35510_FillColor (usWidth * usHeight, CurrentBackColor);		
	
}


/**
 * @brief  对NT35510显示器的某一点以某种颜色进行填充
 * @param  usX ：在特定扫描方向下该点的X坐标
 * @param  usY ：在特定扫描方向下该点的Y坐标
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void NT35510_SetPointPixel (uint16_t usX, uint16_t usY)	
{	
	if ((usX < LCD_X_LENGTH) && (usY < LCD_Y_LENGTH))
  {
		NT35510_SetCursor (usX, usY);
		
		NT35510_FillColor (1, CurrentTextColor);
	}
	
}


/**
 * @brief  读取NT35510 GRAN 的一个像素数据
 * @param  无
 * @retval 像素数据
 */
static uint16_t NT35510_Read_PixelData (void)	
{	
	uint16_t usR=0, usG=0, usB=0 ;

	
	NT35510_Write_Cmd (0x2E);   /* 读数据 */
	
	usR = NT35510_Read_Data (); 	/*FIRST READ OUT DUMMY DATA*/
	
	usR = NT35510_Read_Data ();  	/*READ OUT RED DATA  */
	usB = NT35510_Read_Data ();  	/*READ OUT BLUE DATA*/
	usG = NT35510_Read_Data ();  	/*READ OUT GREEN DATA*/	
	
  return (((usR >> 11) << 11) | ((usG >> 10) << 5) | (usB >> 11));
	
}


/**
 * @brief  获取 NT35510 显示器上某一个坐标点的像素数据
 * @param  usX ：在特定扫描方向下该点的X坐标
 * @param  usY ：在特定扫描方向下该点的Y坐标
 * @retval 像素数据
 */
uint16_t NT35510_GetPointPixel (uint16_t usX, uint16_t usY)
{ 
	uint16_t usPixelData;

	
	NT35510_SetCursor (usX, usY);
	
	usPixelData = NT35510_Read_PixelData ();
	
	return usPixelData;
	
}


/**
 * @brief  在 NT35510 显示器上使用 Bresenham 算法画线段 
 * @param  usX1 ：在特定扫描方向下线段的一个端点X坐标
 * @param  usY1 ：在特定扫描方向下线段的一个端点Y坐标
 * @param  usX2 ：在特定扫描方向下线段的另一个端点X坐标
 * @param  usY2 ：在特定扫描方向下线段的另一个端点Y坐标
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void NT35510_DrawLine (uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2)
{
	uint16_t us; 
	uint16_t usX_Current, usY_Current;
	
	int32_t lError_X = 0, lError_Y = 0, lDelta_X, lDelta_Y, lDistance; 
	int32_t lIncrease_X, lIncrease_Y; 	
	
	
	lDelta_X = usX2 - usX1; //计算坐标增量 
	lDelta_Y = usY2 - usY1; 
	
	usX_Current = usX1; 
	usY_Current = usY1; 
	
	
	if (lDelta_X > 0) 
		lIncrease_X = 1; //设置单步方向 
	
	else if (lDelta_X == 0) 
		lIncrease_X = 0;//垂直线 
	
	else 
  { 
    lIncrease_X = -1;
    lDelta_X = - lDelta_X;
  } 

	
	if (lDelta_Y > 0)
		lIncrease_Y = 1; 
	
	else if (lDelta_Y == 0)
		lIncrease_Y = 0;//水平线 
	
	else 
  {
    lIncrease_Y = -1;
    lDelta_Y = - lDelta_Y;
  } 

	
	if ( lDelta_X > lDelta_Y)
		lDistance = lDelta_X; //选取基本增量坐标轴 
	
	else 
		lDistance = lDelta_Y; 

	
	for (us = 0; us <= lDistance + 1; us ++)//画线输出 
	{  
		NT35510_SetPointPixel (usX_Current, usY_Current);//画点 
		
		lError_X += lDelta_X ; 
		lError_Y += lDelta_Y ; 
		
		if (lError_X > lDistance) 
		{ 
			lError_X -= lDistance; 
			usX_Current += lIncrease_X; 
		}  
		
		if (lError_Y > lDistance) 
		{ 
			lError_Y -= lDistance; 
			usY_Current += lIncrease_Y; 
		} 
		
	}  
	
	
}   


/**
 * @brief  在 NT35510 显示器上画一个矩形
 * @param  usX_Start ：在特定扫描方向下矩形的起始点X坐标
 * @param  usY_Start ：在特定扫描方向下矩形的起始点Y坐标
 * @param  usWidth：矩形的宽度（单位：像素）
 * @param  usHeight：矩形的高度（单位：像素）
 * @param  ucFilled ：选择是否填充该矩形
  *   该参数为以下值之一：
  *     @arg 0 :空心矩形
  *     @arg 1 :实心矩形 
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void NT35510_DrawRectangle (uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled)
{
	if (ucFilled)
	{
		NT35510_OpenWindow (usX_Start, usY_Start, usWidth, usHeight);
		NT35510_FillColor (usWidth * usHeight ,CurrentTextColor);	
	}
	else
	{
		NT35510_DrawLine (usX_Start, usY_Start, usX_Start + usWidth - 1, usY_Start);
		NT35510_DrawLine (usX_Start, usY_Start + usHeight - 1, usX_Start + usWidth - 1, usY_Start + usHeight - 1);
		NT35510_DrawLine (usX_Start, usY_Start, usX_Start, usY_Start + usHeight - 1);
		NT35510_DrawLine (usX_Start + usWidth - 1, usY_Start, usX_Start + usWidth - 1, usY_Start + usHeight - 1);		
	}

}


/**
 * @brief  在 NT35510 显示器上使用 Bresenham 算法画圆
 * @param  usX_Center ：在特定扫描方向下圆心的X坐标
 * @param  usY_Center ：在特定扫描方向下圆心的Y坐标
 * @param  usRadius：圆的半径（单位：像素）
 * @param  ucFilled ：选择是否填充该圆
  *   该参数为以下值之一：
  *     @arg 0 :空心圆
  *     @arg 1 :实心圆
 * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
 * @retval 无
 */
void NT35510_DrawCircle (uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled)
{
	int16_t sCurrentX, sCurrentY;
	int16_t sError;
	
	
	sCurrentX = 0; sCurrentY = usRadius;	  
	
	sError = 3 - (usRadius << 1);     //判断下个点位置的标志
	
	
	while (sCurrentX <= sCurrentY)
	{
		int16_t sCountY;
		
		
		if (ucFilled) 			
			for (sCountY = sCurrentX; sCountY <= sCurrentY; sCountY ++) 
			{                      
				NT35510_SetPointPixel (usX_Center + sCurrentX, usY_Center + sCountY);           //1，研究对象 
				NT35510_SetPointPixel (usX_Center - sCurrentX, usY_Center + sCountY);           //2       
				NT35510_SetPointPixel (usX_Center - sCountY,   usY_Center + sCurrentX);           //3
				NT35510_SetPointPixel (usX_Center - sCountY,   usY_Center - sCurrentX);           //4
				NT35510_SetPointPixel (usX_Center - sCurrentX, usY_Center - sCountY);           //5    
        NT35510_SetPointPixel (usX_Center + sCurrentX, usY_Center - sCountY);           //6
				NT35510_SetPointPixel (usX_Center + sCountY,   usY_Center - sCurrentX);           //7 	
        NT35510_SetPointPixel (usX_Center + sCountY,   usY_Center + sCurrentX);           //0				
			}
		
		else
		{          
			NT35510_SetPointPixel (usX_Center + sCurrentX, usY_Center + sCurrentY);             //1，研究对象
			NT35510_SetPointPixel (usX_Center - sCurrentX, usY_Center + sCurrentY);             //2      
			NT35510_SetPointPixel (usX_Center - sCurrentY, usY_Center + sCurrentX);             //3
			NT35510_SetPointPixel (usX_Center - sCurrentY, usY_Center - sCurrentX);             //4
			NT35510_SetPointPixel (usX_Center - sCurrentX, usY_Center - sCurrentY);             //5       
			NT35510_SetPointPixel (usX_Center + sCurrentX, usY_Center - sCurrentY);             //6
			NT35510_SetPointPixel (usX_Center + sCurrentY, usY_Center - sCurrentX);             //7 
			NT35510_SetPointPixel (usX_Center + sCurrentY, usY_Center + sCurrentX);             //0
    }			
		
		
		sCurrentX ++;

		
		if (sError < 0) 
			sError += 4 * sCurrentX + 6;	  
		
		else
		{
			sError += 10 + 4 * (sCurrentX - sCurrentY);   
			sCurrentY --;
		} 	
		
		
	}
	
	
}

// /**
//  * @brief  在 NT35510 显示器上显示一个英文字符
//  * @param  usX ：在特定扫描方向下字符的起始X坐标
//  * @param  usY ：在特定扫描方向下该点的起始Y坐标
//  * @param  cChar ：要显示的英文字符
//  * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
//  * @retval 无
//  */
// void NT35510_DispChar_EN (uint16_t usX, uint16_t usY, const char cChar)
// {
// 	uint8_t  byteCount, bitCount,fontLength;	
// 	uint16_t ucRelativePositon;
// 	uint8_t *Pfont;
	
// 	//对ascii码表偏移（字模表不包含ASCII表的前32个非图形符号）
// 	ucRelativePositon = cChar - ' ';
	
// 	//每个字模的字节数
// 	fontLength = (LCD_Currentfonts->Width*LCD_Currentfonts->Height)/8;
		
// 	//字模首地址
// 	/*ascii码表偏移值乘以每个字模的字节数，求出字模的偏移位置*/
// 	Pfont = (uint8_t *)&LCD_Currentfonts->table[ucRelativePositon * fontLength];
	
// 	//设置显示窗口
// 	NT35510_OpenWindow (usX, usY, LCD_Currentfonts->Width, LCD_Currentfonts->Height);
	
// 	NT35510_Write_Cmd (CMD_SetPixel);			

// 	//按字节读取字模数据
// 	//由于前面直接设置了显示窗口，显示数据会自动换行
// 	for (byteCount = 0; byteCount < fontLength; byteCount++)
// 	{
// 			//一位一位处理要显示的颜色
// 			for (bitCount = 0; bitCount < 8; bitCount++)
// 			{
// 					if (Pfont[byteCount] & (0x80>>bitCount))
// 						NT35510_Write_Data (CurrentTextColor);			
// 					else
// 						NT35510_Write_Data (CurrentBackColor);
// 			}	
// 	}	
// }


// /**
//  * @brief  在 NT35510 显示器上显示英文字符串
//  * @param  line ：在特定扫描方向下字符串的起始Y坐标
//   *   本参数可使用宏LINE(0)、LINE(1)等方式指定文字坐标，
//   *   宏LINE(x)会根据当前选择的字体来计算Y坐标值。
// 	*		显示中文且使用LINE宏时，需要把英文字体设置成Font8x16
//  * @param  pStr ：要显示的英文字符串的首地址
//  * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
//  * @retval 无
//  */
// void NT35510_DispStringLine_EN ( uint16_t line,  char * pStr)
// {
// 	uint16_t usX = 0;
	
// 	while (* pStr != '\0')
// 	{
// 		if ((usX - NT35510_DispWindow_X_Star + LCD_Currentfonts->Width) > LCD_X_LENGTH)
// 		{
// 			usX = NT35510_DispWindow_X_Star;
// 			line += LCD_Currentfonts->Height;
// 		}
		
// 		if ((line - NT35510_DispWindow_Y_Star + LCD_Currentfonts->Height) > LCD_Y_LENGTH)
// 		{
// 			usX = NT35510_DispWindow_X_Star;
// 			line = NT35510_DispWindow_Y_Star;
// 		}
		
// 		NT35510_DispChar_EN (usX, line, * pStr);
		
// 		pStr ++;
		
// 		usX += LCD_Currentfonts->Width;
		
// 	}
	
// }


// /**
//  * @brief  在 NT35510 显示器上显示英文字符串
//  * @param  usX ：在特定扫描方向下字符的起始X坐标
//  * @param  usY ：在特定扫描方向下字符的起始Y坐标
//  * @param  pStr ：要显示的英文字符串的首地址
//  * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
//  * @retval 无
//  */
// void NT35510_DispString_EN (	uint16_t usX ,uint16_t usY,  char * pStr)
// {
// 	while (* pStr != '\0')
// 	{
// 		if ((usX - NT35510_DispWindow_X_Star + LCD_Currentfonts->Width) > LCD_X_LENGTH)
// 		{
// 			usX = NT35510_DispWindow_X_Star;
// 			usY += LCD_Currentfonts->Height;
// 		}
		
// 		if ((usY - NT35510_DispWindow_Y_Star + LCD_Currentfonts->Height) > LCD_Y_LENGTH)
// 		{
// 			usX = NT35510_DispWindow_X_Star;
// 			usY = NT35510_DispWindow_Y_Star;
// 		}
		
// 		NT35510_DispChar_EN (usX, usY, * pStr);
		
// 		pStr ++;
		
// 		usX += LCD_Currentfonts->Width;
		
// 	}
	
// }


// /**
//  * @brief  在 NT35510 显示器上显示英文字符串(沿Y轴方向)
//  * @param  usX ：在特定扫描方向下字符的起始X坐标
//  * @param  usY ：在特定扫描方向下字符的起始Y坐标
//  * @param  pStr ：要显示的英文字符串的首地址
//  * @note 可使用LCD_SetBackColor、LCD_SetTextColor、LCD_SetColors函数设置颜色
//  * @retval 无
//  */
// void NT35510_DispString_EN_YDir (	 uint16_t usX,uint16_t usY ,  char * pStr)
// {	
// 	while (* pStr != '\0')
// 	{
// 		if ((usY - NT35510_DispWindow_Y_Star + LCD_Currentfonts->Height) >LCD_Y_LENGTH )
// 		{
// 			usY = NT35510_DispWindow_Y_Star;
// 			usX += LCD_Currentfonts->Width;
// 		}
		
// 		if ((usX - NT35510_DispWindow_X_Star + LCD_Currentfonts->Width) >  LCD_X_LENGTH)
// 		{
// 			usX = NT35510_DispWindow_X_Star;
// 			usY = NT35510_DispWindow_Y_Star;
// 		}
		
// 		NT35510_DispChar_EN (usX, usY, * pStr);
		
// 		pStr ++;
		
// 		usY += LCD_Currentfonts->Height;		
// 	}	
// }


// /**
//   * @brief  设置英文字体类型
//   * @param  fonts: 指定要选择的字体
// 	*		参数为以下值之一
//   * 	@arg：Font24x32;
//   * 	@arg：Font16x24;
//   * 	@arg：Font8x16;
//   * @retval None
//   */
// void LCD_SetFont(sFONT *fonts)
// {
//   LCD_Currentfonts = fonts;
// }

// /**
//   * @brief  获取当前字体类型
//   * @param  None.
//   * @retval 返回当前字体类型
//   */
// sFONT *LCD_GetFont(void)
// {
//   return LCD_Currentfonts;
// }


// /**
//   * @brief  设置LCD的前景(字体)及背景颜色,RGB565
//   * @param  TextColor: 指定前景(字体)颜色
//   * @param  BackColor: 指定背景颜色
//   * @retval None
//   */
// void LCD_SetColors(uint16_t TextColor, uint16_t BackColor) 
// {
//   CurrentTextColor = TextColor; 
//   CurrentBackColor = BackColor;
// }

// /**
//   * @brief  获取LCD的前景(字体)及背景颜色,RGB565
//   * @param  TextColor: 用来存储前景(字体)颜色的指针变量
//   * @param  BackColor: 用来存储背景颜色的指针变量
//   * @retval None
//   */
// void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
// {
//   *TextColor = CurrentTextColor;
//   *BackColor = CurrentBackColor;
// }

// /**
//   * @brief  设置LCD的前景(字体)颜色,RGB565
//   * @param  Color: 指定前景(字体)颜色 
//   * @retval None
//   */
// void LCD_SetTextColor(uint16_t Color)
// {
//   CurrentTextColor = Color;
// }

// /**
//   * @brief  设置LCD的背景颜色,RGB565
//   * @param  Color: 指定背景颜色 
//   * @retval None
//   */
// void LCD_SetBackColor(uint16_t Color)
// {
//   CurrentBackColor = Color;
// }

// /**
//   * @brief  清除某行文字
//   * @param  Line: 指定要删除的行
//   *   本参数可使用宏LINE(0)、LINE(1)等方式指定要删除的行，
//   *   宏LINE(x)会根据当前选择的字体来计算Y坐标值，并删除当前字体高度的第x行。
//   * @retval None
//   */
// void NT35510_ClearLine(uint16_t Line)
// {
//   NT35510_Clear(0,Line,LCD_X_LENGTH,((sFONT *)LCD_GetFont())->Height);	/* 清屏，显示全黑 */

// }
// /*********************end of file*************************/




