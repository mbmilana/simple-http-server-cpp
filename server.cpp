#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <unistd.h>
using namespace std;

int main(){
    //создать сокет
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd==-1){
        cerr<<"Socket creation failed\n";
        return 1;
    }

    //переиспользовать порт
    int opt=1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //настройка адреса
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // слушаем на всех интерфейсах
    addr.sin_port = htons(4221);

    //привязка
    if(bind(server_fd, (struct sockaddr *)&addr, sizeof(addr))<0){
        cerr<<"Bind failed\n";
        return 1;
    }

    //слушать
    if(listen(server_fd, 5)<0){
        cerr<<"Listen failed\n";
    }
    cout<<"Server listening on port 4221...\n";

    while(true){
        //принимаем клиента
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if(client_fd<0){
            cerr<<"Accept failed\n";
            continue;
        }

        //читать HTTP-запрос
        char buffer[4096] = {0};
        read(client_fd, buffer, sizeof(buffer)-1);
        string request(buffer);

        //парсинг пути
        string path="/";
        if(request.find("GET /") != string::npos) {
            size_t start = request. find("GET /")+5,
                    end = request.find(" ", start);
            path=request.substr(start, end-start);
            if(path.empty())
                path="/";
        }
        //сформировать ответ
        string response;
        if(path=="/"){
            response = "HTTP/1.1 200 OK\r\n\r\n";
        }
        else{
            response = "HTTP/1.1 404 Not Found\r\n\r\n";
        }
        write(client_fd, response.c_str(), response.length());

        //закрыть соединение
        close(client_fd);
    }
    //закрыть
    close(server_fd);
    return 0;
}
