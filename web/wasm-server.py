# wasm-server.py
import http.server
import socketserver

class WasmHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):        
        # Include additional response headers here. CORS for example:
        #self.send_header('Access-Control-Allow-Origin', '*')
        http.server.SimpleHTTPRequestHandler.end_headers(self)


# Python 3.7.5 adds in the WebAssembly Media Type. Version 2.x doesn't
# have this so add it in.
WasmHandler.extensions_map['.wasm'] = 'application/wasm'


if __name__ == '__main__':
    HOST, PORT = "", 8080
    with socketserver.TCPServer((HOST, PORT), WasmHandler) as server:
        print("Listening on port {}. Press Ctrl+C to stop.".format(PORT))
        server.serve_forever()