#!/usr/bin/env python3
import os
import sys
import time
import html
from urllib.parse import parse_qs

# ========= Settings =========
# Directory the CGI lists from (this is just for rendering; deletes go via HTTP DELETE to the server)
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "storage")

# Your webserver's native DELETE endpoint (matches your config: location /upload { alias ... })
# DELETE /upload/<filename> -> MiniHttpResponse::handleDelete()
DELETE_BASE_URL = "/storage/"

# ========= Helpers =========
def format_file_size(size_bytes):
    if size_bytes == 0:
        return "0 B"
    size_names = ["B", "KB", "MB", "GB"]
    i = 0
    while size_bytes >= 1024 and i < len(size_names) - 1:
        size_bytes /= 1024.0
        i += 1
    return f"{size_bytes:.1f} {size_names[i]}"

def get_file_info(filepath):
    try:
        st = os.stat(filepath)
        return {
            "size": st.st_size,
            "modified": time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(st.st_mtime)),
            "readable_size": format_file_size(st.st_size),
        }
    except Exception:
        return {"size": 0, "modified": "Unknown", "readable_size": "0 B"}

def safe_join(base, *paths):
    joined = os.path.normpath(os.path.join(base, *paths))
    base_abs = os.path.abspath(base)
    joined_abs = os.path.abspath(joined)
    if not (joined_abs == base_abs or joined_abs.startswith(base_abs + os.sep)):
        raise ValueError("Unsafe path")
    return joined_abs

def redirect_with_alert(msg, location="/cgi-bin/browse_uploads.py"):
    print("Content-Type: text/html")
    print()
    print(f"<script>alert('{html.escape(msg)}'); window.location.href='{location}';</script>")
    sys.exit(0)

# ========= POST fallback (non-JS) =========
method = os.environ.get("REQUEST_METHOD", "GET").upper()
if method == "POST":
    try:
        cl = int(os.environ.get("CONTENT_LENGTH", 0))
        body = sys.stdin.read(cl) if cl > 0 else ""
        params = parse_qs(body)

        action = params.get("action", [""])[0]
        if action == "delete" and "filename" in params:
            filename = params["filename"][0]
            if "/" in filename or "\\" in filename or filename in (".", ".."):
                redirect_with_alert("Invalid filename.")
            try:
                path = safe_join(UPLOAD_DIR, filename)
            except ValueError:
                redirect_with_alert("Invalid path.")
            if os.path.isfile(path):
                try:
                    os.remove(path)
                    redirect_with_alert(f"File {filename} deleted successfully!")
                except Exception as e:
                    redirect_with_alert(f"Error deleting file: {e}")
            else:
                redirect_with_alert("File not found.")

        elif action == "delete_all":
            deleted = 0
            if os.path.isdir(UPLOAD_DIR):
                for name in os.listdir(UPLOAD_DIR):
                    try:
                        p = safe_join(UPLOAD_DIR, name)
                    except ValueError:
                        continue
                    if os.path.isfile(p):
                        try:
                            os.remove(p)
                            deleted += 1
                        except Exception:
                            pass
            redirect_with_alert(f"{deleted} files deleted successfully!")

        else:
            redirect_with_alert("Invalid action.")
    except Exception as e:
        redirect_with_alert(f"Error: {e}")

# ========= Build file list (GET view) =========
files = []
try:
    if os.path.isdir(UPLOAD_DIR):
        for name in os.listdir(UPLOAD_DIR):
            try:
                p = safe_join(UPLOAD_DIR, name)
            except ValueError:
                continue
            if os.path.isfile(p):
                files.append({"name": name, "path": p, "info": get_file_info(p)})
        files.sort(key=lambda x: x["name"].lower())
except Exception:
    files = []

# ========= HTML =========
print("Content-Type: text/html")
print()
print(f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>📁 Browse Uploads - Prophet Web Server</title>
<link rel="stylesheet" href="/css/style.css">
<style>
.container{{max-width:900px;margin:0 auto;padding:1rem}}
.nav{{display:flex;justify-content:space-between;align-items:center;margin-bottom:1rem}}
.nav .logo{{font-weight:700}}
.nav-links{{display:flex;gap:1rem;list-style:none;padding:0;margin:0}}
.file-list{{background:var(--card-bg,#f8f9fa);border-radius:8px;padding:1rem;margin:1rem 0}}
.file-item{{display:flex;align-items:center;justify-content:space-between;padding:.75rem;border-bottom:1px solid var(--border-color,#e9ecef);transition:background-color .2s}}
.file-item:last-child{{border-bottom:none}}
.file-item:hover{{background-color:var(--hover-bg,#f1f3f5)}}
.file-info{{display:flex;align-items:center;gap:1rem;flex:1}}
.file-icon{{font-size:1.5rem;width:2rem;text-align:center}}
.file-details{{flex:1}}
.file-name{{font-weight:500;color:var(--text-primary,#333);margin-bottom:.25rem}}
.file-meta{{font-size:.875rem;color:var(--text-secondary,#666)}}
.file-actions{{display:flex;gap:.5rem;flex-wrap:wrap;justify-content:flex-end}}
.btn{{text-decoration:none;padding:.25rem .5rem;border-radius:4px;font-size:.875rem;border:none;cursor:pointer;transition:all .2s}}
.delete-btn{{background:#dc3545;color:#fff}}
.delete-btn:hover{{background:#c82333}}
.delete-http-btn{{background:#0d6efd;color:#fff}}
.delete-http-btn:hover{{background:#0b5ed7}}
.delete-all-btn{{background:#6c757d;color:#fff;padding:.5rem 1rem;margin-bottom:1rem}}
.delete-all-btn:hover{{background:#5a6268}}
.empty-state{{text-align:center;padding:3rem 1rem;color:var(--text-secondary,#666)}}
.empty-state .icon{{font-size:4rem;margin-bottom:1rem;opacity:.5}}
.stats{{display:flex;justify-content:space-between;align-items:center;margin-bottom:1rem;padding:.75rem;background:var(--info-bg,#e3f2fd);border-radius:6px;font-size:.875rem}}
.breadcrumb{{margin-bottom:1rem;font-size:.875rem;color:var(--text-secondary,#666)}}
.breadcrumb a{{color:var(--primary-color,#007bff);text-decoration:none}}
.breadcrumb a:hover{{text-decoration:underline}}
.text-center{{text-align:center}}
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

    <!-- Delete All (POST fallback, no JS required) -->
    <form method="post" onsubmit="return confirm('Are you sure you want to delete ALL files? This action cannot be undone!');" style="margin-bottom:1rem;">
        <input type="hidden" name="action" value="delete_all">
        <button type="submit" class="delete-all-btn">🗑️ Delete All Files (POST)</button>
    </form>

    <div class="file-list">
    """)

    for f in files:
        name = html.escape(f['name'])
        ext = os.path.splitext(f['name'])[1].lower()
        if ext in ['.jpg', '.jpeg', '.png', '.gif', '.bmp', '.svg']:
            icon = "🖼️"
        elif ext in ['.pdf']:
            icon = "📄"
        elif ext in ['.txt', '.md']:
            icon = "📝"
        elif ext in ['.zip', '.rar', '.7z', '.tar', '.gz']:
            icon = "📦"
        elif ext in ['.mp4', '.avi', '.mov', '.mkv']:
            icon = "🎬"
        elif ext in ['.mp3', '.wav', '.flac', '.ogg']:
            icon = "🎵"
        else:
            icon = "📄"

        print(f"""
        <div class="file-item">
            <div class="file-info">
                <div class="file-icon">{icon}</div>
                <div class="file-details">
                    <div class="file-name">{name}</div>
                    <div class="file-meta">{f['info']['readable_size']} • Modified: {f['info']['modified']}</div>
                </div>
            </div>
            <div class="file-actions">
                <!-- POST fallback (works without JS) -->
                <form method="post" style="display:inline;" onsubmit="return confirm('Are you sure you want to delete {name}?');">
                    <input type="hidden" name="action" value="delete">
                    <input type="hidden" name="filename" value="{name}">
                    <button type="submit" class="btn delete-btn">🗑️ Delete (POST)</button>
                </form>
                <!-- Native HTTP DELETE handled by your C++ server (refresh page on success) -->
                <button class="btn delete-http-btn" data-filename="{name}">DELETE (HTTP)</button>
            </div>
        </div>
        """)

    print("""
    </div>
    """)

else:
    print("""
    <div class="empty-state">
        <div class="icon">📂</div>
        <h3>No files uploaded yet</h3>
        <p>Upload some files to see them listed here.</p>
        <div style="margin-top:1.5rem;">
            <a href="/upload/uploadFile.html" class="button">📤 Upload Files</a>
        </div>
    </div>
    """)

print(f"""
    <div class="text-center" style="margin-top:2rem;">
        <a href="/upload/uploadFile.html" class="button secondary">📤 Upload More Files</a>
        <a href="/" class="button secondary">🏠 Back to Home</a>
    </div>

    <div class="footer">
        <p>📁 File Browser • Prophet Web Server v1.0</p>
    </div>
</div>

<script>
(function(){{
  var base = {DELETE_BASE_URL!r}; // injected from Python

  function deleteOne(filename){{
    if(!confirm('DELETE ' + filename + ' ?')) return;
    fetch(base + encodeURIComponent(filename), {{
      method: 'DELETE',
      cache: 'no-store'
    }}).then(function(res){{
      if(res.status === 204) {{
        // Refresh the page so the CGI re-renders the updated file list
        location.reload();
      }} else {{
        alert('Delete failed: HTTP ' + res.status);
      }}
    }}).catch(function(err){{
      alert('Network error: ' + err);
    }});
  }}

  document.querySelectorAll('.delete-http-btn').forEach(function(btn){{
    btn.addEventListener('click', function(){{
      var name = btn.getAttribute('data-filename');
      if(name) deleteOne(name);
    }});
  }});
}})();
</script>

</body>
</html>
""")
