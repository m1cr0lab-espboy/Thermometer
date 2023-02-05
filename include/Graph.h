/**
 * ----------------------------------------------------------------------------
 * @file   Graph.h
 * @author Stéphane Calderoni (https://github.com/m1cr0lab)
 * @brief  ESPboy Digital Thermometer
 * ----------------------------------------------------------------------------
 */
#pragma once

#include <ESPboy.h>
#include <DallasTemperature.h>

struct Graph {

    uint8_t   N;       // maximum number of measurements
    uint8_t   tn;      // current number of measurements
    uint8_t   tnv;     // current number of valid measurements
    uint8_t   ti;      // current measure index

    float_t tc_low;    // low  temperature threshold in °C
    float_t tf_low;    // low  temperature threshold in °F
    float_t tc_high;   // high temperature threshold in °C
    float_t tf_high;   // high temperature threshold in °F

    float_t   last_tc; // last temperature in °C
    float_t   last_tf; // last temperature in °F
    bool      last_tv; // last temperature validity
    float_t * temp[2]; // temperature history in °C (0) and °F (1)
    bool    * valid;   // valid measurement history

    bool is_fahrenheit;

    float_t min;       // min temperature
    float_t max;       // max temperature
    float_t n_min;     // normalized min temperature
    float_t n_max;     // normalized max temperature
    float_t range;     // normalized temperature range
    float_t dr;        // unit graduation in degrees (°C or °F)

    uint8_t margin_x;  // left and right margin
    uint8_t margin_y;  // bottom margin
    uint8_t ox;        // abscissa of origin
    uint8_t oy;        // ordinate of origin
    uint8_t width;     // graph width
    uint8_t height;    // gaph  height
    uint8_t div;       // number of divisions on temperature axis
    uint8_t dy;        // unit graduation in pixels

    Graph(
        float_t const t_low,
        float_t const t_high,
        bool    const is_fahrenheit = false
    )
    : is_fahrenheit(is_fahrenheit)
    {
        if (is_fahrenheit) {
            tf_low  = t_low;  tc_low  = 5.f * (t_low  - 32.f) / 9.f;
            tf_high = t_high; tc_high = 5.f * (t_high - 32.f) / 9.f;
        } else {
            tc_low  = t_low;  tf_low  = t_low  * 1.8f + 32.f;
            tc_high = t_high; tf_high = t_high * 1.8f + 32.f;
        }
    }

    void flipUnit() { is_fahrenheit = !is_fahrenheit & 0x1; update(); }

    void init() {

        margin_x = 4;
        margin_y = 8;
        ox       = margin_x;
        width    = TFT_WIDTH - 2 * margin_x;
        height   = 48;
        oy       = TFT_HEIGHT - margin_y - height;
        N        = width;
        temp[0]  = new float_t[N];
        temp[1]  = new float_t[N];
        valid    = new bool[N];

    }

    void add(float_t const tc) {
 
       last_tv = tc != DEVICE_DISCONNECTED_C;

        if (last_tv) {
            last_tc   = temp[0][ti] = tc;
            last_tf   = temp[1][ti] = tc * 1.8f + 32.f;
            valid[ti] = true;
            if (tnv < N) tnv++;
        } else {
            valid[ti] = false;
        }

        ti++; if (ti == N) ti = 0; if (tn < N) tn++;

        if (last_tv) update();

    }

    void update() {

        if (tnv < 1) return;

        uint8_t k = (ti - tn + N) % N;

        uint8_t m = k;
        while (!valid[m]) { m++; if (m == N) m = 0; }
        min = max = temp[is_fahrenheit][m];

        for (uint8_t i = 0; i < tn; ++i) {

            if (valid[k]) {
                float_t const tk = temp[is_fahrenheit][k];
                     if (tk < min) min = tk;
                else if (tk > max) max = tk;
            }

            k++; if (k == N) k = 0;

        }

        n_min = floor(min);
        n_max = ceil(max);

        if (n_max == n_min) n_max++;

        uint8_t const r1 = n_max - n_min;
        uint8_t const r2 = 2 * (1 + floor((r1 - .1f) / 2));
        uint8_t const r3 = 3 * (1 + floor((r1 - .1f) / 3));

        if (r1 == 1) {
            range = r1;
            dr    = r1 / 8.f;
            div   = 8;
            dy    = 6;
        } else if (r2 < r3) {
            range = r2;
            dr    = r2 / 8.f;
            div   = 8;
            dy    = 6;
        } else {
            range = r3;
            dr    = r3 / 6.f;
            div   = 6;
            dy    = 8;
        }

        n_min = n_max - range;

    }

};

/*
 * ----------------------------------------------------------------------------
 * ESPboy Digital Thermometer
 * ----------------------------------------------------------------------------
 * Copyright (c) 2023 Stéphane Calderoni (https://github.com/m1cr0lab)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * ----------------------------------------------------------------------------
 */