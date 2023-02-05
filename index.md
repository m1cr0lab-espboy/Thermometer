---
layout: default
title: ESPboy Digital Thermometer
---

# {{ page.title }}

<div class="credit">
&copy; 2023 {{ site.author }}
</div>

<div class="source">
    <p class="icon github" markdown="1">
        [GitHub repository][source]
    </p>
</div>

<div class="demo" style="float:right;margin-left:2em;">
    <div class="espboy small">
        <img src="{{ 'assets/images/thermometer-128x128.gif' | relative_url }}">
    </div>
</div>

This project is very simple to implement, but can be useful.

You can make a low-cost digital thermometer on your ESPboy with the Dallas DS18B20 temperature sensor. This sensor is fairly precise and require no external components to function.

The DS18B20 sensor has a temperature range of -55°C to +125°C and an accuracy of ±0.5°C. Its resolution can be set to 9, 10, 11, or 12 bits. The default resolution at power-up is 12-bit (*i.e.*, 0.0625°C precision).

In addition, it's a 1-wire device, so it only needs one digital pin to communicate with the ESPboy's microcontroller. The provided source code uses pin <span class="tag">G2</span> on the ESPboy header, which maps to pin <span class="tag">D4</span> on the ESP8266.

<div id="wiring">
    <div class="photo"></div>
    <div class="photo"></div>
    <div class="photo"></div>
</div>

This simple project lets you collect the sensor's temperature data and plot a graph to follow its evolution over time. You can display temperatures in Celsius or Fahrenheit scale degrees and even switch from one system of units to the other at your convenience by pressing the <span class="tag">ACT</span> button.

## Customization

You can customize the way the application works by modifying the following C++ macros, which can be found at the beginning of the `src/main.cpp` source file:

```cpp
#define LOW_TEMP_THRESHOLD  25
#define HIGH_TEMP_THRESHOLD 30
#define TEMP_IS_FAHRENHEIT  false
#define DS18B20_PIN         D4
#define MEASURE_PERIOD_MS   500
```

`LOW_TEMP_THRESHOLD` and `HIGH_TEMP_THRESHOLD` determine the temperature thresholds at which emojis swap on the GUI. `TEMP_IS_FAHRENHEIT` determines whether the default temperature scale is Celsius (`false`) or Fahrenheit (`true`). `MEASURE_PERIOD_MS` is the time in milliseconds that elapses between each temperature measurement.

## Flash & Test

You can easily flash your ESPboy and test this Digital Thermometer project without having to compile the source code. Click on the <span class="tag">INSTALL</span> button below, then select the serial port to which your ESPboy is connected. Note that only Google Chrome and Microsoft Edge support this tool.

<div class="flasher">
    <esp-web-install-button
        id="upload"
        manifest="{{ 'bin/thermometer.json' | relative_url }}"
        hide-progress>
        <button id="button" slot="activate">Install</button>
        <span slot="unsupported">
            Your browser does not support Web Serial, which is required to connect to ESPboy. Use Google Chrome or Microsoft Edge instead...
        </span>
        <span slot="not-allowed">You are not allowed to use this on HTTP!</span>
    </esp-web-install-button>
    <div class="progress-bar">
        <span style="width:0%"></span>
    </div>
</div>


[source]: https://github.com/m1cr0lab-espboy/Thermometer
