#!/usr/bin/python3
import cgi
import os
import sys
import time
sys.stdout.flush()

UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "upload")

# Send proper headers
print("Content-Type: text/html")
print()

# Ensure upload directory exists
try:
    os.makedirs(UPLOAD_DIR, exist_ok=True)
except Exception as e:
    print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>❌ Upload Error - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
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
        
        <h1>❌ Upload Directory Error</h1>
        
        <div class="card">
            <h3>Failed to create upload directory</h3>
            <p class="error-message">Error: {e}</p>
            <div class="text-center">
                <a href="/upload/uploadFile.html" class="button">🔄 Try Again</a>
                <a href="/" class="button secondary">🏠 Home</a>
            </div>
        </div>
    </div>
</body>
</html>""")
    sys.exit(1)

form = cgi.FieldStorage()

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

print("""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>📁 File Upload Result - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .upload-success {
            background: linear-gradient(135deg, #27ae60, #2ecc71);
            color: white;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: center;
            animation: fadeInScale 0.6s ease-out;
        }
        
        .upload-error {
            background: linear-gradient(135deg, #e74c3c, #c0392b);
            color: white;
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: center;
            animation: shake 0.5s ease-in-out;
        }
        
        .file-info {
            background: #f8f9fa;
            border-left: 4px solid #3498db;
            padding: 15px;
            margin: 15px 0;
        }
        
        @keyframes fadeInScale {
            0% { opacity: 0; transform: scale(0.8); }
            100% { opacity: 1; transform: scale(1); }
        }
        
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-5px); }
            75% { transform: translateX(5px); }
        }
    </style>
</head>
<body>
    <div class="container fade-in">
        <!-- Navigation -->
        <nav class="nav">
            <div class="logo">🚀 Prophet Server</div>
            <ul class="nav-links">
                <li><a href="/">Home</a></li>
                <li><a href="/about.html">About</a></li>
                <li><a href="/cgi-bin/login.py">Login</a></li>
                <li><a href="/upload/uploadFile.html">Upload</a></li>
            </ul>
        </nav>

        <h1>📁 File Upload Result</h1>""")

if "file" not in form:
    print("""
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

fileitem = form["file"]

if fileitem.filename:
    filename = os.path.basename(fileitem.filename)
    filepath = os.path.join(UPLOAD_DIR, filename)
    upload_time = time.strftime("%Y-%m-%d %H:%M:%S")

    try:
        # Stream the file in chunks instead of reading all at once
        with open(filepath, "wb") as f:
            while True:
                chunk = fileitem.file.read(8192)  # 8KB chunks
                if not chunk:
                    break
                f.write(chunk)
        
        # Get file size after writing
        file_size = os.path.getsize(filepath)

        print(f"""
        <div class="upload-success">
            <h3>✅ File Uploaded Successfully!</h3>
            <p>Your file has been saved to the server.</p>
        </div>
        
        <div class="card">
            <h3>📋 Upload Details</h3>
            <div class="file-info">
                <p><strong>📁 Filename:</strong> {filename}</p>
                <p><strong>📏 File Size:</strong> {format_file_size(file_size)}</p>
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
                    <li>Files are accessible via direct URL</li>
                </ul>
            </div>
            
            <div class="card">
                <h3>🔧 Server Info</h3>
                <p><strong>Upload Directory:</strong> {UPLOAD_DIR}</p>
                <p><strong>Server Status:</strong> <span class="status running">✅ Online</span></p>
                <p><strong>CGI Processing:</strong> <span class="highlight">Active</span></p>
            </div>
        </div>
        
        <div class="text-center mt-3">
            <a href="/upload/uploadFile.html" class="button">📤 Upload Another</a>
            <a href="/" class="button secondary">🏠 Home</a>
            <a href="/cgi-bin/hello_process.py" class="button secondary">👋 Hello Demo</a>
        </div>
    </div>
</body>
</html>""")
        
    except Exception as e:
        print(f"""
        <div class="upload-error">
            <h3>❌ Upload Failed</h3>
            <p>Failed to save file to server.</p>
        </div>
        
        <div class="card">
            <h3>🔍 Error Details</h3>
            <div class="file-info">
                <p><strong>Error Message:</strong> {e}</p>
                <p><strong>Attempted Filename:</strong> {filename}</p>
                <p><strong>Attempted Path:</strong> {filepath}</p>
            </div>
            
            <h3>💡 Troubleshooting</h3>
            <ul class="feature-list">
                <li>Check if upload directory has write permissions</li>
                <li>Verify file size is within server limits</li>
                <li>Ensure filename contains valid characters</li>
                <li>Try uploading a different file</li>
            </ul>
            
            <div class="text-center mt-3">
                <a href="/upload/uploadFile.html" class="button">🔄 Try Again</a>
                <a href="/" class="button secondary">🏠 Home</a>
            </div>
        </div>
    </div>
</body>
</html>""")
else:
    print("""
        <div class="upload-error">
            <h3>❌ No File Selected</h3>
            <p>The form was submitted without selecting a file.</p>
        </div>
        
        <div class="card">
            <h3>📋 How to Upload Files</h3>
            <ul class="feature-list">
                <li><strong>Step 1:</strong> Click "Choose File" button</li>
                <li><strong>Step 2:</strong> Select a file from your computer</li>
                <li><strong>Step 3:</strong> Click "Upload" to submit</li>
                <li><strong>Step 4:</strong> Wait for confirmation message</li>
            </ul>
            
            <div class="text-center mt-3">
                <a href="/upload/uploadFile.html" class="button">🔄 Try Again</a>
                <a href="/" class="button secondary">🏠 Home</a>
            </div>
        </div>
    </div>
</body>
</html>""")
