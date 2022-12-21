#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <fstream>
#include <string>
#include <poll.h>

#define RESPONSE_TIMEOUT 1500
#define RESPONSE_BUF 8096

using namespace std;


void parse_operations(int socket_fd, string file_path);
string construct_get(string file_path);
string construct_post(string file_path);
int prepare_socket(string ip_address, string port_number);
bool wait_response(int socket_fd);
void handle_response(int socket_fd, string operation, string file_path);
void handle_get_response(string response, string file_path);

int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 4) {
    cout << "ERROR: expected 2 mandatory arguments: IP Address, Command File Path and one optional argument: Port Number";
  }
  string ip_address;
  string port_number;
  string file_path;
  if (argc == 3) {
    ip_address = argv[1];
    port_number = "80";
    file_path = argv[2];
  } else {
    ip_address = argv[1];
    port_number = argv[2];
    file_path = argv[3];
  }
  int socket_fd = prepare_socket(ip_address, port_number);
  parse_operations(socket_fd, file_path);
}

int prepare_socket(string ip_address, string port_number) {
  struct addrinfo hints, *address;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  int err = getaddrinfo(ip_address.c_str(), port_number.c_str(), &hints, &address);
  if (err != 0) {
    cout << "Error getting IP Address info, terminating...";
    exit(EXIT_FAILURE);
  }
  int socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
  int connection_err = connect(socket_fd, address->ai_addr, address->ai_addrlen);
  if (connection_err != 0) {
    cout << "Error while establishing connection, terminating...";
    exit(EXIT_FAILURE);
  }
  return socket_fd;
}

void parse_operations(int socket_fd, string file_path) {
  ifstream operation_file;
  operation_file.open(file_path, ios::in);

  string line;
  while(operation_file.peek() != EOF) {

    std::getline(operation_file, line);
    size_t string_break = line.find(' ');
    string operation = line.substr(0, string_break);
    line.erase(0, string_break+1);
    string_break = line.find(' ');
    string file_path = line.substr(0, string_break);

    if (operation == "client_get") {
      string msg = construct_get(file_path);
      send(socket_fd, msg.c_str(), msg.length(), 0);
    } else if (operation == "client_post") {
      string msg = construct_post(file_path);
      send(socket_fd, msg.c_str(), msg.length(), 0);
    }

    if (!wait_response(socket_fd)) continue;

    handle_response(socket_fd, operation, file_path);
  }
}

string construct_get(string file_path) {
  string res = "GET " + file_path + " HTTP/1.1" + '\r' + '\n' + '\r' + '\n';
  return res;
}

string construct_post(string file_path) {
  string res = "POST " + file_path + " HTTP/1.1" + '\r' + '\n' + '\r' + '\n';
  ifstream file_stream(file_path);
  string data = "";
  while(file_stream.peek() != EOF) {
    string line;
    getline(file_stream, line);
    data += line;
  }
  res += data;
  return res;
}

bool wait_response(int socket_fd) {
  struct pollfd poll_fd[1];
  poll_fd[0].fd = socket_fd;
  poll_fd[0].events = POLLIN;
  int is_ready = poll(poll_fd, 1, RESPONSE_TIMEOUT);
  if (!is_ready) {
    cout << "No response received from server, skipping to next instruction.";
    return false;
  }
  return true;
}

void handle_response(int socket_fd, string operation, string file_path) {
  char response[RESPONSE_BUF];
  recv(socket_fd, response, RESPONSE_BUF, 0);
  string resp(response);
  cout << resp;
  if (operation == "client_get") {
    handle_get_response(response, file_path);
  }
}

void handle_get_response(string response, string file_path) {
  size_t string_break = response.find(' ');
  response.erase(0, string_break+1);
  string_break = response.find(' ');
  string status_code = response.substr(0, string_break);
  if (status_code == "404") return;
  else if (status_code == "200") {
    response.erase(0, string_break+6);
    ofstream output_stream;
    output_stream.open(file_path, ios::out);
    output_stream << response;
  }
}
