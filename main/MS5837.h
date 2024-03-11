/*

MS5837 Pressure I2C Driver

Author: Tedi Qafko
Created: February 4th, 2024

*/

// #ifndef MS5837_I2C_DRIVER_H
// #define MS5837_I2C_DRIVER_H

#include "driver/i2c.h" /*  Needed for I2C */

const uint8_t MS5837_ADDR = 0x76; /* 1110110x write -> x = 0, read -> x = 1 (p. 9)*/
const uint8_t MS5837_RESET = 0x1E;
const uint8_t MS5837_ADC_READ = 0x00;
const uint8_t MS5837_PROM_READ = 0xA0;
const uint8_t MS5837_CONVERT_D1_8192 = 0x4A;
const uint8_t MS5837_CONVERT_D2_8192 = 0x5A;

const float MS5837::Pa = 100.0f;
const float MS5837::bar = 0.001f;
const float MS5837::mbar = 1.0f;

const uint8_t MS5837::MS5837_30BA = 0;
const uint8_t MS5837::MS5837_02BA = 1;
const uint8_t MS5837::MS5837_UNRECOGNISED = 255;

const uint8_t MS5837_02BA01 = 0x00; // Sensor version: From MS5837_02BA datasheet Version PROM Word 0
const uint8_t MS5837_02BA21 = 0x15; // Sensor version: From MS5837_02BA datasheet Version PROM Word 0
const uint8_t MS5837_30BA26 = 0x1A; // Sensor version: From MS5837_30BA datasheet Version PROM Word 0

class MS5837 {
public:
	static const float Pa;
	static const float bar;
	static const float mbar;

	static const uint8_t MS5837_30BA;
	static const uint8_t MS5837_02BA;
	static const uint8_t MS5837_UNRECOGNISED;

	MS5837();

	bool init(TwoWire &wirePort = Wire);
	bool begin(TwoWire &wirePort = Wire); // Calls init()

	/** Set model of MS5837 sensor. Valid options are MS5837::MS5837_30BA (default)
	 * and MS5837::MS5837_02BA.
	 */
	void setModel(uint8_t model);
	uint8_t getModel();

	/** Provide the density of the working fluid in kg/m^3. Default is for
	 * seawater. Should be 997 for freshwater.
	 */
	void setFluidDensity(float density);

	/** The read from I2C takes up to 40 ms, so use sparingly is possible.
	 */
	void read();

	/** Pressure returned in mbar or mbar*conversion rate.
	 */
	float pressure(float conversion = 1.0f);

	/** Temperature returned in deg C.
	 */
	float temperature();

	/** Depth returned in meters (valid for operation in incompressible
	 *  liquids only. Uses density that is set for fresh or seawater.
	 */
	float depth();

	/** Altitude returned in meters (valid for operation in air only).
	 */
	float altitude();

private:

	//This stores the requested i2c port
	TwoWire * _i2cPort;

	uint16_t C[8];
	uint32_t D1_pres, D2_temp;
	int32_t TEMP;
	int32_t P;
	uint8_t _model;

	float fluidDensity;

	/** Performs calculations per the sensor data sheet for conversion and
	 *  second order compensation.
	 */
	void calculate();

	uint8_t crc4(uint16_t n_prom[]);
};

#endif
