#define GPIO_BASE    0xfe200000
#define GPIO_END     0xfe2000b3

//regiser address offset
//////////////////////////////////////////////////////////
#define GPFSEL0      0x00
#define GPFSET0      0x1c
#define GPCLR0       0x28
#define GPFLEV0      0x34
//////////////////////////////////////////////////////////


//gpsel0 register mode
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
    unsigned int registers[6];
};
