//////////////////////////////////
#include <Wire.h>
/* 调试失败。 */
/* SCL接D1，SDA接D2。 */
#define SCL 5
#define SDA 4

#define uint16_t unsigned int
/* #define iic_add 0x5b */
#define iic_add 0xb6

typedef struct
{
        uint32_t P;
        uint16_t Temp;
        uint16_t Hum;
        uint16_t Alt;
} bme;

bme Bme;
uint32_t Lux;

void iic_read(unsigned char reg,unsigned char *data,uint8_t len )
{
        Wire.beginTransmission(iic_add);
        Wire.write(reg);
        Wire.endTransmission();
        delayMicroseconds(10);
        if(len>4)
                Wire.requestFrom(iic_add,10);
        else
                Wire.requestFrom(iic_add,4);
        while (Wire.available()) //
        {
                for (uint8_t i = 0; i < len; i++)
                {
                        data[i] = Wire.read();
                }
        }
}

void get_bme(void)
{
        uint16_t data_16[2]={0};
        uint8_t data[10]={0};
        iic_read(0x45,data,10);
        Bme.Temp=(data[0]<<8)|data[1];
        data_16[0]=(data[2]<<8)|data[3];
        data_16[1]=(data[4]<<8)|data[5];
        Bme.P=(((uint32_t)data_16[0])<<16)|data_16[1];
        Bme.Hum=(data[6]<<8)|data[7];
        Bme.Alt=(data[8]<<8)|data[9];
}
void get_lux(void)
{
        uint16_t data_16[2]={0};
        uint8_t data[10]={0};
        iic_read(0x15,data,4);
        data_16[0]=(data[0]<<8)|data[1];
        data_16[1]=(data[2]<<8)|data[3];
        Lux=(((uint32_t)data_16[0])<<16)|data_16[1];

}


void setup() {
        Serial.begin(115200);
        /* Wire.setClock(40000); */
        Wire.begin(SDA, SCL);
        delay(1);
}
void loop() {
        get_bme();
        Serial.print("Temp: ");
        Serial.print( (float)Bme.Temp/100);
        Serial.print(" DegC  PRESS : ");
        Serial.print( ((float)Bme.P)/100);
        Serial.print(" Pa  HUM : ");
        Serial.print( (float)Bme.Hum/100);
        Serial.print(" % ALT:");
        Serial.print( Bme.Alt);
        Serial.println("m");
        get_lux();
        Serial.print( "Lux: ");
        Serial.print( ((float)Lux)/100);
        Serial.println(" lux");

        delay(1000);
}
