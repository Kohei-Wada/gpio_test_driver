#define GPIO_BASE    0xfe200000
#define GPIO_END     0xf00000b3

#define GPFSEL0      0x00
#define GPFSEL1      0x04
#define GPFSEL2      0x08
#define GPFSET0      0x1c
#define GPCLR0       0x28
#define GPFLEV0      0x34

#define GPSEL_INPUT   0b000
#define GPSEL_OUTPUT  0b001

#define MEM_SIZE 180


struct gpio_test_dev {
    struct cdev   cdev;
    struct mutex  mutex;
    unsigned long base;
    unsigned long io_base;
};
