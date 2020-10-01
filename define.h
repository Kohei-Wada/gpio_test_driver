#define GPIO_BASE    0xfe200000
#define GPIO_END     0xfe2000b3

//regiser address offset
//////////////////////////////////////////////////////////
#define GPFSEL0      0x00
#define GPFSEL1      0x04
#define GPFSEL2      0x08
#define GPFSEL3      0x0c
#define GPFSEL4      0x10
#define GPFSEL5      0x14

#define GPFSET0      0x1c
#define GPFSET1      0x20

#define GPCLR0       0x28
#define GPCLR1       0x2c

#define GPFLEV0      0x34
//////////////////////////////////////////////////////////


//gpsel0 register description
#define GPSEL_INPUT   0b000
#define GPSEL_OUTPUT  0b001
#define GPSEL_FUNC0   0b100
#define GPSEL_FUNC1   0b101
#define GPSEL_FUNC2   0b110
#define GPSEL_FUNC3   0b111
#define GPSEL_FUNC4   0b011
#define GPSEL_FUNC5   0b010

//iomemsize
#define MEM_SIZE 180


struct gpio_test_dev {
    struct cdev   cdev;
    struct mutex  mutex;
    unsigned long base;
    unsigned long io_base;
};
