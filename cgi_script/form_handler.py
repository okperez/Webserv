
import os

# Start the HTML
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>CGI Test</title>")
print("<link rel=\"stylesheet\" type=\"text/css\" href=\"/session.css\">")
print("</head>")
print("<body>")
print("<div id=\"center\">")
print("<h1>CGI Test</h1>")
print("<h2>Environment Variables</h2>")
print("<ul id=\"text\" >")

# Print each environment variable
for key, value in os.environ.items():
    print(f"<li><strong>{key}</strong>: {value}</li>")

print("</ul>")
print("</div>")
print("</body>")
print("</html>")
