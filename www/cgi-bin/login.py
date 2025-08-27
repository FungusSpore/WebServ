#!/usr/bin/env python3
import os
import cgi
import http.cookies
import html

print("Content-Type: text/html")

# Parse form data
form   = cgi.FieldStorage()
action = form.getfirst("action", "")          # "login", "logout", "clear" (logout keeps cookie)
posted = form.getfirst("user_id", "") or ""   # posted user_id (may be empty)

# Current user comes ONLY from your custom env (server must populate it)
# env_user_id = os.environ.get("CGI_COOKIE", "")

# Extract session_id from the non-standard env you pass: HTTP_SET_COOKIE
raw_set_cookie = os.environ.get("CGI_COOKIE", "")

def get_cookie_value_from_set_cookie(raw_header: str, key: str) -> str:
    """Extract key=value from a Set-Cookie-like header (best effort)."""
    if not raw_header:
        return ""
    try:
        c = http.cookies.SimpleCookie()
        c.load(raw_header)  # works for single Set-Cookie line like: session_id="..."; Path=/; HttpOnly
        if key in c and c[key].value is not None:
            return c[key].value
    except Exception:
        pass
    # Fallback naive parse
    parts = [p.strip() for p in raw_header.split(';')]
    for p in parts:
        if '=' in p:
            k, v = p.split('=', 1)
            if k.strip() == key:
                return v.strip().strip('"')
    return ""

session_id = get_cookie_value_from_set_cookie(raw_set_cookie, "session_id") or ""

# Decide what to display and what to send back
if action == "login" and posted:
    # User is logging in with a new username
    current_user = posted
    output_user = posted
    output_session = session_id  # Keep existing session
elif action == "logout":
    # User is logging out, destroy session and create new one
    current_user = "Guest"
    output_user = "Guest"
    output_session = f"DESTROY:{session_id}"  # Signal server to destroy this specific session
else:
    # Default case: display current user from cookie, output same
    current_user = get_cookie_value_from_set_cookie(raw_set_cookie, "user_id") or "Guest"
    output_user = current_user
    output_session = session_id  # Keep existing session

# Always emit your custom header BEFORE the blank line
# Format: cgi_cookie: session_id="<session_id>"; user_id=<output_user>
# Fixed: C++ server now properly handles line ending detection
print(f'cgi_cookie: session_id={output_session}; user_id={output_user}')

print()  # end of headers

# HTML
safe_user = html.escape(current_user, quote=True)
guest_html = "<span class='muted'>Guest User</span>"

user_status_class = 'guest-status' if current_user == "Guest" else ''
user_display = guest_html if current_user == "Guest" else safe_user

print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🔑 User Authentication - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
    <style>
        .auth-container {{
            max-width: 480px;
            margin: 0 auto;
        }}
        
        .user-status {{
            background: linear-gradient(135deg, #667eea, #764ba2);
            color: white;
            padding: 20px;
            border-radius: 15px;
            margin: 20px 0;
            text-align: center;
        }}
        
        .guest-status {{
            background: linear-gradient(135deg, #95a5a6, #7f8c8d);
        }}
        
        .login-form {{
            background: rgba(255, 255, 255, 0.95);
            backdrop-filter: blur(10px);
            padding: 30px;
            border-radius: 15px;
            border: 1px solid rgba(255, 255, 255, 0.2);
            box-shadow: 0 8px 32px rgba(31, 38, 135, 0.37);
        }}
        
        .form-group {{
            margin: 20px 0;
            text-align: left;
        }}
        
        .form-group label {{
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #2c3e50;
        }}
        
        .form-input {{
            width: 100%;
            padding: 12px 16px;
            border: 2px solid #e1e8ed;
            border-radius: 10px;
            font-size: 16px;
            transition: all 0.3s ease;
            box-sizing: border-box;
        }}
        
        .form-input:focus {{
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }}
        
        .session-info {{
            background: #f8f9fa;
            border-left: 4px solid #667eea;
            padding: 15px;
            margin: 20px 0;
            border-radius: 0 8px 8px 0;
        }}
        
        .muted {{
            color: #7f8c8d;
            font-style: italic;
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

        <div class="auth-container">
            <h1>🔑 User Authentication</h1>
            
            <div class="user-status {user_status_class}">
                <h3>👤 Current User</h3>
                <p class="h2">{user_display}</p>
                <small>Session-based authentication active</small>
            </div>""")

# UI
if current_user and current_user != "Guest":
    print(f"""
            <div class="card">
                <h3>🚪 Session Management</h3>
                <div class="session-info">
                    <p><strong>Logged in as:</strong> {safe_user}</p>
                    <p><strong>Session Status:</strong> <span class="status running">✅ Active</span></p>
                    <p><strong>Authentication:</strong> Cookie-based</p>
                </div>
                
                <form method="POST" action="/cgi-bin/login.py" class="text-center">
                    <input type="hidden" name="action" value="logout">
                    <button type="submit" class="button secondary">🚪 Logout</button>
                </form>
            </div>
            
            <div class="card">
                <h3>🔧 Account Actions</h3>
                <ul class="feature-list">
                    <li>Session is stored server-side for security</li>
                    <li>Logout will destroy your current session</li>
                    <li>Navigate freely while logged in</li>
                    <li>Session persists across page visits</li>
                </ul>
            </div>
""")
else:
    print("""
            <div class="card">
                <div class="login-form">
                    <h3>🔐 Sign In</h3>
                    <form method="POST" action="/cgi-bin/login.py">
                        <input type="hidden" name="action" value="login">
                        
                        <div class="form-group">
                            <label for="user_id">👤 User ID</label>
                            <input type="text" 
                                   id="user_id"
                                   name="user_id" 
                                   class="form-input"
                                   placeholder="Enter your username"
                                   required>
                        </div>
                        
                        <div class="text-center">
                            <button type="submit" class="button">🔑 Login</button>
                        </div>
                    </form>
                </div>
            </div>
            
            <div class="card">
                <h3>ℹ️ Authentication Info</h3>
                <ul class="feature-list">
                    <li>Enter any username to create a session</li>
                    <li>Sessions are managed server-side</li>
                    <li>No password required for demo purposes</li>
                    <li>Cookies used for session tracking</li>
                </ul>
            </div>
""")

print("""
        </div>

        <!-- Quick Actions -->
        <div class="text-center mt-3">
            <a href="/" class="button secondary">🏠 Home</a>
            <a href="/cgi-bin/hello_process.py" class="button secondary">👋 Hello Demo</a>
            <a href="/cgi-bin/tic-tac-toe.py" class="button secondary">🎯 Tic Tac Toe</a>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>Secure session management powered by Prophet Web Server CGI</p>
        </div>
    </div>
</body>
</html>
""")
