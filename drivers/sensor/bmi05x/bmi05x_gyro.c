/* Bosch BMI05X inertial measurement unit driver
 *
 * Copyright (c) 2022 Meta Platforms, Inc. and its affiliates
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/sensor.h>
#include <zephyr/pm/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/byteorder.h>

#define DT_DRV_COMPAT bosch_bmi05x_gyro
//#include "bmi05x.h"
//#include <app/drivers/bmi05x.h>
#include <app/drivers/sensor/bmi05x.h>

LOG_MODULE_REGISTER(BMI05X_GYRO, CONFIG_SENSOR_LOG_LEVEL);

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

static int bmi05x_gyro_transceive_i2c(const struct device *dev, uint8_t reg, bool write, void *data,
				      size_t length)
{
	const struct bmi05x_gyro_config *bmi05x = dev->config;

	if (!write) {
		return i2c_write_read_dt(&bmi05x->bus.i2c, &reg, 1, data, length);
	}
	if (length > CONFIG_BMI05X_I2C_WRITE_BURST_SIZE) {
		return -EINVAL;
	}
	uint8_t buf[1 + CONFIG_BMI05X_I2C_WRITE_BURST_SIZE];

	buf[0] = reg;
	memcpy(&buf[1], data, length);
	return i2c_write_dt(&bmi05x->bus.i2c, buf, 1 + length);
}

static int bmi05x_bus_check_i2c(const union bmi05x_bus *bus)
{
	return i2c_is_ready_dt(&bus->i2c) ? 0 : -ENODEV;
}

static const struct bmi05x_gyro_bus_io bmi05x_i2c_api = {
	.check = bmi05x_bus_check_i2c,
	.transceive = bmi05x_gyro_transceive_i2c,
};

#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c) */

#if DT_ANY_INST_ON_BUS_STATUS_OKAY(spi)

static int bmi05x_gyro_transceive_spi(const struct device *dev, uint8_t reg, bool write, void *data,
				      size_t length)
{
	const struct bmi05x_gyro_config *bmi05x = dev->config;
	const struct spi_buf tx_buf[2] = {{.buf = &reg, .len = 1}, {.buf = data, .len = length}};
	const struct spi_buf_set tx = {.buffers = tx_buf, .count = write ? 2 : 1};

	if (!write) {
		uint16_t dummy;
		const struct spi_buf rx_buf[2] = {{.buf = &dummy, .len = 1},
						  {.buf = data, .len = length}};
		const struct spi_buf_set rx = {.buffers = rx_buf, .count = 2};

		return spi_transceive_dt(&bmi05x->bus.spi, &tx, &rx);
	}

	return spi_write_dt(&bmi05x->bus.spi, &tx);
}

static int bmi05x_bus_check_spi(const union bmi05x_bus *bus)
{
	return spi_is_ready_dt(&bus->spi) ? 0 : -ENODEV;
}

static const struct bmi05x_gyro_bus_io bmi05x_spi_api = {
	.check = bmi05x_bus_check_spi,
	.transceive = bmi05x_gyro_transceive_spi,
};

#endif /* DT_ANY_INST_ON_BUS_STATUS_OKAY(spi) */

static inline int bmi05x_bus_check(const struct device *dev)
{
	const struct bmi05x_gyro_config *config = dev->config;

	return config->api->check(&config->bus);
}

static int bmi05x_gyro_transceive(const struct device *dev, uint8_t reg, bool write, void *data,
				  size_t length)
{
	const struct bmi05x_gyro_config *cfg = dev->config;

	return cfg->api->transceive(dev, reg, write, data, length);
}

int bmi05x_gyro_read(const struct device *dev, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
	return bmi05x_gyro_transceive(dev, reg_addr | BIT(7), false, data, len);
}

int bmi05x_gyro_byte_read(const struct device *dev, uint8_t reg_addr, uint8_t *byte)
{
	return bmi05x_gyro_transceive(dev, reg_addr | BIT(7), false, byte, 1);
}

int bmi05x_gyro_byte_write(const struct device *dev, uint8_t reg_addr, uint8_t byte)
{
	return bmi05x_gyro_transceive(dev, reg_addr & 0x7F, true, &byte, 1);
}

int bmi05x_gyro_word_write(const struct device *dev, uint8_t reg_addr, uint16_t word)
{
	uint8_t tx_word[2] = {(uint8_t)(word & 0xff), (uint8_t)(word >> 8)};

	return bmi05x_gyro_transceive(dev, reg_addr & 0x7F, true, tx_word, 2);
}

int bmi05x_gyro_reg_field_update(const struct device *dev, uint8_t reg_addr, uint8_t pos,
				 uint8_t mask, uint8_t val)
{
	uint8_t old_val;
	int ret;

	ret = bmi05x_gyro_byte_read(dev, reg_addr, &old_val);
	if (ret < 0) {
		return ret;
	}

	return bmi05x_gyro_byte_write(dev, reg_addr, (old_val & ~mask) | ((val << pos) & mask));
}

static const struct bmi05x_range bmi05x_gyr_range_map[] = {
	{125, BMI05X_GYR_RANGE_125DPS},   {250, BMI05X_GYR_RANGE_250DPS},
	{500, BMI05X_GYR_RANGE_500DPS},   {1000, BMI05X_GYR_RANGE_1000DPS},
	{2000, BMI05X_GYR_RANGE_2000DPS},
};
#define BMI05X_GYR_RANGE_MAP_SIZE ARRAY_SIZE(bmi05x_gyr_range_map)

int32_t bmi05x_gyr_reg_val_to_range(uint8_t reg_val)
{
	return bmi05x_reg_val_to_range(reg_val, bmi05x_gyr_range_map, BMI05X_GYR_RANGE_MAP_SIZE);
}

static int bmi05x_gyr_odr_set(const struct device *dev, uint16_t freq_int, uint16_t freq_milli)
{
	int odr = bmi05x_freq_to_odr_val(freq_int, freq_milli);

	if (odr < 0) {
		return odr;
	}

	if (odr < BMI05X_GYRO_BW_532_ODR_2000_HZ || odr > BMI05X_GYRO_BW_32_ODR_100_HZ) {
		return -ENOTSUP;
	}

	return bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_BANDWIDTH, (uint8_t)odr);
}

static int bmi05x_gyr_range_set(const struct device *dev, uint16_t range)
{
	struct bmi05x_gyro_data *bmi05x = dev->data;
	int32_t reg_val =
		bmi05x_range_to_reg_val(range, bmi05x_gyr_range_map, BMI05X_GYR_RANGE_MAP_SIZE);
	int ret;

	if (reg_val < 0) {
		return reg_val;
	}

	ret = bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_RANGE, reg_val);
	if (ret < 0) {
		return ret;
	}

	bmi05x->scale = BMI05X_GYR_SCALE(range);

	return ret;
}

static int bmi05x_gyr_config(const struct device *dev, enum sensor_channel chan,
			     enum sensor_attribute attr, const struct sensor_value *val)
{
	switch (attr) {
	case SENSOR_ATTR_FULL_SCALE:
		return bmi05x_gyr_range_set(dev, sensor_rad_to_degrees(val));
	case SENSOR_ATTR_SAMPLING_FREQUENCY:
		return bmi05x_gyr_odr_set(dev, val->val1, val->val2 / 1000);
	default:
		LOG_DBG("Gyro attribute not supported.");
		return -ENOTSUP;
	}
}

static int bmi05x_attr_set(const struct device *dev, enum sensor_channel chan,
			   enum sensor_attribute attr, const struct sensor_value *val)
{
#ifdef CONFIG_PM_DEVICE
	enum pm_device_state state;

	(void)pm_device_state_get(dev, &state);
	if (state != PM_DEVICE_STATE_ACTIVE) {
		return -EBUSY;
	}
#endif

	switch (chan) {
	case SENSOR_CHAN_GYRO_X:
	case SENSOR_CHAN_GYRO_Y:
	case SENSOR_CHAN_GYRO_Z:
	case SENSOR_CHAN_GYRO_XYZ:
		return bmi05x_gyr_config(dev, chan, attr, val);
	default:
		LOG_DBG("attr_set() not supported on this channel.");
		return -ENOTSUP;
	}
}

static int bmi05x_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct bmi05x_gyro_data *bmi05x = dev->data;
	size_t i;
	int ret;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_GYRO_XYZ) {
		LOG_DBG("Unsupported sensor channel");
		return -ENOTSUP;
	}

	ret = bmi05x_gyro_read(dev, BMI05X_REG_GYRO_X_LSB, (uint8_t *)bmi05x->gyr_sample,
			       sizeof(bmi05x->gyr_sample));
	if (ret < 0) {
		return ret;
	}

	/* convert samples to cpu endianness */
	for (i = 0; i < ARRAY_SIZE(bmi05x->gyr_sample); i++) {
		bmi05x->gyr_sample[i] = sys_le16_to_cpu(bmi05x->gyr_sample[i]);
	}

	return ret;
}

static void bmi05x_to_fixed_point(int16_t raw_val, uint16_t scale, struct sensor_value *val)
{
	int32_t converted_val;

	/*
	 * maximum converted value we can get is: max(raw_val) * max(scale)
	 *	max(raw_val) = +/- 2^15
	 *	max(scale) = 4785
	 *	max(converted_val) = 156794880 which is less than 2^31
	 */
	converted_val = raw_val * scale;
	val->val1 = converted_val / 1000000;
	val->val2 = converted_val % 1000000;
}

static void bmi05x_channel_convert(enum sensor_channel chan, uint16_t scale, uint16_t *raw_xyz,
				   struct sensor_value *val)
{
	int i;
	uint8_t ofs_start, ofs_stop;

	switch (chan) {
	case SENSOR_CHAN_GYRO_X:
		ofs_start = ofs_stop = 0U;
		break;
	case SENSOR_CHAN_GYRO_Y:
		ofs_start = ofs_stop = 1U;
		break;
	case SENSOR_CHAN_GYRO_Z:
		ofs_start = ofs_stop = 2U;
		break;
	default:
		ofs_start = 0U;
		ofs_stop = 2U;
		break;
	}

	for (i = ofs_start; i <= ofs_stop; i++, val++) {
		bmi05x_to_fixed_point(raw_xyz[i], scale, val);
	}
}

static inline void bmi05x_gyr_channel_get(const struct device *dev, enum sensor_channel chan,
					  struct sensor_value *val)
{
	struct bmi05x_gyro_data *bmi05x = dev->data;

	bmi05x_channel_convert(chan, bmi05x->scale, bmi05x->gyr_sample, val);
}

static int bmi05x_channel_get(const struct device *dev, enum sensor_channel chan,
			      struct sensor_value *val)
{
#ifdef CONFIG_PM_DEVICE
	enum pm_device_state state;

	(void)pm_device_state_get(dev, &state);
	if (state != PM_DEVICE_STATE_ACTIVE) {
		return -EBUSY;
	}
#endif

	switch ((int16_t)chan) {
	case SENSOR_CHAN_GYRO_X:
	case SENSOR_CHAN_GYRO_Y:
	case SENSOR_CHAN_GYRO_Z:
	case SENSOR_CHAN_GYRO_XYZ:
		bmi05x_gyr_channel_get(dev, chan, val);
		return 0;
	default:
		LOG_DBG("Channel not supported.");
		return -ENOTSUP;
	}
}

#ifdef CONFIG_PM_DEVICE
static int bmi05x_gyro_pm_action(const struct device *dev, enum pm_device_action action)
{
	uint8_t reg_val;
	int ret;

	switch (action) {
	case PM_DEVICE_ACTION_RESUME:
		reg_val = BMI05X_GYRO_PM_NORMAL;
		break;
	case PM_DEVICE_ACTION_SUSPEND:
		reg_val = BMI05X_GYRO_PM_SUSPEND;
		break;
	default:
		return -ENOTSUP;
	}

	ret = bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_LPM1, reg_val);
	if (ret < 0) {
		LOG_ERR("Failed to set power mode");
		return ret;
	}
	k_msleep(BMI05X_GYRO_POWER_MODE_CONFIG_DELAY);

	return ret;
}
#endif /* CONFIG_PM_DEVICE */

static const struct sensor_driver_api bmi05x_api = {
	.attr_set = bmi05x_attr_set,
#ifdef CONFIG_BMI05X_GYRO_TRIGGER
	.trigger_set = bmi05x_trigger_set_gyr,
#endif
	.sample_fetch = bmi05x_sample_fetch,
	.channel_get = bmi05x_channel_get,
};

int bmi05x_gyro_init(const struct device *dev)
{
	const struct bmi05x_gyro_config *config = dev->config;
	uint8_t val = 0U;
	int ret;

	ret = bmi05x_bus_check(dev);
	if (ret < 0) {
		LOG_ERR("Bus not ready for '%s'", dev->name);
		return ret;
	}

	/* Write the default page as zero*/
	ret = bmi05x_gyro_byte_write(dev, 0X07, 0X00);
	if (ret < 0) {
		LOG_ERR("Cannot reboot chip.");
		return ret;
	}

	/* reboot the chip */
	/*ret = bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_SOFTRESET, BMI05X_SOFT_RESET_CMD);
	if (ret < 0) {
		LOG_ERR("Cannot reboot chip.");
		return ret;
	}*/

	k_msleep(BMI05X_GYRO_SOFTRESET_DELAY);

	//ret = bmi05x_gyro_byte_read(dev, BMI05X_REG_GYRO_CHIP_ID, &val);
	ret = bmi05x_gyro_byte_read(dev, 0x00, &val);
	if (ret < 0) {
		LOG_ERR("Failed to read chip id.");
		return ret;
	}
	LOG_INF("chip detected (0x%02x)!", val);
	if (val != BMI05X_GYRO_CHIP_ID) {
		LOG_ERR("Unsupported chip detected (0x%02x)!", val);
		return -ENODEV;
	}

	/* set gyro default range */
	/*ret = bmi05x_gyr_range_set(dev, config->gyro_fs);
	if (ret < 0) {
		LOG_ERR("Cannot set default range for gyroscope.");
		return ret;
	}*/

	/* set gyro default bandwidth */
	/*ret = bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_BANDWIDTH, config->gyro_hz);
	if (ret < 0) {
		LOG_ERR("Failed to set gyro's default ODR.");
		return ret;
	}*/

#ifdef CONFIG_BMI05X_GYRO_TRIGGER
	ret = bmi05x_gyr_trigger_mode_init(dev);
	if (ret < 0) {
		LOG_ERR("Cannot set up trigger mode.");
		return ret;
	}
#endif
/* with BMI05X_DATA_SYNC set, it is expected that the INT3 or INT4 is wired to either INT1
 * or INT2
 */
#if defined(CONFIG_BMI05X_GYRO_TRIGGER) || BMI05X_GYRO_ANY_INST_HAS_DATA_SYNC
	/* set gyro ints */
	/* set ints */
	ret = bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_INT_CTRL, 0x80);
	if (ret < 0) {
		LOG_ERR("Failed to map interrupts.");
		return ret;
	}
	ret = bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_INT3_INT4_IO_CONF,
				     config->int3_4_conf_io);
	if (ret < 0) {
		LOG_ERR("Failed to map interrupts.");
		return ret;
	}
	ret = bmi05x_gyro_byte_write(dev, BMI05X_REG_GYRO_INT3_INT4_IO_MAP, config->int3_4_map);
	if (ret < 0) {
		LOG_ERR("Failed to map interrupts.");
		return ret;
	}
#endif

	return ret;
}

#define BMI05X_CONFIG_SPI(inst)                                                                    \
	.bus.spi = SPI_DT_SPEC_INST_GET(                                                           \
		inst, SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8), 2),

#define BMI05X_CONFIG_I2C(inst) .bus.i2c = I2C_DT_SPEC_INST_GET(inst),

#define BMI05X_GYRO_TRIG(inst)                                                                     \
	.int3_4_map = DT_INST_PROP(inst, int3_4_map_io),                                           \
	.int3_4_conf_io = DT_INST_PROP(inst, int3_4_conf_io),

#if BMI05X_GYRO_ANY_INST_HAS_DATA_SYNC
/* the bmi05x-gyro should not have trigger mode with data-sync enabled */
BUILD_ASSERT(CONFIG_BMI05X_GYRO_TRIGGER_NONE,
	     "Only none trigger type allowed for bmi05x-gyro with data-sync enabled");
/* with data-sync, one of the int pins should be wired directory to the accel's int pins, their
 * config should be defined
 */
#define BMI05X_GYRO_TRIGGER_PINS(inst) BMI05X_GYRO_TRIG(inst)
#else
#define BMI05X_GYRO_TRIGGER_PINS(inst)                                                             \
	IF_ENABLED(CONFIG_BMI05X_GYRO_TRIGGER, (BMI05X_GYRO_TRIG(inst)))
#endif

#define BMI05X_CREATE_INST(inst)                                                                   \
                                                                                                   \
	static struct bmi05x_gyro_data bmi05x_drv_##inst;                                          \
                                                                                                   \
	static const struct bmi05x_gyro_config bmi05x_config_##inst = {                            \
		COND_CODE_1(DT_INST_ON_BUS(inst, spi), (BMI05X_CONFIG_SPI(inst)),                  \
			    (BMI05X_CONFIG_I2C(inst)))                                             \
			.api = COND_CODE_1(DT_INST_ON_BUS(inst, spi), (&bmi05x_spi_api),           \
					   (&bmi05x_i2c_api)),                                     \
		IF_ENABLED(CONFIG_BMI05X_GYRO_TRIGGER,                                             \
			   (.int_gpio = GPIO_DT_SPEC_INST_GET(inst, int_gpios),))                  \
			.gyro_hz = DT_INST_ENUM_IDX(inst, gyro_hz),                                \
		BMI05X_GYRO_TRIGGER_PINS(inst).gyro_fs = DT_INST_PROP(inst, gyro_fs),              \
	};                                                                                         \
                                                                                                   \
	PM_DEVICE_DT_INST_DEFINE(inst, bmi05x_gyro_pm_action);                                     \
	SENSOR_DEVICE_DT_INST_DEFINE(inst, bmi05x_gyro_init, PM_DEVICE_DT_INST_GET(inst),          \
				     &bmi05x_drv_##inst, &bmi05x_config_##inst, POST_KERNEL,       \
				     CONFIG_SENSOR_INIT_PRIORITY, &bmi05x_api);

/* Create the struct device for every status "okay" node in the devicetree. */
DT_INST_FOREACH_STATUS_OKAY(BMI05X_CREATE_INST)
