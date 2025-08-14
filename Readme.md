# Key Reader
<p align="center">
  <img width="341" height="368" src="Key Machine w Reader sm.png">
</p>

This project is a [Key Code Cutter](https://www.instructables.com/Key-Code-Cutter/) accessory.  The Key Reader uses a camera to determine a key's code and sends the code to the Key Code Cutter to be cut.

The software is designed to run on my "STM32F429 3.5TFT & Camera v1.0" board.
<p align="center">
  <img width="714" height="535" src="Main Board.png">
</p>


## SdFat
<pThis project uses an unmodified version of Bill Greiman's SdFat library. Copyright Bill Greiman 2011-2024 (currently using version 2.2.3)  This can be loaded using the Arduino IDE's library manager.</p>
<br>
<br>

<p align="center">
  <img width="731" height="641" src="Preview.png">
</p>
<p align="center">
  <img width="480" height="321" src="Scan.png">
</p>

**Actions:**
- Preview streams either color or black and white video so that the user can verify that the key is aligned properly.
- Scan takes a high res image, then analyzes the image to determine the key code.
- Cut, enabled after a valid scan, sends the key code information to the Key Code Cutter to be cut.

See my 
[Key Reader](https://www.instructables.com/Key-Reader/) instructable for more information.

The instructable contains links to where the bare PCB can be purchased.

