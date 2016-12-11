import zmq
import time
import sys
import pyaudio
import wave

port = "5556"
if len(sys.argv) > 1:
    port =  sys.argv[1]
    int(port)
context = zmq.Context()
socket = context.socket(zmq.REP)
socket.bind("tcp://*:%s" % port)

while True:
    #  Wait for next request from client
    message = socket.recv()

    if strcmp(message, 'Emergency Detected!'):

        # define stream chunk
        chunk = 1024

        # open a wav format music
        f = wave.open(r"/Users/hassenennifar/Documents/Gitlab/helpMe/emergency.wav", 'rb')

        # instantiate PyAudio
        p = pyaudio.PyAudio()

        # open stream
        stream = p.open(format=p.get_format_from_width(f.getsampwidth()),
                        channels=f.getnchannels(),
                        rate=f.getframerate(),
                        output=True)
        # read data
        data = f.readframes(chunk)

        # paly stream
        while data != '':
            stream.write(data)
            data = f.readframes(chunk)

        # stop stream
        stream.stop_stream()
        stream.close()

        # close PyAudio
        p.terminate()



    print "Received request: ", message
    time.sleep (1)  
    socket.send("World from %s" % port)
