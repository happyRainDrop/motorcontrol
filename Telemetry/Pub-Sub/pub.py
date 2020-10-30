# Telemetry ZeroMQ Publisher
# Copyright (c) 2020 Applied Engineering

import concurrent.futures
import logging
import msgpack
import platform
import queue
import serial
import threading
import zmq

# Set logging verbosity.
# CRITICAL will not log anything.
# ERROR will only log exceptions.
# INFO will log more information.
log_level = logging.INFO

# ZeroMQ Context.
context = zmq.Context()
# Define the socket using the Context.
sock = context.socket(zmq.PUB)
sock.bind('epgm://224.0.0.1:28650')

# Define message end sequence.
end = b'EOM\n'

def readFromArduino(queue, exit_event):
    '''Read data from serial.'''
    
    while not exit_event.is_set():
        try:
            queue.put(link.read_until(end).rstrip(end))
            logging.info('Producer received data.')
        
        except Exception as e:
            logging.error('A %s error occurred.', e.__class__)
    
    logging.info('Producer received event. Exiting now.')
    link.close()

def sendZmqMulticast(queue, exit_event):
    '''Multicast data with ZeroMQ.'''
    while not exit_event.is_set() or not queue.empty():
        try:
            sock.send(queue.get())
            logging.info('Consumer sending data. Queue size is %d.', queue.qsize())

        except Exception as e:
            logging.error('A %s error occurred.', e.__class__)
    
    logging.info('Consumer received event. Exiting now.')
    sock.close()

if __name__ == '__main__':
    try:
        logging.basicConfig(format='[%(asctime)s] %(levelname)s: %(message)s', level=log_level, datefmt="%H:%M:%S")

        if platform.system() == 'Darwin':
            link = serial.Serial('/dev/tty.usbmodem14101', 500000)
        elif platform.system() == 'Linux':
            link = serial.Serial('/dev/ttyACM0', 500000)
        else:
            link = serial.Serial('COM9', 500000)
        
        # Throw away first reading
        _ = link.read_until(end).rstrip(end)

        pipeline = queue.Queue(maxsize=100)
        exit_event = threading.Event()
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
            executor.submit(readFromArduino, pipeline, exit_event)
            executor.submit(sendZmqMulticast, pipeline, exit_event)
    
    except KeyboardInterrupt:
        logging.info('Setting exit event.')
        exit_event.set()

    except Exception as e:
        logging.error('A %s error occurred.', e.__class__)
        exit_event.set()