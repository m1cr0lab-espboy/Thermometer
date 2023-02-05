/**
 * ----------------------------------------------------------------------------
 * @file   main.cpp
 * @author Stéphane Calderoni (https://github.com/m1cr0lab)
 * @brief  ESPboy Digital Thermometer
 * ----------------------------------------------------------------------------
 */
#include <ESPboy.h>
#include <DallasTemperature.h>
#include "assets.h"
#include "Graph.h"

// --------------------------------
// CUSTOMIZATION
// --------------------------------
#define LOW_TEMP_THRESHOLD  25
#define HIGH_TEMP_THRESHOLD 30
#define TEMP_IS_FAHRENHEIT  false
#define DS18B20_PIN         D4
#define MEASURE_PERIOD_MS   500
// --------------------------------

#define MCP23017_LED_LOCK_PIN 9
#define IDLE_DELAY_MS         1500

enum class State : uint8_t { IDLE, START, MEASUREMENT };

OneWire           ds18b20_bus(DS18B20_PIN);
DallasTemperature ds18b20(&ds18b20_bus);
LGFX_Sprite       framebuffer(&espboy.tft);
State             state;
uint32_t          last_ms;

Graph graph(LOW_TEMP_THRESHOLD, HIGH_TEMP_THRESHOLD, TEMP_IS_FAHRENHEIT);

void showWaiting() {

    framebuffer.setTextDatum(CC_DATUM);
    framebuffer.setTextSize(1);
    framebuffer.setTextColor(COLOR_BRIGHT);
    framebuffer.drawString(
        "Waiting for data",
        TFT_WIDTH  >> 1,
        TFT_HEIGHT >> 1,
        &fonts::Font0
    );

}

void drawThermometer(uint8_t const x, uint8_t const y, float_t const t) {

    static float_t h;

    float_t       dt = t - graph.min;
    float_t       r  = graph.max - graph.min; if (r == 0) r = graph.range;
    float_t const th = GAUGE_HEIGHT * dt / r;
    float_t const dh = h - th;

    if (graph.last_tv) h -= dh * .1f;

    framebuffer.pushImage(
        x,
        y,
        THERMOMETER_WIDTH,
        THERMOMETER_HEIGHT,
        THERMOMETER_COLORMAP
    );

    LGFX_Sprite gauge(&framebuffer);
    float_t const h_offset = GAUGE_HEIGHT - h;

    gauge.createSprite(GAUGE_WIDTH, h + 1);
    gauge.pushImage(0, -h_offset, GAUGE_WIDTH, GAUGE_HEIGHT, GAUGE_COLORMAP);
    gauge.pushSprite(x + GAUGE_PADDING, y + GAUGE_PADDING + h_offset);
    gauge.deleteSprite();

}

void drawTinyFont(char const * const str, uint8_t x, uint8_t const y) {

    uint8_t const n = strlen(str);
    uint8_t       k;

    for (uint8_t i = 0; i < n; ++i) {

        uint8_t const c = str[i];
        if (c == 0x20) { x += 5; continue; }
        switch (c) {
            case 0x2b: k = 0; break;
            case 0x2d: k = 1; break;
            case 0x2e: k = 2; break;
            default:   k = 3 + c - 0x30;
        }

        framebuffer.drawBitmap(
            x,
            y,
            TINY_FONT + k * TINY_FONT_HEIGHT,
            TINY_FONT_WIDTH,
            TINY_FONT_HEIGHT,
            COLOR_BRIGHT
        );

        x += 5;

    }

}

void drawGraph() {

    uint8_t flip = 0;

    for (uint8_t i = 0; i <= graph.div; ++i) {

        uint8_t const y = graph.oy + i * graph.dy;

        framebuffer.drawFastHLine(
            graph.ox,
            y,
            graph.width,
            i == graph.div
                ? COLOR_AXES
                : flip ? COLOR_GRAD_D : COLOR_GRAD_B
        );

        flip = !flip & 0x1;

    }

    uint8_t k = graph.ti;
    for (uint8_t i = 0; i < graph.tn; ++i) {

        if (graph.valid[k]) {

            float_t t  = graph.temp[graph.is_fahrenheit][k];
            float_t dt = t - graph.n_min;
            float_t w  = graph.width  * i  / graph.N;
            float_t h  = (float_t) graph.height * dt / graph.range;
            uint8_t x  = graph.ox + graph.width - 1 - w;
            uint8_t y  = graph.oy + graph.height;

            framebuffer.drawGradientVLine(x, y - 1, -h + 1, COLOR_GRAPH_D, COLOR_GRAPH_B);
            framebuffer.drawPixel(x, y - h, COLOR_GRAPH_P);

        }

        !k ? k = graph.N : k--;

    }

    flip = 0;
    for (uint8_t i = 0; i <= graph.div; ++i) {

        if (!(i & 0x1)) {

            uint8_t const y = graph.oy + i * graph.dy;

            framebuffer.drawFastHLine(graph.ox - 1, y, 3, COLOR_AXES);
            framebuffer.drawFastHLine(graph.ox + graph.width - 2, y, 3, COLOR_AXES);

            char buffer[7];
                 if (abs(graph.dr) == .125f) snprintf(buffer, 7, "%5.2f", graph.n_max - i * graph.dr);
            else if (abs(graph.dr) ==  .25f) snprintf(buffer, 7, "%4.1f", graph.n_max - i * graph.dr);
            else                             snprintf(buffer, 7,    "%u", (int8_t) (graph.n_max - i * graph.dr));
            drawTinyFont(buffer, graph.ox + 4, y - 2);

        }

        flip = !flip & 0x1;

    }

    framebuffer.drawFastVLine(
        graph.ox,
        graph.oy,
        graph.height,
        COLOR_AXES
    );

    framebuffer.drawFastVLine(
        graph.ox + graph.width - 1,
        graph.oy,
        graph.height,
        COLOR_AXES
    );

}

void showTemperature() {

    float_t t = graph.min;
    if (graph.last_tv) {
        t = graph.is_fahrenheit
            ? graph.last_tf
            : graph.last_tc;
    }

    drawThermometer(6, 8, t);

    if (graph.last_tv) {

        float_t const t_high = graph.is_fahrenheit ? graph.tf_high : graph.tc_high;
        float_t const t_low  = graph.is_fahrenheit ? graph.tf_low  : graph.tc_low;
        uint8_t hue;
        uint8_t emoji_index;

        if (t < t_low) {
            hue         = HUE_TEMP_LOW;
            emoji_index = 0;
        } else if (t > t_high) {
            hue         = HUE_TEMP_HIGH;
            emoji_index = 2;
        } else {
            hue         = HUE_TEMP_LOW + (HUE_TEMP_HIGH - HUE_TEMP_LOW) * (t - t_low) / (t_high - t_low);
            emoji_index = 1;
        }

        framebuffer.pushImage(
            (TFT_WIDTH - EMOJI_SIZE) >> 1,
            4,
            EMOJI_SIZE,
            EMOJI_SIZE,
            EMOJI_COLORMAP + emoji_index * EMOJI_FRAME_SIZE
        );

        framebuffer.setTextDatum(BC_DATUM);
        framebuffer.setTextSize(.54f);
        framebuffer.setTextColor(Color::hsv2rgb565(hue, 0xcc, 0xff));
        framebuffer.drawFloat(
            graph.is_fahrenheit ? graph.last_tf : graph.last_tc,
            1,
            TFT_WIDTH >> 1,
            60,
            &fonts::Font7
        );

        framebuffer.pushImage(
            TFT_WIDTH - 8 - TEMP_UNIT_WIDTH,
            60 - (TEMP_UNIT_HEIGHT << 1),
            TEMP_UNIT_WIDTH,
            TEMP_UNIT_HEIGHT,
            TEMP_UNIT_COLORMAP + (graph.is_fahrenheit ? TEMP_FAHRENHEIT : 0)
        );

        char buffer[8];
        snprintf(
            buffer, 8, "%.1f  ",
            graph.is_fahrenheit ? graph.last_tc : graph.last_tf
        );
        framebuffer.setTextDatum(TR_DATUM);
        framebuffer.setTextSize(1);
        framebuffer.setTextColor(COLOR_UNIT);
        framebuffer.drawString(buffer, TFT_WIDTH - 8, 4 + ((EMOJI_SIZE - 8) >> 1));
        framebuffer.pushImageRotateZoom(
            TFT_WIDTH - 8 - .3f * TEMP_UNIT_WIDTH,
            3 + ((EMOJI_SIZE - 8) >> 1) + .3f * TEMP_UNIT_HEIGHT,
            TEMP_UNIT_WIDTH >> 1, TEMP_UNIT_HEIGHT >> 1,
            0,
            .6f, .6f,
            TEMP_UNIT_WIDTH,
            TEMP_UNIT_HEIGHT,
            TEMP_UNIT_COLORMAP + (graph.is_fahrenheit ? 0 : TEMP_FAHRENHEIT)
        );

    } else {

        framebuffer.setTextDatum(CC_DATUM);
        framebuffer.setTextSize(1);
        framebuffer.setTextColor(COLOR_ERROR);
        framebuffer.drawString(
            "Sensor error",
            (TFT_WIDTH >> 1) + 4,
            6 + (THERMOMETER_HEIGHT >> 1),
            &fonts::Font0
        );

    }

    if (graph.tnv > 1) drawGraph();

}

void getTemperature() {

    uint32_t const now_ms = millis();
    if (now_ms - last_ms < MEASURE_PERIOD_MS) return;

                       ds18b20.requestTemperatures();
    float_t const tc = ds18b20.getTempCByIndex(0);

    graph.add(tc);
    last_ms = now_ms;

}

void idle() {

    uint32_t const now_ms = millis();

    if (now_ms - last_ms < IDLE_DELAY_MS) return;

    state   = State::START;
    last_ms = now_ms;

}

void start() {

    espboy.mcp.digitalWrite(MCP23017_LED_LOCK_PIN, LOW);
    ds18b20.begin();
    graph.init();
    state = State::MEASUREMENT;
    last_ms = millis();

}

void setup() {

    espboy.begin();
    framebuffer.createSprite(TFT_WIDTH, TFT_HEIGHT);

    state   = State::IDLE;
    last_ms = millis();

}

void loop() {

    espboy.update();

    switch (state) {

        case State::IDLE:
            idle();
            break;

        case State::START:
            start();
            break;

        case State::MEASUREMENT:
            if (espboy.button.pressed(Button::ACT)) graph.flipUnit();
            getTemperature();

    }

    framebuffer.clear(COLOR_BG);

    state == State::MEASUREMENT && graph.tn > 1
        ? showTemperature()
        : showWaiting();

    framebuffer.pushSprite(0, 0);

}

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