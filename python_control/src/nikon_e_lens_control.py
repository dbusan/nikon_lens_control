# project name: nikon e lens control
# author:       Daniel C Busan
# date:         2019/03/06

import helper
import serial
import time

aperture_values = {
    'F1_4': 0,
    'F1_6': 1,
    'F1_8': 2,
    'F2_0': 3,
    'F2': 3,
    'F2_2': 4,
    'F2_5': 5,
    'F2_8': 6,
    'F3_2': 7,
    'F3_5': 8,
    'F4_0': 9,
    'F4': 9,
    'F4_5': 10,
    'F5_0': 11,
    'F5': 11,
    'F5_6': 12,
    'F6_3': 13,
    'F7_1': 14,
    'F8_0': 15,
    'F8': 15,
    'F9_0': 16,
    'F9': 16,
    'F10_0': 17,
    'F10': 17,
    'F11_0': 18,
    'F11': 18,
    'F13_0': 19,
    'F13': 19,
    'F14_0': 20,
    'F14': 20,
    'F16_0': 21,
    'F16': 21
}


class Lens:
    def __init__(self, port='/dev/nikonlens'):

        # serial_ports = helper.serial_ports()
        #
        # # list all connected ACM ports
        # self.port = get_lens_port([port for port in serial_ports if 'nikonlens' in port])

        # uncomment previous lines if you want the program to automatically detect serial ports.
        # otherwise it will use /dev/nikonlens

        self.port = port

        try:
            self.serial_connection = serial.Serial(self.port)
            # prepare payload to initialise lens
            payload_data = 'IF 0\n'.encode('utf-8')

            # send payload
            self.serial_connection.write(payload_data)
            time.sleep(0.1)

        except (OSError, serial.SerialException) as e:
            # print(e)
            raise

    def set_aperture(self, aperture):
        if aperture not in aperture_values.keys():
            from pprint import pprint
            print('Invalid aperture value specified:', aperture)
            print('Valid values are')
            pprint(list(aperture_values.keys()))
            exit(1)

        if self.serial_connection.is_open:
            # prepare payload to change aperture
            # AP 0-21 changes the aperture of the lens to the nth step
            # specified
            payload_data = 'AP {}\n'.format(aperture_values[aperture])
            payload_data = payload_data.encode('utf-8')

            # write data to serial
            self.serial_connection.write(payload_data)
            #
            print(self.serial_connection.read_all().decode('utf-8'))

        else:
            print('Serial connection must be open first.')
            exit(1)

    def drive_focus(self, steps):
        if self.serial_connection.is_open:
            # FC n_steps - positive value moves focus one way, negative moves it the other way
            payload_data = 'FC {}\n'.format(steps).encode('utf-8')

            # send payload
            self.serial_connection.write(payload_data)

            print(self.serial_connection.read_all().decode('utf-8'))

        else:
            print('Serial connection must be open before driving focus.')

    # close connection to lens
    def close_connection(self):
        if self.serial_connection.is_open:
            self.serial_connection.close()

    def __del__(self):
        if self.serial_connection.is_open:
            print('Closing Serial Connection')
            self.serial_connection.close()


def get_lens_port(micro_list):
    # for micro in micro_list:
    #     # connect to micro and check if it's the right one
    #     # check ID
    #     try:
    #         connection = serial.Serial(micro)
    #
    #         connection.close()
    #     except (OSError, serial.SerialException):
    #         pass

    # TODO implement ID for the micro-controller to verify we are connected to the right one
    if len(micro_list) > 0:
        return micro_list[0]
    else:
        return None

