#!/usr/bin/python3
import signal
import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
import os
import sys
import tempfile
import io
import cgi
import time

def signal_handler(signum, frame):
    print("CGI: Received signal, exiting", file=sys.stderr)
    sys.stderr.flush()
    os._exit(1)

signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

def log(msg):
    print(msg, file=sys.stderr)
    sys.stderr.flush()

log("CGI: Script started")
for key in ["REQUEST_METHOD", "CONTENT_LENGTH", "CONTENT_TYPE", "QUERY_STRING", "SCRIPT_NAME"]:
    log(f"  {key} = {os.environ.get(key, '<not set>')}")

# # SEND HEADERS IMMEDIATELY - don't wait for data processing
# sys.stdout.write("Content-Type: text/html\r\n")
# sys.stdout.write("\r\n")  # Empty line to end headers
# sys.stdout.flush()  # Force headers to be sent immediately

# Start HTML page immediately
# print("""<!DOCTYPE html>
# <html lang="en">
# <head>
#     <meta charset="UTF-8">
#     <meta name="viewport" content="width=device-width, initial-scale=1.0">
#     <title>📁 File Upload Processing - Prophet Web Server</title>
#     <link rel="stylesheet" href="/css/style.css">
#     <style>
#         .upload-success {
#             background: linear-gradient(135deg, #27ae60, #2ecc71);
#             color: white;
#             padding: 20px;
#             border-radius: 10px;
#             margin: 20px 0;
#             text-align: center;
#             animation: fadeInScale 0.6s ease-out;
#         }
        
#         .upload-error {
#             background: linear-gradient(135deg, #e74c3c, #c0392b);
#             color: white;
#             padding: 20px;
#             border-radius: 10px;
#             margin: 20px 0;
#             text-align: center;
#             animation: shake 0.5s ease-in-out;
#         }
        
#         .file-info {
#             background: #f8f9fa;
#             border-left: 4px solid #3498db;
#             padding: 15px;
#             margin: 15px 0;
#         }
        
#         @keyframes fadeInScale {
#             0% { opacity: 0; transform: scale(0.8); }
#             100% { opacity: 1; transform: scale(1); }
#         }
        
#         @keyframes shake {
#             0%, 100% { transform: translateX(0); }
#             25% { transform: translateX(-5px); }
#             75% { transform: translateX(5px); }
#         }
#     </style>
# </head>
# <body>
#     <div class="container fade-in">
#         <nav class="nav">
#             <div class="logo">🚀 Prophet Server</div>
#             <ul class="nav-links">
#                 <li><a href="/">Home</a></li>
#                 <li><a href="/about.html">About</a></li>
#                 <li><a href="/cgi-bin/login.py">Login</a></li>
#                 <li><a href="/upload/uploadFile.html">Upload</a></li>
#             </ul>
#         </nav>
        
#         <h1>📁 File Upload Processing</h1>""")
# sys.stdout.flush()

try:
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
except ValueError:
    content_length = 0
content_type = os.environ.get('CONTENT_TYPE', '')

print("Content-Length: {}", content_length, file=sys.stderr)

log(f"CGI: Content-Length: {content_length}")
log(f"CGI: Content-Type: {content_type}")

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
UPLOAD_DIR = os.path.join(SCRIPT_DIR, "storage")
TMP_DIR = os.path.join(SCRIPT_DIR, "tmp")
os.makedirs(UPLOAD_DIR, exist_ok=True)
os.makedirs(TMP_DIR, exist_ok=True)

# Stream stdin directly to a temp file
log("CGI: Streaming stdin to temp file")
temp_path = None
try:
    # Use local tmp directory instead of system tmp
    import uuid
    temp_filename = f"upload_{uuid.uuid4().hex}.tmp"
    temp_path = os.path.join(TMP_DIR, temp_filename)
    
    with open(temp_path, "wb") as temp:
        bytes_read = 0
        while bytes_read < content_length:

            chunk_size = min(8388608, content_length - bytes_read)
            # log(f"CGI: Attempting toread {chunk_size} bytes (read {bytes_read}/{content_length})")
            
            try:
                # Check if data is available before reading
                import select
                ready, _, _ = select.select([sys.stdin], [], [], 1.0)  # 1 second timeout
                if not ready:
                    log("CGI: No data available on stdin after 1 second")
                    break
                    
                chunk = sys.stdin.buffer.read(chunk_size)
                # log(f"CGI: Actually read {len(chunk) if chunk else 0} bytes")
            except Exception as e:
                log(f"CGI: Error reading from stdin: {e}")
                break
                
            if not chunk:
                log(f"CGI: Unexpected EOF, read {bytes_read} of {content_length}")
                break
            temp.write(chunk)
            bytes_read += len(chunk)
            
            # Log progress for large files
            if bytes_read % (1024*1024) == 0:  # Every MB
                log(f"CGI: Progress: {bytes_read}/{content_length} bytes")
                
    log(f"CGI: Finished reading stdin: {bytes_read} bytes to {temp_path}")

    # Parse multipart from temp file
    with open(temp_path, "rb") as f:
        data_stream = io.BytesIO(f.read())

except Exception as e:
    log(f"CGI: Error during stdin processing: {e}")
    if temp_path and os.path.exists(temp_path):
        os.unlink(temp_path)
    # Headers already sent, send error HTML
    print(f"""
        <div class="upload-error">
            <h3>❌ Upload Failed</h3>
            <p>Failed to process upload data.</p>
        </div>
        
        <div class="card">
            <h3>🔍 Error Details</h3>
            <div class="file-info">
                <p><strong>Error Message:</strong> {e}</p>
            </div>
            
            <div class="text-center mt-3">
                <a href="/upload/uploadFile.html" class="button">🔄 Try Again</a>
                <a href="/" class="button secondary">🏠 Home</a>
            </div>
        </div>
    </div>
</body>
</html>""")
    sys.exit(1)

form = cgi.FieldStorage(fp=data_stream, environ=os.environ.copy())
if "file" not in form or not getattr(form["file"], "filename", None):
    log("CGI: No file found")
    # Headers already sent, send no file HTML
    print(f"""
        <div class="upload-error">
            <h3>❌ No File Uploaded</h3>
            <p>Please select a file before submitting the form.</p>
        </div>
        
        <div class="card">
            <h3>💡 Upload Instructions</h3>
            <ul class="feature-list">
                <li>Click "Choose File" to select a file from your computer</li>
                <li>Make sure a file is selected before clicking "Upload"</li>
                <li>Supported file types: images, documents, text files</li>
                <li>Maximum file size: Check server configuration</li>
            </ul>
            
            <div class="text-center mt-3">
                <a href="/upload/uploadFile.html" class="button">🔄 Try Again</a>
                <a href="/" class="button secondary">🏠 Home</a>
            </div>
        </div>
    </div>
</body>
</html>""")
    sys.exit(0)

file_item = form["file"]
filename = os.path.basename(file_item.filename)
name, ext = os.path.splitext(filename)
counter = 1
new_filename = filename
filepath = os.path.join(UPLOAD_DIR, new_filename)
while os.path.exists(filepath):
    new_filename = f"{name}({counter}){ext}"
    filepath = os.path.join(UPLOAD_DIR, new_filename)
    counter += 1

log(f"CGI: Moving temp file to {filepath}")
os.rename(temp_path, filepath)
bytes_written = os.path.getsize(filepath)
log(f"CGI: File write complete, {bytes_written} bytes total")

# Helper function to format file size
def format_file_size(size_bytes):
    if size_bytes == 0:
        return "0 B"
    size_names = ["B", "KB", "MB", "GB"]
    i = 0
    while size_bytes >= 1024 and i < len(size_names) - 1:
        size_bytes /= 1024.0
        i += 1
    return f"{size_bytes:.1f} {size_names[i]}"

upload_time = time.strftime("%Y-%m-%d %H:%M:%S")

# Before sending the success HTML, send headers again
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")  # Empty line to end headers

# Send complete HTML document with proper structure
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>📁 Upload Success - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .upload-success {{
            background: linear-gradient(135deg, #27ae60, #2ecc71);
            color: white;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: center;
            animation: fadeInScale 0.6s ease-out;
        }}
        
        .file-info {{
            background: #f8f9fa;
            border-left: 4px solid #3498db;
            padding: 15px;
            margin: 15px 0;
        }}
        
        @keyframes fadeInScale {{
            0% {{ opacity: 0; transform: scale(0.8); }}
            100% {{ opacity: 1; transform: scale(1); }}
        }}
    </style>
</head>
<body>
    <div class="container fade-in">
        <nav class="nav">
            <div class="logo">🚀 Prophet Server</div>
            <ul class="nav-links">
                <li><a href="/">Home</a></li>
                <li><a href="/about.html">About</a></li>
                <li><a href="/cgi-bin/login.py">Login</a></li>
                <li><a href="/upload/uploadFile.html">Upload</a></li>
            </ul>
        </nav>
        
        <h1>📁 File Upload Success</h1>

        <div class="upload-success">
            <h3>✅ File Uploaded Successfully!</h3>
            <p>Your file has been saved to the server.</p>
        </div>
        
        <div class="card">
            <h3>📋 Upload Details</h3>
            <div class="file-info">
                <p><strong>📁 Original Filename:</strong> {filename}</p>
                <p><strong>💾 Saved As:</strong> {new_filename}</p>
                <p><strong>📏 File Size:</strong> {format_file_size(bytes_written)}</p>
                <p><strong>⏰ Upload Time:</strong> {upload_time}</p>
                <p><strong>💾 Saved To:</strong> {filepath}</p>
            </div>
        </div>
        
        <div class="card-grid">
            <div class="card">
                <h3>🎯 What's Next?</h3>
                <ul class="feature-list">
                    <li>Your file is now stored on the server</li>
                    <li>Upload another file if needed</li>
                    <li>Check the upload directory for your files</li>
                </ul>
            </div>
        </div>
        
        <div class="text-center mt-3">
            <a href="/cgi-bin/browse_uploads.py" class="button">📂 Browse Files</a>
            <a href="/upload/uploadFile.html" class="button">📤 Upload Another</a>
            <a href="/" class="button secondary">🏠 Home</a>
        </div>
    </div>
</body>
</html>""")

log("CGI: Script ending")

# Cleanup temp file
if temp_path and os.path.exists(temp_path):
    try:
        os.unlink(temp_path)
        log("CGI: Temp file cleaned up")
    except:
        pass
