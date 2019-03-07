# Nikon Lens Control Repository
---
Author: Daniel Busan

## Description
This repository contains two sets of source code: one for the Arduino connected to the Nikon Lens - ```micro_firmware```, the other for the Python module and example code to control it - ```python_control```.

---

## Usage
```python_control``` contains the nikon_e_lens_control library. This library opens a serial connection to a connected Arduino that is flashed with ```micro_firmware``` and has the ```NikonLens``` library available (either in the ```~/Arduino/libraries``` or locally). 

Instantiate a ```Lens``` object that also initialises the serial connection to ```/dev/nikonlens``` by default. This can be overriden if the port parameter is specified (eg. ```/dev/ttyACM0```).


### Aperture Control
To drive the aperture of the specify the desired f number as the parameter of the ```lens.set_aperture(F_NUMBER)``` method. 

For example: ```lens.set_aperture('F1_4')``` sets the aperture to fully open, while ```lens.set_aperture('F16')``` sets the aperture to the smallest opening.

### Focus Control
To drive the focus ring of the lens, the ```lens.drive_focus(n_steps)``` method is called, where n_steps is an integer ranging from -12000 to 12000. The lens drives the focus ring in the specified direction until the limit is reached. Any further focus step commands in that direction will not drive the focus ring any further. Total step count is around 11000. 

Negative values drive the focus ring towards minimum, positive values drive the focus to infinity (and beyond). **The minimum** ```n_steps``` value required to move the focus ring is **10**.

For Example: ```lens.drive_focus(1500)``` drives the focus ring 1500 steps towards infinity. 


## Full source code example

```
import nikon_e_lens_control as nikon
import time
def main():
    # connect to micro controller attached to lens

    # instantiate lens object
    # by default looks for /dev/nikonlens
    # however you can specify the port parameter to the desired device
    # nikon.Lens(port='/dev/ttyACM0') for example.
    lens = nikon.Lens()

    # set aperture
    lens.set_aperture('F16_0')
    # wait for the lens to finish
    time.sleep(1)

    # drive focus ring
    lens.drive_focus(1000)

    # remember to close the connection
    lens.close_connection()
```