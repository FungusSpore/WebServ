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

# Decide what to display immediately
current_user = posted if (action == "login" and posted) else (get_cookie_value_from_set_cookie(raw_set_cookie, "user_id"))

# Prepare outgoing cookie if login (kept simple, add Max-Age if you want persistence)
# cookie_out = http.cookies.SimpleCookie()
# if action == "login" and posted:
#     cookie_out["user_id"] = posted
#     cookie_out["user_id"]["path"] = "/"
#     # cookie_out["user_id"]["max-age"] = 2592000  # 30 days (uncomment for persistence)
#     print(cookie_out.output())

# Always emit your custom header BEFORE the blank line
# Format: cgi_cookie: session_id="<session_id>"; user_id=<posted or "">
print(f'cgi_cookie: session_id={session_id}; user_id={current_user}')

print()  # end of headers

# HTML
safe_user = html.escape(current_user, quote=True)
guest_html = "<span class='muted'>Guest</span>"

print(f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Login Test</title>
  <link rel="stylesheet" href="/css/style.css">
  <style>   
    body {{ font-family: Arial, sans-serif; background:#f9f9f9; color:#333; text-align:center; padding:50px; }}
    .container {{ max-width:520px; margin:auto; background:#fff; padding:30px; border-radius:12px; box-shadow:0 0 15px rgba(0,0,0,0.1); }}
    h1 {{ color:#4CAF50; }}
    form {{ margin-top:20px; display:inline-block; }}
    input[type="text"] {{ padding:10px; width:70%; border:1px solid #ccc; border-radius:6px; }}
    button, a.button {{
      display:inline-block; padding:10px 20px; background:#4CAF50; color:#fff; border:none; border-radius:6px;
      cursor:pointer; margin:10px 5px; text-decoration:none; font-size:1em;
    }}
    button:hover, a.button:hover {{ background:#45a049; }}
    .muted {{ color:#888; font-weight:normal; }}
    .secondary {{ background:#607d8b; }}
    .secondary:hover {{ background:#546e7a; }}
    .msg {{ margin:15px 0; font-size:1.1em; }}
    .tiny {{ color:#777; font-size:0.9em; margin-top:6px; }}
  </style>
</head>
<body>
  <div class="container">
    <h1>🔑 Login Test</h1>
    <p class="msg">Welcome, <strong>{"%s" if current_user == "Guest" else safe_user}</strong>!</p>
""".replace("%s", guest_html))

# UI
print("""
<form method="POST" action="/cgi-bin/login.py">
  <input type="hidden" name="action" value="login">
  <input type="text" name="user_id" placeholder="Enter User ID" required>
  <button type="submit">Login</button>
</form>
<a href="/" class="button secondary">🏠 Home</a>
""")

print("""
  </div>
</body>
</html>
""")
