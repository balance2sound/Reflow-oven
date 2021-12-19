# DIY-reflow-oven
## Reflow oven for soldering SMD and BGA's
### Buy list
- Oven. We've bought ours for 30 euros. Link: [https://www.amazon.es/Orbegozo-HO-980-temporizador-temperatura/dp/B08HKLRBBD](url) (if it have a fan the performance could be better)
- AC SSR's x2 with heatsink
- Arduino Mega or Due x1 **Note:** If using Arduino Due don't exceed I/O pins voltage: 3.3V!
- Screen HX8357 480X320 x1 **Note:** We use the Boodmer's HX8357/ILI4986 graphic library. GUI designers like GuiSlice does not support this screen from native.
- Heat resistant Mains cable.
- Rotary encoder with built-in button x1.
- Switch x1 .
- Max6675 Thermopar controller x 3 (1 for top, 1 for botton and 1 for the pcb).
- Thermopar type K x3.
- Usb or SD card module to save records (Optional).
- 1 85x85 fan (pull air) and other smaller fans to pull air in to the enclosure.
- Heat insulator (like ceramic fiber).
# Tutorial 
- Dissasemble the oven, saving the timer and the thermostat. We could use these parts in a future or as a replacement for others oven.
- Take measurements of the oven. Find the place where the components could be cool (control board, SSR's and the power supply), even if we use fans to keep them cool **>85ºC** may damage the components 
