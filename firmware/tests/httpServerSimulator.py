from http.server import BaseHTTPRequestHandler, HTTPServer
import time

hostName = '0.0.0.0'
serverPort = 80

def read_file(filename:str):
    with open(filename) as f:
        contents = f.read()
        # print(contents)
    return contents

class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):
        print(self.request)
        content = read_file('tests/filetest.txt')
        self.send_response(200)
        self.send_header("Content-type", "text/plain")
        self.end_headers()
        self.wfile.write(bytes(content.encode('utf-8')))

if __name__ == "__main__":        
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server stopped.")