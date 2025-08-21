#!/usr/bin/env python3
import cgi

# Always output headers first, then exactly one blank line
print("Content-type: text/html\r\n\r\n")

print("<html><body style='text-align:center;'>")
print("<h1 style='color: green;'>GeeksforGeeks</h1>")

form = cgi.FieldStorage()

if form.getvalue("name"):
    name = form.getvalue("name")
    print(f"<h2>Hello, {name}!</h2>")
    print("<p>Thank you for using our script.</p>")

if form.getvalue("happy"):
    print("<p>Yayy! We're happy too! 😊</p>")

if form.getvalue("sad"):
    print("<p>Oh no! Why are you sad? 😢</p>")

print("</body></html>")
