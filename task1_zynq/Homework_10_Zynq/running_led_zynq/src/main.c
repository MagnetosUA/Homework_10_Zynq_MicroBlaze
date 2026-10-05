#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"

#define LED_GPIO_DEVICE_ID       XPAR_AXI_GPIO_0_DEVICE_ID
#define INPUT_GPIO_DEVICE_ID     XPAR_AXI_GPIO_1_DEVICE_ID
#define TIMER_DEVICE_ID          XPAR_AXI_TIMER_0_DEVICE_ID
#define INTC_DEVICE_ID           XPAR_SCUGIC_SINGLE_DEVICE_ID
#define TIMER_INTERRUPT_ID       XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR

#define LED_CHANNEL              1
#define BUTTON_CHANNEL           1
#define SWITCH_CHANNEL           2

#define BTN_FASTER               0x01
#define BTN_SLOWER               0x02
#define BTN_STOP_RESUME          0x04

#define TIMER_NUMBER             0

XGpio LedGpio;
XGpio InputGpio;
XTmrCtr Timer;
XScuGic Intc;

volatile u32 led_pattern = 0x01;
volatile int timer_event = 0;

volatile int running = 1;

/* Timer period in milliseconds */
volatile u32 timer_period_ms = 500;

static void TimerCallback(void *CallBackRef, u8 TmrCtrNumber)
{
    timer_event = 1;
}

static void SetTimerPeriod(u32 period_ms)
{
    u32 timer_clock;
    u32 ticks;

    timer_clock = XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ;

    ticks = (timer_clock / 1000) * period_ms;

    XTmrCtr_Stop(&Timer, TIMER_NUMBER);

    XTmrCtr_SetResetValue(
        &Timer,
        TIMER_NUMBER,
        ticks - 1
    );

    XTmrCtr_Start(&Timer, TIMER_NUMBER);
}

static int SetupInterruptSystem(void)
{
    XScuGic_Config *IntcConfig;
    int status;

    IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);

    if (IntcConfig == NULL) {
        return XST_FAILURE;
    }

    status = XScuGic_CfgInitialize(
        &Intc,
        IntcConfig,
        IntcConfig->CpuBaseAddress
    );

    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    Xil_ExceptionInit();

    Xil_ExceptionRegisterHandler(
        XIL_EXCEPTION_ID_INT,
        (Xil_ExceptionHandler)XScuGic_InterruptHandler,
        &Intc
    );

    status = XScuGic_Connect(
        &Intc,
        TIMER_INTERRUPT_ID,
        (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
        &Timer
    );

    if (status != XST_SUCCESS) {
        return XST_FAILURE;
    }

    XScuGic_Enable(
        &Intc,
        TIMER_INTERRUPT_ID
    );

    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

int main(void)
{
    int status;
    u32 buttons;
    u32 switch_value;

    u32 previous_buttons = 0;

    xil_printf("\r\n");
    xil_printf("Zynq Running LED started\r\n");

    /*
     * LED GPIO
     */
    status = XGpio_Initialize(
        &LedGpio,
        LED_GPIO_DEVICE_ID
    );

    if (status != XST_SUCCESS) {
        xil_printf("LED GPIO init failed\r\n");
        return XST_FAILURE;
    }

    XGpio_SetDataDirection(
        &LedGpio,
        LED_CHANNEL,
        0x00
    );

    /*
     * Buttons + switch GPIO
     */
    status = XGpio_Initialize(
        &InputGpio,
        INPUT_GPIO_DEVICE_ID
    );

    if (status != XST_SUCCESS) {
        xil_printf("Input GPIO init failed\r\n");
        return XST_FAILURE;
    }

    XGpio_SetDataDirection(
        &InputGpio,
        BUTTON_CHANNEL,
        0x07
    );

    XGpio_SetDataDirection(
        &InputGpio,
        SWITCH_CHANNEL,
        0x01
    );

    /*
     * AXI Timer
     */
    status = XTmrCtr_Initialize(
        &Timer,
        TIMER_DEVICE_ID
    );

    if (status != XST_SUCCESS) {
        xil_printf("Timer init failed\r\n");
        return XST_FAILURE;
    }

    XTmrCtr_SetHandler(
        &Timer,
        TimerCallback,
        &Timer
    );

    XTmrCtr_SetOptions(
        &Timer,
        TIMER_NUMBER,
        XTC_INT_MODE_OPTION |
        XTC_AUTO_RELOAD_OPTION |
        XTC_DOWN_COUNT_OPTION
    );

    /*
     * Interrupt controller
     */
    status = SetupInterruptSystem();

    if (status != XST_SUCCESS) {
        xil_printf("Interrupt setup failed\r\n");
        return XST_FAILURE;
    }

    SetTimerPeriod(timer_period_ms);

    XGpio_DiscreteWrite(
        &LedGpio,
        LED_CHANNEL,
        led_pattern
    );

    while (1)
    {
        buttons = XGpio_DiscreteRead(
            &InputGpio,
            BUTTON_CHANNEL
        );

        switch_value = XGpio_DiscreteRead(
            &InputGpio,
            SWITCH_CHANNEL
        );

        /*
         * Detect button press edge
         */
        u32 pressed =
            buttons & ~previous_buttons;

        previous_buttons = buttons;

        /*
         * Faster
         */
        if (pressed & BTN_FASTER)
        {
            if (timer_period_ms > 100)
            {
                timer_period_ms -= 100;

                SetTimerPeriod(timer_period_ms);

                xil_printf(
                    "Speed faster: %lu ms\r\n",
                    timer_period_ms
                );
            }
        }

        /*
         * Slower
         */
        if (pressed & BTN_SLOWER)
        {
            if (timer_period_ms < 2000)
            {
                timer_period_ms += 100;

                SetTimerPeriod(timer_period_ms);

                xil_printf(
                    "Speed slower: %lu ms\r\n",
                    timer_period_ms
                );
            }
        }

        /*
         * Stop / Resume
         */
        if (pressed & BTN_STOP_RESUME)
        {
            running = !running;

            if (running)
            {
                xil_printf("RUN\r\n");
            }
            else
            {
                xil_printf("STOP\r\n");
            }
        }

        /*
         * Timer event
         */
        if (timer_event)
        {
            timer_event = 0;

            if (running)
            {
                /*
                 * SW0 = 0 -> forward
                 * SW0 = 1 -> backward
                 */

                if ((switch_value & 0x01) == 0)
                {
                    led_pattern <<= 1;

                    if (led_pattern > 0x08)
                    {
                        led_pattern = 0x01;
                    }
                }
                else
                {
                    led_pattern >>= 1;

                    if (led_pattern == 0)
                    {
                        led_pattern = 0x08;
                    }
                }

                XGpio_DiscreteWrite(
                    &LedGpio,
                    LED_CHANNEL,
                    led_pattern
                );
            }
        }
    }

    return 0;
}
