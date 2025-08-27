#!/usr/bin/env python3
import cgi
import html

# Always output headers first, then exactly one blank line
print("Content-type: text/html")
print()

# Parse form data
form = cgi.FieldStorage()
name = form.getfirst("name", "")
happy = form.getfirst("happy", "")
sad = form.getfirst("sad", "")

print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>👋 Hello CGI - Prophet Web Server</title>
    <link rel="stylesheet" href="/css/style.css">
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
        <h1>👋 Hello CGI Demo</h1>
        
        <div class="text-center mb-3">
            <span class="status running">🟢 CGI Script Active</span>
            <span class="highlight">Dynamic Content Generation</span>
        </div>
""")

# Process form responses
if name:
    safe_name = html.escape(name, quote=True)
    print(f"""
        <div class="card">
            <h3>🎉 Hello, {safe_name}!</h3>
            <p>Thank you for using our CGI script! Your name was processed successfully.</p>
        </div>
    """)

if happy:
    print("""
        <div class="card" style="border-left: 5px solid #27ae60;">
            <h3>😊 That's Wonderful!</h3>
            <p>Yayy! We're happy too! Thanks for spreading the joy!</p>
        </div>
    """)

if sad:
    print("""
        <div class="card" style="border-left: 5px solid #e74c3c;">
            <h3>� Oh No!</h3>
            <p>Why are you sad? We hope our web server can cheer you up!</p>
        </div>
    """)

# Interactive form
print("""
        <h2>🎮 Try the Interactive Form</h2>
        <div class="card">
            <form method="GET" action="/cgi-bin/hello_process.py">
                <div style="margin: 15px 0;">
                    <label for="name"><strong>Your Name:</strong></label><br>
                    <input type="text" name="name" id="name" placeholder="Enter your name" 
                           style="padding: 10px; width: 70%; border: 1px solid #ddd; border-radius: 5px; margin-top: 5px;">
                </div>
                
                <div style="margin: 15px 0;">
                    <label><strong>How are you feeling?</strong></label><br>
                    <label style="margin: 10px; font-weight: normal;">
                        <input type="checkbox" name="happy" value="1"> 😊 Happy
                    </label>
                    <label style="margin: 10px; font-weight: normal;">
                        <input type="checkbox" name="sad" value="1"> 😢 Sad
                    </label>
                </div>
                
                <button type="submit" class="button success">
                    <i>👋</i> Say Hello!
                </button>
            </form>
        </div>

        <h2>🔧 CGI Information</h2>
        <div class="card">
            <p><strong>Script:</strong> hello_process.py</p>
            <p><strong>Method:</strong> GET (query parameters)</p>
            <p><strong>Purpose:</strong> Demonstrates basic form processing and CGI interaction</p>
            <p><strong>Features:</strong> Input sanitization, HTML escaping, dynamic responses</p>
        </div>

        <!-- Quick Actions -->
        <div class="text-center mt-3">
            <a href="/" class="button secondary">🏠 Home</a>
            <a href="/cgi-bin/tic-tac-toe.py" class="button">🎯 Play Game</a>
            <a href="/cgi-bin/login.py" class="button">🔑 Login Demo</a>
        </div>

        <!-- Footer -->
        <div class="footer">
            <p>CGI script powered by Prophet Web Server | Python {"{}"}</p>
        </div>
    </div>
</body>
</html>""".format("3.x"))
