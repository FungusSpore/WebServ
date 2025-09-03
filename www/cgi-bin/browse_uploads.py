#!/usr/bin/env python3
import os
import sys
import time
import html
from urllib.parse import parse_qs

# Set the upload directory relative to this script
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "storage")

# Handle POST requests (delete operations)
if os.environ.get('REQUEST_METHOD') == 'POST':
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        if content_length > 0:
            post_data = sys.stdin.read(content_length)
            params = parse_qs(post_data)
            
            if 'action' in params:
                action = params['action'][0]
                
                if action == 'delete' and 'filename' in params:
                    filename = params['filename'][0]
                    file_path = os.path.join(UPLOAD_DIR, filename)
                    if os.path.exists(file_path) and os.path.isfile(file_path):
                        os.remove(file_path)
                        print("Content-Type: text/html")
                        print()
                        print(f"<script>alert('File {html.escape(filename)} deleted successfully!'); window.location.href='/cgi-bin/browse_uploads.py';</script>")
                        sys.exit(0)
                
                elif action == 'delete_all':
                    deleted_count = 0
                    for filename in os.listdir(UPLOAD_DIR):
                        file_path = os.path.join(UPLOAD_DIR, filename)
                        if os.path.isfile(file_path):
                            os.remove(file_path)
                            deleted_count += 1
                    print("Content-Type: text/html")
                    print()
                    print(f"<script>alert('{deleted_count} files deleted successfully!'); window.location.href='/cgi-bin/browse_uploads.py';</script>")
                    sys.exit(0)
    except Exception as e:
        print("Content-Type: text/html")
        print()
        print(f"<script>alert('Error: {e}'); window.location.href='/cgi-bin/browse_uploads.py';</script>")
        sys.exit(0)

# Send proper headers
print("Content-Type: text/html")
print()

def format_file_size(size_bytes):
    """Convert bytes to human readable format"""
    if size_bytes == 0:
        return "0 B"
    size_names = ["B", "KB", "MB", "GB"]
    i = 0
    while size_bytes >= 1024 and i < len(size_names) - 1:
        size_bytes /= 1024.0
        i += 1
    return f"{size_bytes:.1f} {size_names[i]}"

def get_file_info(filepath):
    """Get file information"""
    try:
        stat = os.stat(filepath)
        return {
            'size': stat.st_size,
            'modified': time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(stat.st_mtime)),
            'readable_size': format_file_size(stat.st_size)
        }
    except:
        return {
            'size': 0,
            'modified': 'Unknown',
            'readable_size': '0 B'
        }

# Get list of files in upload directory
try:
    if os.path.exists(UPLOAD_DIR) and os.path.isdir(UPLOAD_DIR):
        files = []
        for filename in os.listdir(UPLOAD_DIR):
            filepath = os.path.join(UPLOAD_DIR, filename)
            if os.path.isfile(filepath):
                file_info = get_file_info(filepath)
                files.append({
                    'name': filename,
                    'path': filepath,
                    'info': file_info
                })
        # Sort files by name
        files.sort(key=lambda x: x['name'].lower())
    else:
        files = []
        upload_dir_exists = False
except Exception as e:
    files = []
    error_message = str(e)

# Generate HTML
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>📁 Browse Uploads - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .file-list {{
            background: var(--card-bg, #f8f9fa);
            border-radius: 8px;
            padding: 1rem;
            margin: 1rem 0;
        }}
        .file-item {{
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0.75rem;
            border-bottom: 1px solid var(--border-color, #e9ecef);
            transition: background-color 0.2s ease;
        }}
        .file-item:last-child {{
            border-bottom: none;
        }}
        .file-item:hover {{
            background-color: var(--hover-bg, #f1f3f5);
        }}
        .file-info {{
            display: flex;
            align-items: center;
            gap: 1rem;
            flex: 1;
        }}
        .file-icon {{
            font-size: 1.5rem;
            width: 2rem;
            text-align: center;
        }}
        .file-details {{
            flex: 1;
        }}
        .file-name {{
            font-weight: 500;
            color: var(--text-primary, #333);
            margin-bottom: 0.25rem;
        }}
        .file-meta {{
            font-size: 0.875rem;
            color: var(--text-secondary, #666);
        }}
        .file-actions {{
            display: flex;
            gap: 0.5rem;
        }}
        .file-actions a {{
            text-decoration: none;
            padding: 0.25rem 0.5rem;
            border-radius: 4px;
            font-size: 0.875rem;
            transition: all 0.2s ease;
        }}
        .delete-btn {{
            background-color: #dc3545;
            color: white;
            border: none;
            padding: 0.25rem 0.5rem;
            border-radius: 4px;
            font-size: 0.875rem;
            cursor: pointer;
            transition: all 0.2s ease;
        }}
        .delete-btn:hover {{
            background-color: #c82333;
        }}
        .delete-all-btn {{
            background-color: #6c757d;
            color: white;
            border: none;
            padding: 0.5rem 1rem;
            border-radius: 4px;
            font-size: 0.875rem;
            cursor: pointer;
            transition: all 0.2s ease;
            margin-bottom: 1rem;
        }}
        .delete-all-btn:hover {{
            background-color: #5a6268;
        }}
        .empty-state {{
            text-align: center;
            padding: 3rem 1rem;
            color: var(--text-secondary, #666);
        }}
        .empty-state .icon {{
            font-size: 4rem;
            margin-bottom: 1rem;
            opacity: 0.5;
        }}
        .stats {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1rem;
            padding: 0.75rem;
            background: var(--info-bg, #e3f2fd);
            border-radius: 6px;
            font-size: 0.875rem;
        }}
        .breadcrumb {{
            margin-bottom: 1rem;
            font-size: 0.875rem;
            color: var(--text-secondary, #666);
        }}
        .breadcrumb a {{
            color: var(--primary-color, #007bff);
            text-decoration: none;
        }}
        .breadcrumb a:hover {{
            text-decoration: underline;
        }}
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

        <!-- Main Content -->
        <h1>📁 Browse Uploaded Files</h1>
        
        <div class="breadcrumb">
            <a href="/">🏠 Home</a> / <a href="/upload/uploadFile.html">📤 Upload</a> / 📁 Browse
        </div>
""")

if files:
    total_files = len(files)
    total_size = sum(f['info']['size'] for f in files)
    total_size_readable = format_file_size(total_size)
    
    print(f"""
        <div class="stats">
            <span>📊 <strong>{total_files}</strong> file{'s' if total_files != 1 else ''} found</span>
            <span>💾 Total size: <strong>{total_size_readable}</strong></span>
        </div>
        
        <!-- Delete All Button -->
        <form method="post" onsubmit="return confirm('Are you sure you want to delete ALL files? This action cannot be undone!');" style="margin-bottom: 1rem;">
            <input type="hidden" name="action" value="delete_all">
            <button type="submit" class="delete-all-btn">🗑️ Delete All Files</button>
        </form>
        
        <div class="file-list">
    """)
    
    for file in files:
        file_name = html.escape(file['name'])
        file_ext = os.path.splitext(file['name'])[1].lower()
        
        # Choose icon based on file extension
        if file_ext in ['.jpg', '.jpeg', '.png', '.gif', '.bmp', '.svg']:
            icon = "🖼️"
        elif file_ext in ['.pdf']:
            icon = "📄"
        elif file_ext in ['.txt', '.md']:
            icon = "📝"
        elif file_ext in ['.zip', '.rar', '.7z', '.tar', '.gz']:
            icon = "📦"
        elif file_ext in ['.mp4', '.avi', '.mov', '.mkv']:
            icon = "🎬"
        elif file_ext in ['.mp3', '.wav', '.flac', '.ogg']:
            icon = "🎵"
        else:
            icon = "📄"
        
        print(f"""
            <div class="file-item">
                <div class="file-info">
                    <div class="file-icon">{icon}</div>
                    <div class="file-details">
                        <div class="file-name">{file_name}</div>
                        <div class="file-meta">
                            {file['info']['readable_size']} • Modified: {file['info']['modified']}
                        </div>
                    </div>
                </div>
                <div class="file-actions">
                    <form method="post" style="display: inline;" onsubmit="return confirm('Are you sure you want to delete {file_name}?');">
                        <input type="hidden" name="action" value="delete">
                        <input type="hidden" name="filename" value="{file_name}">
                        <button type="submit" class="delete-btn">🗑️ Delete</button>
                    </form>
                </div>
            </div>
        """)
    
    print("        </div>")
else:
    print(f"""
        <div class="empty-state">
            <div class="icon">📂</div>
            <h3>No files uploaded yet</h3>
            <p>Upload some files to see them listed here.</p>
            <div style="margin-top: 1.5rem;">
                <a href="/upload/uploadFile.html" class="button">📤 Upload Files</a>
            </div>
        </div>
    """)

print(f"""
        <!-- Action Buttons -->
        <div class="text-center" style="margin-top: 2rem;">
            <a href="/upload/uploadFile.html" class="button secondary">📤 Upload More Files</a>
            <a href="/" class="button secondary">🏠 Back to Home</a>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>📁 File Browser • Prophet Web Server v1.0</p>
        </div>
    </div>
</body>
</html>""")
