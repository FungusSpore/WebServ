#!/usr/bin/env python3
import cgi
import os
import sys

UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "upload")

# Send proper headers
print("Content-Type: text/html\r\n\r\n", end="")

# Ensure upload directory exists
try:
    os.makedirs(UPLOAD_DIR, exist_ok=True)
except Exception as e:
    print(f"<h2>Failed to create upload dir: {e}</h2>")
    sys.exit(1)

form = cgi.FieldStorage()

if "file" not in form:
    print("<h2>No file uploaded</h2>")
    sys.exit(0)

fileitem = form["file"]

if fileitem.filename:
    filename = os.path.basename(fileitem.filename)
    filepath = os.path.join(UPLOAD_DIR, filename)

    try:
        with open(filepath, "wb") as f:
            while True:
                chunk = fileitem.file.read(8192)
                if not chunk:
                    break
                f.write(chunk)

        print(f"<h2>File '{filename}' uploaded successfully!</h2>")
        print(f"<p>Saved to: {filepath}</p>")
    except Exception as e:
        print(f"<h2>Failed to save file: {e}</h2>")
else:
    print("<h2>No file selected</h2>")
