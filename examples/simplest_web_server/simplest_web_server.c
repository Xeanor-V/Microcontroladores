// Copyright (c) 2015 Cesanta Software Limited
// All rights reserved

#include "mongoose.h"
#include <bits/stdc++.h>
using namespace std;
typedef unsigned int uint;
vector< map<string,int> > casilla_votos;
map<string,bool> double_check;

//Metodos mongooso
static const char *s_http_port = "8000";
static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
  if (ev == MG_EV_HTTP_REQUEST) {
    mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
  }
}

void listener()
{
  //Configuracion del socket
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int puerto = 8080;
      
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }  
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( puerto );    
    // Forcefully attaching socket to the puerto 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    //Hilo para unir clientes
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                     (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    cout<<"Aceptado\n";
    while(true)
    {
      vector<string> muestras;
      for(int i = 0; i < 1024; i++)
      {
      unsigned char buffer1[1] = {0}, buffer2[1] = {0};
      read( new_socket , buffer1, 1);
      int dato1 = buffer1[0];
      //cout<<buffer1<<endl;
      read( new_socket , buffer2, 1);
      int dato2 = buffer2[0];
      //cout<<buffer2<<endl;
      int res = 0;
      if(dato1 & (1<<7))
      {
          dato1 = dato1 ^ (1<<7);
          res = dato1;
          res = res << 6;
          res = res | dato2;
      }
      else
      {
          dato2 = dato2 ^ (1<<7);
          res = dato2;
          res = res << 6;
          res = res | dato1;
      }
      string s = to_string(res);
      muestras.push_back(s);
      }
      cout<<"2048 muestras\n";
      ofstream JSON("Corazon.json");
      JSON<<"[";
      for(uint i = 0; i < muestras.size(); i++)
      {
        if(i>0)JSON<<",";
        JSON<<muestras[i];
      }
      JSON<<"]";
      JSON.close();
    }
    return;
}


/*
enviar_arreglo(char *array)
{
  int i = 0;
  int size = (sizeof(array)/sizeof(array[0]));
  for(i = 0 ; i < size; i++) U2TXREG = array[i];
}
*/




int main(void) 
{
  struct mg_mgr mgr;
  struct mg_connection *nc;
  //char at[] = "AT+CIPSTART=\"TCP\",\"192.168.0.195\",8000\r\n";
  //cout<<at<<endl;
  mg_mgr_init(&mgr, NULL);
  printf("Starting web server on port %s\n", s_http_port);
  nc = mg_bind(&mgr, s_http_port, ev_handler);
  if (nc == NULL) {
    printf("Failed to create listener\n");
    return 1;
  }
  // Set up HTTP server parameters
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = ".";  // Serve current directory
  s_http_server_opts.enable_directory_listing = "yes";
  thread listen(listener);
  for (;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  listen.join();
  mg_mgr_free(&mgr);

  return 0;
}


