#include "my_socket.hpp"

MySocket::MySocket(int sock):sock(sock)
{

}


void MySocket::close()
{
    ::close(sock);
    ::shutdown(sock, SHUT_RDWR);
}

void MySocket::flush()
{   
    for(int i = 0; i < out_buf.size(); i += 1024){
        send(sock, (this->out_buf.c_str() + i), std::min((int)out_buf.size() - i, 1024), MSG_NOSIGNAL);
    }
    this->out_buf = "";
}

void MySocket::reset()
{
    this->in_buf = "";
}

void MySocket::read()
{   

    char buf[2048];

    int n = recv(sock, buf, sizeof(buf), MSG_NOSIGNAL);

    if(n == 0)
    {
        this->close();
        throw SocketException();
    }
    buf[n] = '\0';
    this->in_buf += std::string(buf);
}

int MySocket::get_sock()
{
    return this->sock;
}

MySocket& operator << (MySocket& s, std::string str)
{
    s.out_buf += str;
    return s;
}

MySocket& operator >> (MySocket& s, std::string &str)
{   
    s.read();
    str = s.in_buf;
    s.reset();
    return s;
}