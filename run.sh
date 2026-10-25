#!/bin/sh
cd "$(dirname "$0")" || exit 1
exec python3 -u -c 'import http.server, webbrowser; server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), http.server.SimpleHTTPRequestHandler); url = "http://127.0.0.1:%d/" % server.server_address[1]; print("Open " + url, flush=True); webbrowser.open(url); server.serve_forever()'
