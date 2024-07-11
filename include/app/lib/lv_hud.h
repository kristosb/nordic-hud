/*
MIT License

Copyright (c) 2024 kristosb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef APP_LIB_LV_HUD_H_
#define APP_LIB_LV_HUD_H_

/**
 * @defgroup lib_LV_HUD Heads up display LVGL library
 * @ingroup lib
 * @{
 *
 * @brief Head up display LVGL out-of-tree library.
 *
 * This library Visualise data from IMU sensor
 * as compas and pitch ladder indicator.
 */

/**
 * @brief Return @p val if non-zero, or Kconfig-controlled default.
 *
 * Function returns the provided value if non-zero, or a Kconfig-controlled
 * default value if the parameter is zero. This trivial function is provided in
 * order to have a library interface example that is trivial to test.
 *
 * @param val Value to return if non-zero
 *
 * @retval val if @p val is non-zero
 * @retval CONFIG_LV_HUD_GET_VALUE_DEFAULT if @p val is zero
 */
int custom_get_value(int val);

/** @} */

#endif /* APP_LIB_LV_HUD_H_ */
