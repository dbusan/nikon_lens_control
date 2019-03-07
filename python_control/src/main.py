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

    lens.set_aperture('F1_4')
    # wait for the lens to finish
    time.sleep(0.5)

    # drive the focus 1000 steps towards infinity
    lens.drive_focus(1000)

    # wait for the lens to finish
    time.sleep(0.7)
    lens.drive_focus(-1000)

    # close connection
    time.sleep(1)
    lens.close_connection()


if __name__ == '__main__':
    main()
