/* #include <portmacro.h> */
/* const byte interruptPin = 25; */
/* volatile int interruptCounter = 0; */
/* int numberOfInterrupts = 0; */

/* portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; */

/* void setup() { */

/*         Serial.begin(115200); */
/*         Serial.println("Monitoring interrupts: "); */
/*         pinMode(interruptPin, INPUT_PULLUP); */
/*         attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING); */

/* } */

/* void handleInterrupt() { */
/*         portENTER_CRITICAL_ISR(&mux); */
/*         interruptCounter++; */
/*         portEXIT_CRITICAL_ISR(&mux); */
/* } */

/* void loop() { */

/*         if(interruptCounter>0){ */

/*                 portENTER_CRITICAL(&mux); */
/*                 interruptCounter--; */
/*                 portEXIT_CRITICAL(&mux); */

/*                 numberOfInterrupts++; */
/*                 Serial.print("An interrupt has occurred. Total: "); */
/*                 Serial.println(numberOfInterrupts); */
/*         } */
/* } */
int pin = 12;          //首先我们需要找一个灯来观察
volatile int state = LOW;    //设置灯状态

void blink()            //触发函数
{
        state = !state;            //将状态变量求反
}

void setup()
{
        pinMode(pin, OUTPUT);
        attachInterrupt(D1, blink, RISING);    //设置interrupt，NodeMCU和Arduino的的管脚号都可用。
        /* LOW 当针脚输入为低时，触发中断。 */
        /* CHANGE 当针脚输入发生改变时，触发中断。 */
        /* RISING 当针脚输入由低变高时，触发中断。 */
        /* FALLING 当针脚输入由高变低时，触发中断。 */
        /* detachInterrupt 取消interrupt。 */
}

void loop()
{
        digitalWrite(pin, state);        //对观察灯写入状态值
}
