# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_BMI05X bmi05x_accel.c)
zephyr_library_sources_ifdef(CONFIG_BMI05X bmi05x_gyro.c)
zephyr_library_sources_ifdef(CONFIG_BMI05X bmi05x.c)
zephyr_library_sources_ifdef(CONFIG_BMI05X_ACCEL_TRIGGER bmi05x_accel_trigger.c)
zephyr_library_sources_ifdef(CONFIG_BMI05X_GYRO_TRIGGER bmi05x_gyro_trigger.c)

# zephyr_library_sources(bmi05x_accel.c)
# zephyr_library_sources(bmi05x_gyro.c)
# zephyr_library_sources(bmi05x.c)
# zephyr_library_sources(bmi05x_accel_trigger.c)
# zephyr_library_sources(bmi05x_gyro_trigger.c)