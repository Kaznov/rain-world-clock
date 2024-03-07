# Rain World Clock

**Rain World Clock** is a digital clock inspired by [Rain World](http://rainworldgame.com/) game. 

Based on E-Ink paper display and ESP-8266 MCU, using the Internet connection to keep the current time, date and timezone.

It is a fan project, not affiliated with VideoCult.

# Final product

# Features
- A different picture based on the day of the week / time of the day / special events,
- Keeping the time using WiFi connection,
- Different timezones support,
- Automatic switch to Daylight Saving Time

# Planned features:
- A companion Android app for easier configuration,
- Adding custom pictures,
- Weather info and forecast,
- Synchronization with calendar

# Build it yourself!

**It's easy!**\*

<sub>\*It's not that hard</sub>

You will need:

- 800x480 e-ink display.
  
  I used [raw display from Waveshare](https://www.waveshare.com/product/displays/e-paper/epaper-1/7.5inch-e-paper.htm), and later put it in a picture frame.
  
  You can get [a display already in the frame from Waveshare](https://www.waveshare.com/product/displays/e-paper/epaper-1/7.5inch-e-paper-g.htm), but it's a bit more expensive. However, this way you get a perfectly-fit frame with acrylic glass.
  
  I haven't tried it, but you should be able to use [a raw display from GoodDisplay](https://buyepaper.com/products/gdey075t7) without any changes to the code.
  
  Remember to check some local [distributors](https://www.waveshare.com/distributors) and save money on shipment!
  
- An ESP-8266 MCU board.

  The easiest approach is to buy a [dedicated board](https://www.waveshare.com/product/displays/e-paper/driver-boards/e-paper-esp8266-driver-board.htm) for e-ink displays. They will be always marked as e-Paper driver boards. This way the setup is a bit easier, as you don't have to manually plug-in separate cables, everything is connected by one tape-cable.
  
  If you already have an ESP-8266, you can connect it using [e-Paper Raw Panel Driver HAT](https://www.waveshare.com/product/displays/e-paper/driver-boards/e-paper-driver-hat.htm) and a JST PH2.0 8P cable (I'm not sure if it's included in the HAT package). If you use your own ESP-8266, I recommend one with 4MB flash. 2MB should still suffice, but 1MB won't be enough.
  
- A frame to put your display in
  
  I just bought the most-fitting picture frame in Jysk. If you chose one already in a frame, you likely need to find some way to make it stand up.

- A micro-USB data cable and a USB charger

The total cost should be ~70-80$

# Installation instructions

**TODO**

# Art used

I'd like to thank all the artists that agreed to use their work. You can find the links to art sources in the folder `extras/slugcats_original`.

Great thanks to:
- [Pansear Doodles](https://pansear-doodles.tumblr.com/)
- [@Rouughnix](ttps://twitter.com/Rouughnix)
- [Valkyrie Gaming](https://www.youtube.com/@ignitedvalkyrie)

And of course, great thanks to VideoCult for making this amazing game. If you haven't played Rain World, and you like survival platformers with deep hidden lore - you should [give it a try](https://store.steampowered.com/app/312520/Rain_World/)!
