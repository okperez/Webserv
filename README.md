🌐 Aperçu

Un serveur HTTP/1.1 écrit en C++98, conçu pour gérer des connexions simultanées sans blocage grâce à l'I/O Multiplexing.
🚀 Fonctionnalités

✅ Non-Blocking : Utilisation de select() (ou poll/epoll) pour une gestion asynchrone.

✅ Multiplexage : Un seul thread gère des centaines de clients en parallèle.

✅ Protocole HTTP/1.1 : Support complet des méthodes GET, POST et DELETE.

✅ Configuration : Paramétrage des ports, hôtes, client_max_body_size et routes.

✅ CGI : Support des scripts externes (Python, PHP, Bash) via l'interface CGI.

✅ Gestion d'Erreurs : Pages d'erreurs (404, 500, etc.) personnalisables.
🛠️ Usage
make
./webserv [config_file.conf]

⚙️ Exemple de Config
Nginx
server {
    listen 8080;
    server_name localhost;
    root ./www;
    index index.html;
}
