#!/usr/bin/python3
import signal
import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
import os
import sys

def signal_handler(signum, frame):
    print("CGI: Received signal, exiting", file=sys.stderr)
    sys.stderr.flush()
    os._exit(1)

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

print("CGI: Script started", file=sys.stderr)
sys.stderr.flush()

#====


print("CGI: Dumping selected environment variables", file=sys.stderr)
for key in ["REQUEST_METHOD", "CONTENT_LENGTH", "CONTENT_TYPE", "QUERY_STRING", "SCRIPT_NAME"]:
    print(f"  {key} = {os.environ.get(key, '<not set>')}", file=sys.stderr)
sys.stderr.flush()

# Get content info from environment
try:
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
except ValueError:
    content_length = 0
content_type = os.environ.get('CONTENT_TYPE', '')

print(f"CGI: Parsed Content-Length: {content_length}", file=sys.stderr)
print(f"CGI: Parsed Content-Type: {content_type}", file=sys.stderr)
sys.stderr.flush()

#=====
# Get content info from environment
content_length = int(os.environ.get('CONTENT_LENGTH', 0))
content_type = os.environ.get('CONTENT_TYPE', '')

print(f"CGI: Content-Length: {content_length}", file=sys.stderr)
print(f"CGI: Content-Type: {content_type}", file=sys.stderr)
sys.stderr.flush()

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_DIR = os.path.join(SCRIPT_DIR, "storage")
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Read ALL the data from stdin first
print("CGI: Reading all stdin data", file=sys.stderr)
sys.stderr.flush()

all_data = b''
bytes_read = 0
while bytes_read < content_length:
    chunk_size = min(8192, content_length - bytes_read)
    chunk = sys.stdin.buffer.read(chunk_size)
    if not chunk:
        print(f"CGI: Unexpected EOF, read {bytes_read} of {content_length}", file=sys.stderr)
        break
    all_data += chunk
    bytes_read += len(chunk)
    if bytes_read % (1024*1024) == 0:
        print(f"CGI: Read {bytes_read}/{content_length} bytes from stdin", file=sys.stderr)
        sys.stderr.flush()

print(f"CGI: Finished reading stdin: {len(all_data)} bytes", file=sys.stderr)
sys.stderr.flush()

print("Content-Type: text/plain\r\n\r\n")
sys.stdout.flush()

# Now parse the multipart data manually or use FieldStorage on the complete data
import io
import cgi

# Create a file-like object from our complete data
data_stream = io.BytesIO(all_data)

# Create environment for FieldStorage
environ = os.environ.copy()
environ['wsgi.input'] = data_stream

print("CGI: Creating FieldStorage from complete data", file=sys.stderr)
sys.stderr.flush()

# Parse with FieldStorage using our complete data
form = cgi.FieldStorage(fp=data_stream, environ=environ)

print("CGI: FieldStorage created from complete data", file=sys.stderr)
sys.stderr.flush()

# Rest of your original processing code...
if "file" not in form or not getattr(form["file"], "filename", None):
    print("CGI: No file found", file=sys.stderr)
    print("No file uploaded.")
    sys.exit(0)

file_item = form["file"]
filename = os.path.basename(file_item.filename)

print(f"CGI: Processing file {filename}", file=sys.stderr)
sys.stderr.flush()

name, ext = os.path.splitext(filename)
counter = 1
new_filename = filename
filepath = os.path.join(UPLOAD_DIR, new_filename)

while os.path.exists(filepath):
    new_filename = f"{name}({counter}){ext}"
    filepath = os.path.join(UPLOAD_DIR, new_filename)
    counter += 1

print(f"CGI: Writing to {filepath}", file=sys.stderr)
sys.stderr.flush()

bytes_written = 0
with open(filepath, "wb") as f:
    while True:
        chunk = file_item.file.read(8192)
        if not chunk:
            break
        f.write(chunk)
        bytes_written += len(chunk)

print(f"CGI: File write complete, {bytes_written} bytes total", file=sys.stderr)
sys.stderr.flush()

print(f"File '{new_filename}' uploaded successfully!")
print("CGI: Script ending", file=sys.stderr)
sys.stderr.flush()
