
import os

# Print the HTTP headers
print("Content-Type: text/html\n")

# Start the HTML
print("<html>")
print("<head>")
print("<title>CGI Test</title>")
print("</head>")
print("<body>")
print("<h1>CGI Test</h1>")
print("<h2>Environment Variables</h2>")
print("<ul>")

# Print each environment variable
for key, value in os.environ.items():
    print(f"<li><strong>{key}</strong>: {value}</li>")

print("</ul>")
print("</body>")
print("</html>")
