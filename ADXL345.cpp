/*
 * ADXL345.c - Basic library for ADXL345 sensor
 *
 * MIT License
 *
 * Copyright (c) 2017 SGN Robotica Educacional
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Wire.h>
#include <SPI.h>

#include "ADXL345.h"

/********************** Sensor registers *******************/
#define ADXL345_REG_DEVID               0x00  // Device ID.
#define ADXL345_REG_THRESH_TAP          0x1D  // Tap threshold.
#define ADXL345_REG_OFSX                0x1E  // X-axis offset.
#define ADXL345_REG_OFSY                0x1F  // Y-axis offset.
#define ADXL345_REG_OFSZ                0x20  // Z-axis offset.
#define ADXL345_REG_DUR                 0x21  // Tap duration.
#define ADXL345_REG_Latent              0x22  // Tap latency.
#define ADXL345_REG_Window              0x23  // Tap window.
#define ADXL345_REG_THRESH_ACT          0x24  // Activity threshold.
#define ADXL345_REG_THRESH_INACT        0x25  // Inactivity threshold.
#define ADXL345_REG_TIME_INACT          0x26  // Inactivity time.
#define ADXL345_REG_ACT_INACT_CTL       0x27  // Axis enable control for activity and inactivity detection.
#define ADXL345_REG_THRESH_FF           0x28  // Free-fall threshold.
#define ADXL345_REG_TIME_FF             0x29  // Free-fall time.
#define ADXL345_REG_TAP_AXES            0x2A  // Axis control for tap/double tap.
#define ADXL345_REG_ACT_TAP_STATUS      0x2B  // Source of tap/double tap.
#define ADXL345_REG_BW_RATE             0x2C  // Data rate and power mode control.
#define ADXL345_REG_POWER_CTL           0x2D  // Power-saving features control.
#define ADXL345_REG_INT_ENABLE          0x2E  // Interrupt enable control.
#define ADXL345_REG_INT_MAP             0x2F  // Interrupt mapping control.
#define ADXL345_REG_INT_SOURCE          0x30  // Source of interrupts.
#define ADXL345_REG_DATA_FORMAT         0x31  // Data format control.
#define ADXL345_REG_DATAX0              0x32  // X-Axis Data 0.
#define ADXL345_REG_DATAX1              0x33  // X-Axis Data 1.
#define ADXL345_REG_DATAY0              0x34  // Y-Axis Data 0.
#define ADXL345_REG_DATAY1              0x35  // Y-Axis Data 1.
#define ADXL345_REG_DATAZ0              0x36  // Z-Axis Data 0.
#define ADXL345_REG_DATAZ1              0x37  // Z-Axis Data 1.
#define ADXL345_REG_FIFO_CTL            0x38  // FIFO control.
#define ADXL345_REG_FIFO_STATUS         0x39  // FIFO status.


#define ADXL345_BYTE_DEVICE_ID          0xE5
#define ADXL345_BIT_PWR_REG_MEASURE    (1 << 3)

/*! ADXL345 ID */
#define ADXL345_I2C_ADDRESS             0x53


ADXL345::ADXL345(ADXL345_COMM_t commType, uint8_t csPin=0)
{
    communicationType = commType;
    _csPin = csPin;

    if(communicationType == ADXL345_COMM_SPI)
    {
        SPI.begin();
        SPI.setDataMode(SPI_MODE3);
        pinMode(csPin, OUTPUT);
        digitalWrite(csPin, HIGH);
    }
    else if(communicationType == ADXL345_COMM_I2C)
    {
        Wire.begin();
    }
}



void ADXL345::writeReg(uint8_t regAddr, uint8_t data)
{
    if(communicationType == ADXL345_COMM_I2C)
    {
        Wire.beginTransmission(ADXL345_I2C_ADDRESS);
        Wire.write(regAddr);
        Wire.write(data);
        Wire.endTransmission();
    }
    else if(communicationType == ADXL345_COMM_SPI)
    {
        digitalWrite(_csPin, LOW);
        SPI.transfer(regAddr);
        SPI.transfer(data);
        digitalWrite(_csPin, HIGH);
    }
}

uint8_t ADXL345::readReg(uint8_t regAddr)
{
    uint8_t buffer;

    if(communicationType == ADXL345_COMM_I2C)
    {
        Wire.beginTransmission(ADXL345_I2C_ADDRESS);
        Wire.write(regAddr);
        Wire.endTransmission(false);
        Wire.requestFrom(ADXL345_I2C_ADDRESS, 1, true);

        buffer = Wire.read();
    }
    else if(communicationType == ADXL345_COMM_SPI)
    {
        uint8_t spiAddress = 0x80 | regAddr;

        digitalWrite(_csPin, LOW);
        SPI.transfer(spiAddress);
        buffer = SPI.transfer(0x00);
        digitalWrite(spiAddress, HIGH);
    }

    return buffer;
}

void ADXL345::readRegs(uint8_t regAddr, uint8_t *pBuffer, uint8_t size)
{
    if(pBuffer == NULL) return;

    if(communicationType == ADXL345_COMM_I2C)
    {
        Wire.beginTransmission(ADXL345_I2C_ADDRESS);
        Wire.write(regAddr);
        Wire.endTransmission();

        Wire.requestFrom(ADXL345_I2C_ADDRESS, (int) size, true);

        while(Wire.available())
        {
            *pBuffer++ = (uint8_t) Wire.read();
        }
    }
    else if(communicationType == ADXL345_COMM_SPI)
    {
        uint8_t spiAddress = 0x80 | regAddr;

        if(size > 1)
        {
            spiAddress = spiAddress | 0x40;
        }

        digitalWrite(_csPin, LOW);
        SPI.transfer(spiAddress);
        for(uint8_t i = 0; i < size; i++)
        {
            *pBuffer++ = SPI.transfer(0x00);
        }
        digitalWrite(_csPin, HIGH);
    }
}