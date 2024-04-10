# Student ID : 20223060
# Name : Eunji Kim

import socket
import os

print("Student ID: 20223060")
print("Name: Eunji Kim")

while True:

    mysock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    input_cmd = input('> ')

    # end
    if(input_cmd == 'quit'):
        mysock.close()
        break
    
    # input 에러
    if(len(input_cmd.split()) != 2):
        print("Wrong command")
        continue

    url = input_cmd.split()[1]
    server = url.split('/')
    protocol = server[0].split(':')[0]


    if(protocol != "http"):
        print("Only support http, not {}\n" .format(protocol))
        continue

    host = server[2].split(':')[0]

    try:
        mysock.connect( (host, 80) )
        req_header = "GET /"
        for i in range(3, len(server)):
            req_header += server[i]                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
            if (i != len(server)-1): req_header += '/'

        req_header += " HTTP/1.0\r\nHost: {}\r\nUser-agent: HW1/1.0\r\nConnection: close\r\n\r\n".format(host)
        print(req_header)

        mysock.send(req_header.encode())

        response = mysock.recv(1024).split(b'\r\n\r\n')
        res_header = response[0].split(b'\r\n')


        status = res_header[0].decode().split()

        if status[1] != '200':
            print(" ".join(status[1:]))
            continue

        idx = response[0].decode().split().index('Content-Length:')
        total_size = response[0].decode().split()[idx+1]
        print("Total Size {} bytes" .format(total_size))

        file = open(server[-1], 'wb')
        file.write(response[1])

        percent = 10

        while True:
            data = mysock.recv(1024)
            if not data:
                file.close()
                break

            file.write(data)

            current = os.path.getsize(server[-1])
            download = int(current / int(total_size)*100)

            if (download >= percent):
                print("Current Downloading {}/{}(bytes) {}%" .format (current, total_size, download))
                percent += 10

        
        current = os.path.getsize(server[-1])
        download = int(current / int(total_size)*100)
        if(download != 100): print("Current Downloading {}/{}(bytes) {}%" .format (current, total_size, download))
        print("Current Downloading {}/{}(bytes) {}%" .format(total_size, total_size, 100))
        print("Download Complete: {}, {}/{}" .format(server[-1], total_size, total_size))

    except:
        print("{}: unknown host\ncannot connect to server {}\n" .format(host ,host))
        continue

    mysock.close()