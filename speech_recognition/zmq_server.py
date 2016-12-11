import zmq
import time
import sys
import pyaudio
import wave
import re
import requests
import transcribe_streaming
import json

from geopy.geocoders import Nominatim


def play_audio_file(filename):
    # define stream chunk
    chunk = 1024

    # open a wav format music
    f = wave.open(filename, 'rb')

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


def handle_emergency_detection():

    play_audio_file(r"/Users/hassenennifar/Documents/Gitlab/helpMe/emergency.wav")
    transcript = transcribe_streaming.main()
    # print(transcript)

    if re.search(r'\b(no|none|nope|help|ambulance|hurt|I\'m not|I am not|no response)\b', transcript, re.I):
        print('Calling for help..')
        play_audio_file(r"/Users/hassenennifar/Documents/Gitlab/helpMe/calling.wav")
        send_url = 'http://freegeoip.net/json'
        r = requests.get(send_url)
        j = json.loads(r.text)
        lat = j['latitude']
        lon = j['longitude']
        print(lat)
        print(lon)
        geolocator = Nominatim()
        mylocation = geolocator.reverse(str(lat) + ", " + str(lon))
        print(mylocation.address)

    elif re.search(r'\b(yes|yeah|good|fine|ok)\b', transcript, re.I):
        print('Happy to hear you are doing well, no actions required')
        play_audio_file(r"/Users/hassenennifar/Documents/Gitlab/helpMe/doingwell.wav")

    else:
        print('I am sorry, I did not understand what you said')


def main_loop_server():
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

        print "Received request: ", message
        time.sleep(1)
        socket.send("World from %s" % port)

        if message == 'Emergency Detected!':
            handle_emergency_detection()


def main():
    main_loop_server()
    # handle_emergency_detection()

if __name__ == '__main__':
        main()
