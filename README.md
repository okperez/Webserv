🌐 Aperçu

Un serveur HTTP/1.1 écrit en C++98, capable de gérer plusieurs connexions simultanées sans jamais bloquer, grâce à l'utilisation de l'I/O Multiplexing.


✨ Fonctionnalités

    Non-Blocking: Utilisation de select() (ou poll/epoll) pour une gestion asynchrone des sockets.

    Multiplexage: Un seul thread peut gérer des centaines de clients en parallèle.

    Protocole HTTP/1.1: Support des méthodes GET, POST et DELETE.

    Fichier de Configuration: Paramétrage flexible des ports, hôtes, limites de corps (client_max_body_size) et routes.

    CGI: Support des scripts externes (Python, PHP, Bash) via l'interface CGI.

    Gestion d'Erreurs: Pages d'erreurs personnalisables (404, 403, 500, etc.).

🛠️ Installation & Usage

    # Compilation
    make

    # Lancer le serveur avec une config
    ./webserv [config_file.conf]
    
⚙️ Exemple de Configuration

    server {
    listen 8080;
    server_name localhost;
    root ./www;
    index index.html;

    location /cgi-bin {
        allow_methods GET POST;
        cgi_ext .py;
    }
    }
