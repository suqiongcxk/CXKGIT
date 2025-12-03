#include "image.h"
#include "LCD_Test.h"
#include "LCD_1in69.h"
#include "DEV_Config.h"
#include "stdio.h"

void LCD_1in69_test()
{
    printf("LCD_1IN69 Demo\r\n");
    //一些硬件初始化
    DEV_Module_Init();

    printf("LCD_1IN69_ Init and Clear...\r\n");

    //PWM输出用于配置LCD背光亮度
    LCD_1IN69_SetBackLight(1000);

    //硬件复位，先高再低再高
    LCD_1IN69_Init(VERTICAL);

    //硬件复位，先高再低再高
    LCD_1IN69_Clear(WHITE);
    
// #if 1
    printf("Paint_NewImage\r\n");

    //创建一个图像缓存区用于绘图，包含了长宽，旋转角度，颜色等信息
    Paint_NewImage(LCD_1IN69_WIDTH, LCD_1IN69_HEIGHT, 0, WHITE);

    printf("Set Clear and Display Funtion\r\n");
    /*
    // void (*DISPLAY)(UWORD,UWORD,UWORD);
    // void (*CLEAR)(UWORD);



    我要好好讲讲下面的这几行代码了，下面两个是函数指针的赋值
    把LCD_1IN69_Clear函数的地址赋值给CLEAR指针变量
    把LCD_1IN69_DrawPoint函数的地址赋值给DISPLAY指针
    现在先解释一些上面的参数 void (*Clear)(UWORD)  这是一个函数指针变量
    Clear是变量名，*Clear表示这是一个指针变量，void表示这个函数没有返回值
    (UWORD)表示这个函数有一个参数，参数类型是UWORD,这个UWORD是uint16_t的宏定义
    所以我就可以传任意地满足以上要求的函数名，这样就完成了回调函数的注册
    
    void Paint_SetClearFuntion(void (*Clear)(UWORD))
    {
     CLEAR=Clear;
    }
     这样就代表我的函数绑定功能把LCD_1IN69_Clear函数绑定给了CLEAR指针变量
    以后我只要调用CLEAR(Color)就相当于调用了LCD_1IN69_Clear(Color)
    以后我只要调用DISPLAY(X,Y,Color)就相当于调用了LCD_1IN69_DrawPoint(X,Y,Color)
 */
    Paint_SetClearFuntion(LCD_1IN69_Clear);
    Paint_SetDisplayFuntion(LCD_1IN69_DrawPoint);

    printf("Paint_Clear\r\n");

    //这里的clean用的是把屏幕用白色填满
    Paint_Clear(WHITE);
    DEV_Delay_ms(100);

    printf("drawing...\r\n");

    //旋转180度显示
    Paint_SetRotate(180);

    //英文显示，参数有X坐标，Y坐标，字符串，字体，背景色（就是占据的那一块地方的颜色，效果可以联想按钮），前景色
    Paint_DrawString_EN(30, 10, "123", &Font24, YELLOW, RED);  
    Paint_DrawString_EN(30, 34, "ABC", &Font24, BLUE, CYAN);
    
    //画浮点数，参数有X坐标，Y坐标，浮点数，小数点位数，字体，背景色，前景色
		
		
    Paint_DrawFloatNum (30, 58, 987.654321,3, &Font12, WHITE, BLACK);
    Paint_DrawString_CN(30, 130,  "陈旭康",  &Font24CN, WHITE, BLUE);
    
		
    //画图片，参数有图片数组指针，X起始坐标，Y起始坐标，图片宽度，图片高度
    Paint_DrawImage(gImage_1, 30, 70, 60, 60);
		
		
    Paint_DrawRectangle(185, 10, 285, 58, RED, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    //斜线,两点一线，加上线的粗细跟虚实
    Paint_DrawLine(185, 10, 285, 58, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    Paint_DrawLine(285, 10, 185, 58, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    
    //盲猜一波奥运五环
    Paint_DrawCircle(120, 60, 25, BLUE, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    Paint_DrawCircle(150, 60, 25, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    Paint_DrawCircle(190, 60, 25, RED, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    Paint_DrawCircle(145, 85, 25, YELLOW, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    Paint_DrawCircle(165, 85, 25, GREEN, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);


    DEV_Delay_ms(5000);
// #endif


// #if 1
//     printf("Paint_NewImage\r\n");

//     //创建一个顺时针旋转角度90度显示的图像缓存区用于绘图，包含了长宽，旋转角度，颜色等信息
//     Paint_NewImage(LCD_1IN69_WIDTH, LCD_1IN69_HEIGHT, 90, WHITE);

//     printf("Set Clear and Display Funtion\r\n");
//     Paint_SetClearFuntion(LCD_1IN69_Clear);
//     Paint_SetDisplayFuntion(LCD_1IN69_DrawPoint);

//     printf("Paint_Clear\r\n");
//     Paint_Clear(WHITE);
//     DEV_Delay_ms(100);

//     printf("drawing...\r\n");
//     Paint_DrawString_EN(30, 10, "123", &Font24, YELLOW, RED);  
//     Paint_DrawString_EN(30, 34, "ABC", &Font24, BLUE, CYAN);
    
//     Paint_DrawFloatNum (30, 58, 987.654321,3, &Font12, WHITE, BLACK);
//     Paint_DrawString_CN(30, 130, "陈旭康", &Font24CN, WHITE, BLUE);
    
//     Paint_DrawImage(gImage_1, 30, 70, 60, 60);

//     Paint_DrawRectangle(100, 20, 160, 120, RED, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawLine(100, 20, 160, 120, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
//     Paint_DrawLine(100, 120, 160, 20, MAGENTA, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    
//     Paint_DrawCircle(190, 60, 25, BLUE, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(220, 60, 25, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(250, 60, 25, RED, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(205, 85, 25, YELLOW, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
//     Paint_DrawCircle(235, 85, 25, GREEN, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    
//     DEV_Delay_ms(3000);
// #endif

    // printf("quit...\r\n");
    // DEV_Module_Exit();
}

