import os
from urllib import parse


environ = os.environ
query_string = environ['QUERY_STRING'];
file = environ['QUERY_STRING'].split('=')[1];
print (file);
# server_host = environ.get('HTTP_HOST', 'localhost')
# server_port = environ.get('SERVER_PORT', 80)
# path = environ.get('SCRIPT_URL', '/')
# query_params = {}

# if '?' in environ.get('REQUEST_URI', '/'):
#     query_params = dict(parse.parse_qsl(parse.urlsplit(environ['REQUEST_URI']).query))

# print(f"Server Host: {server_host}")
# print(f"Server Port: {server_port}")
# print(f"Path: {path}")
# print(f"Query Params: {query_params}")
