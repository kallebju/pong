/*
 * 	startup.c
 *
 */
 
#define STK 0xE000E010 
#define STK_CTRL	((volatile unsigned int *) (STK))
#define STK_LOAD	((volatile unsigned int *) (STK+0x4))
#define STK_VAL		((volatile unsigned int *) (STK+0x8))

#define PORT_BASE	0x40021000 

#define portModer	((volatile unsigned int *)		(PORT_BASE))
#define portOtyper	((volatile unsigned short *)	(PORT_BASE+0x4))
#define portOspeedr	((volatile unsigned int *)		(PORT_BASE+0x8))
#define portPupdr	((volatile unsigned int *)		(PORT_BASE+0xC))

#define portIdrLow	((volatile unsigned char *)		(PORT_BASE+0x10))
#define portIdrHigh	((volatile unsigned char *)		(PORT_BASE+0x11))
#define portOdrLow	((volatile unsigned char *)		(PORT_BASE+0x14))
#define portOdrHigh	((volatile unsigned char *)		(PORT_BASE+0x15))

#define B_E			0x40
#define B_SELECT	0x4
#define B_RW		0x2
#define B_RS		0x1

#define B_RST		0x20
#define B_CS2		0x10
#define B_CS1		0x8
#define LCD_ON			0x3F
#define LCD_OFF			0x3E
#define LCD_SET_ADD		0x40
#define LCD_SET_PAGE	0xB8
#define LCD_DISP_START	0xC0
#define LCD_BUSY		0x80

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

void startup ( void )
{
__asm volatile(
	" LDR R0,=0x2001C000\n"		/* set stack */
	" MOV SP,R0\n"
	" BL main\n"				/* call main */
	"_exit: B .\n"				/* never return */
	) ;
}
void delay_250ns(void){

	

	*STK_CTRL = 0;

	*STK_LOAD = ((158/4)-1);

	*STK_VAL = 0;

	*STK_CTRL = 5;

	while( (*STK_CTRL & 0x10000) == 0){

	}

	

	*STK_CTRL = 0;

	

}




void delay_500ns(void){

	delay_250ns();

	delay_250ns();

}




void delay_mikro(unsigned int us){	

	for(int a = 0; a <= us; a++){

		delay_250ns();

		delay_250ns();

		delay_250ns();

		delay_250ns();

	}

}




void delay_milli(unsigned int ms){



		for(int a = 0; a <= ms*1000; a++){

		delay_250ns();

		delay_250ns();

		delay_250ns();

		delay_250ns();

	}

}




static void graphic_ctrl_bit_set(uint8_t x){

	uint8_t c;

	c = *portOdrLow;

	c &= ~B_SELECT;

	c |= (~B_SELECT & x);

	*portOdrLow = c;

}




static void graphic_ctrl_bit_clear(uint8_t x){

	uint8_t c;

	c = *portOdrLow;

	c &= ~B_SELECT;

	c &= ~x;

	*portOdrLow = c;

}




void select_controller(uint8_t controller){

	switch(controller){

		case 0: 

			graphic_ctrl_bit_clear(B_CS1|B_CS2);

			break;

		case B_CS1:

			graphic_ctrl_bit_set(B_CS1);

			graphic_ctrl_bit_clear(B_CS2);

			break;

		case B_CS2:

			graphic_ctrl_bit_set(B_CS2);

			graphic_ctrl_bit_clear(B_CS1);

			break;

		case B_CS1 | B_CS2:

			graphic_ctrl_bit_set(B_CS1|B_CS2);

			break;

	}

}




static void graphic_wait_ready(){

	uint8_t c;

	graphic_ctrl_bit_clear(B_E);

	*portModer = 0x00005555;

	graphic_ctrl_bit_clear(B_RS);

	graphic_ctrl_bit_set(B_RW);

	delay_500ns();

	

	while(1){

		graphic_ctrl_bit_set(B_E);

		delay_500ns();

		c = (*portIdrHigh & LCD_BUSY);

		graphic_ctrl_bit_clear(B_E);

		delay_500ns();

		if(c == 0) break;

	}

	graphic_ctrl_bit_set(B_E);  //E = 1 enligt bok dock ej FL

	*portModer = 0x55555555;

}




static uint8_t graphic_read(uint8_t controller){

	uint8_t c;

	graphic_ctrl_bit_clear(B_E);

	*portModer = 0x00005555;

	graphic_ctrl_bit_set(B_RS|B_RW);

	select_controller(controller);

	delay_500ns();

	graphic_ctrl_bit_set(B_E);

	delay_500ns();

	c = *portIdrHigh;

	graphic_ctrl_bit_clear(B_E);

	*portModer = 0x55555555;

	if(controller & B_CS1){

		select_controller(B_CS1);

		graphic_wait_ready();

	}

	if(controller & B_CS2){

		select_controller(B_CS2);

		graphic_wait_ready();

	}

	return c;

}




static void graphic_write(uint8_t value, uint8_t controller){

	*portOdrHigh = value;

	select_controller(controller);

	delay_500ns();

	graphic_ctrl_bit_set(B_E);

	delay_500ns();

	graphic_ctrl_bit_clear(B_E);

	

	if(controller & B_CS1){

		select_controller(B_CS1);

		graphic_wait_ready();

	}

	if(controller & B_CS2){

		select_controller(B_CS1);

		graphic_wait_ready();

	}

	*portOdrHigh = 0;

	graphic_ctrl_bit_set(B_E);

	select_controller(0);

}




void graphic_write_command(uint8_t cmd, uint8_t controller){

	graphic_ctrl_bit_clear(B_E);

	select_controller(controller);

	graphic_ctrl_bit_clear(B_RS|B_RW);

	graphic_write(cmd, controller);




}




void graphic_write_data(uint8_t data, uint8_t controller){

	graphic_ctrl_bit_clear(B_E);

	select_controller(controller);

	graphic_ctrl_bit_set(B_RS);

	graphic_ctrl_bit_clear(B_RW);

	graphic_write(data, controller);

}




static uint8_t graphic_read_data(uint8_t controller){

	graphic_read(controller);

	return graphic_read(controller);

}




void app_init(){

	*portModer &= ~0xFFFFFFFF;

	*portModer |= 0x55555555;

}




void graphic_initialize(){

	graphic_ctrl_bit_set(B_E);

	delay_mikro(10);

	graphic_ctrl_bit_clear(B_CS1|B_CS2|B_RST|B_E);

	delay_milli(30);

	graphic_ctrl_bit_set(B_RST);

	delay_milli(100);

	graphic_write_command(LCD_OFF,			B_CS1|B_CS2);

	graphic_write_command(LCD_ON,			B_CS1|B_CS2);

	graphic_write_command(LCD_DISP_START,	B_CS1|B_CS2);

	graphic_write_command(LCD_SET_ADD,		B_CS1|B_CS2);

	graphic_write_command(LCD_SET_PAGE,		B_CS1|B_CS2);

	select_controller(0);

}




void graphic_clear_screen(){

	for(int page = 0; page<=7; page++){

		graphic_write_command(LCD_SET_PAGE | page , B_CS1|B_CS2);

		graphic_write_command(LCD_SET_ADD | 0 , B_CS1|B_CS2);

		for(int add = 0; add<=63; add++){

			graphic_write_data(0, B_CS1|B_CS2);

		}

	}

} 




void pixel(int x, int y, int set){
	if((x < 1) || (y < 1) || (x > 128) || (y > 64)){	//Behöver inte kolla x, då x:s maximala värde är det maximala värdet som rymms i en char.???
		return;
	}
	unsigned char mask;
	unsigned char controller;
	unsigned char x_fysisk;
	unsigned char temp;
	int index = (y-1)/8;
	switch((y-1)%8){
		case 0:	mask = 0x1;
				break;
		case 1: mask = 0x2;
				break;
		case 2: mask = 0x4;
				break;
		case 3: mask = 0x8;
				break;
		case 4: mask= 0x10;
				break;
		case 5: mask= 0x20;
				break;
		case 6: mask = 0x40;
				break;
		case 7: mask = 0x80;
				break;
	}
	if(set==0){
		mask = ~mask;
	}
	if(x>64){
		controller = B_CS2;
		x_fysisk = x-65;
	}
	else{
		controller = B_CS1;
		x_fysisk = x-1;
	}
	graphic_write_command(LCD_SET_ADD | x_fysisk, controller);
	graphic_write_command(LCD_SET_PAGE | index, controller);
	temp = graphic_read_data(controller);
	graphic_write_command(LCD_SET_ADD | x_fysisk, controller);
	if(set == 1){
		mask = mask|temp;
	}
	else{
		mask = mask&temp;
	}
	graphic_write_data(mask,controller);
}
void main(void)
{
	
}

