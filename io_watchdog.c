#include<stdio.h>
#include<sys/io.h>
#include<stdlib.h>
#include<unistd.h>


/* IO Ports */
#define REG		0x2e
#define VAL		0x2f
/* Logical device Numbers LDN */
#define LDNREG		0x07
#define GPIOLND		0x07


static int superio_enter(void)
{
	outb(0x87, REG);
	outb(0x01, REG);
	outb(0x55, REG);
	outb(0x55, REG);
	return 0;
}

static int superio_inb(int reg)
{
	int val;
	outb(reg, REG);
	val = inb(VAL);
	return val;
}

static int superio_inw(int reg, int type)
{
	int val;
	if(type)
	{
		outb(reg++, REG);
	}else{
		outb(reg--, REG);
	}
	val = inb(VAL) << 8;
	outb(reg, REG);
	val |= inb(VAL);
	return val;
}

static void exit_superio()
{
	outb(0x02, REG);
	outb(0x02, VAL);
}

static void feed_dog(int val)
{
	outb(0x73, REG);
	outb(val, VAL);
	if(val>255)
	{
		outb(0x74, REG);
		outb(val>>8, VAL);
	}
	
}

int main()
{
	unsigned int chip_type;
	int ret = iopl(3);
	if(ret == -1)
	{
		printf("iopl error.\n");
		return -1;
	}

	superio_enter();
	chip_type = superio_inw(0x20, 1);
	if(chip_type)
	{
		printf("Chip found, Chip %04x.\n", chip_type);
	}
	exit_superio();

	superio_enter();
	//select logic device
	outb(LDNREG, REG);
	outb(GPIOLND, VAL);

	int ctrl = superio_inb(0x71);
	printf("Watch dog Control Register:%02x\n", ctrl);

	printf("Watch dog defalut Configuration Register:%02x\n", superio_inb(0x72));
	//set configuartion
	outb(0x72, REG);
	outb(0x90, VAL);//1001 0000
	int cfg = superio_inb(0x72);
	printf("Watch dog Configuration Register:%02x\n", cfg);

	int timeout = superio_inw(0x74, 0);
	printf("Watch dog TIme-out default Value Register:%04x\n", timeout);
	outb(0x73, REG);
	outb(0x1e, VAL);//12c:300s 1e:30s  3c:60s
	outb(0x74, REG);
	outb(0x00, VAL);
	exit_superio();

	usleep(20*1000000);

	superio_enter();
	int timeout2 = superio_inw(0x74, 0);
	printf("Watch dog TIme-out2 Value Register:%04x\n", timeout2);

	feed_dog(50);//喂狗，并将重新设置为50s
	int timeout3 = superio_inw(0x74, 0);
	printf("Watch dog TIme-out3 Value Register:%04x\n", timeout3);

	//关闭喂狗
	//outb(0x72, REG);
	//outb(0x80, VAL);

	exit_superio();
	
	iopl(0);
	return 0;
}
