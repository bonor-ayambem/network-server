#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <string>
#include <vector>

#include "hashmap.hpp"
#include "pack109.hpp"

using namespace std;

vector<unsigned char> encrypt(vector<unsigned char> msg){
    for(int i = 0; i < msg.size(); i++){
        msg[i] = msg[i] ^ 42;
    }

    return msg;
}

int main(int argc, char *argv[]) {
    HashMap map;
    
    //check for errors within call arguments
    if(argc < 3){
        printf("Missing input arguments\n");
        exit(1);
    }
    string flag = argv[1];
    if(flag != "--hostname"){
        printf("Missing hostname flag\n");
        exit(1);
    }

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    vector<unsigned char> buffer(100000);
    struct sockaddr_in serv_addr, cli_addr;
    int  n;
   
   /* First call to socket() function */ 
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   string address = argv[2];
   int pos = address.find(":");
   portno = atoi(address.substr(pos+1, std::string::npos).c_str());

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
      
   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */    
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
        
    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("ERROR on accept");
        exit(1);
    }
        
    while(true){
       /* If connection is established then start communicating */
       bzero(buffer.data(), buffer.size());
       n = recv(newsockfd, buffer.data(), buffer.size(), MSG_CONFIRM);
       if (n < 0) {
           perror("ERROR reading from socket");
           exit(1);
       }
       
       vector<unsigned char> received_message;
       int i = 0;
       while(buffer[i] != '\0'){
           received_message.push_back(buffer[i]);
           i++;
       }

       vector<unsigned char> decrypted_message = encrypt(received_message);

       if(pack109::deserialize_string(pack109::slice(decrypted_message, 2, 7)) == "File"){
           struct File deserialized_message = pack109::deserialize_file(decrypted_message);
           map.insert(deserialized_message.name, deserialized_message);
           printf("File added to map: %s\n", deserialized_message.name.c_str());
           n = write(newsockfd,"FILE SENT", 9);
       }
       else if(pack109::deserialize_string(pack109::slice(decrypted_message, 2, 10)) == "Request"){
           struct Request deserialized_message = pack109::deserialize_request(decrypted_message);
           struct File get_file = map.get(deserialized_message.name);

           vector<unsigned char> serialized_file = pack109::serialize(get_file);
           vector<unsigned char> encrypted_file = encrypt(serialized_file);

           /* Send file to the client */
            n = send(newsockfd, encrypted_file.data(), encrypted_file.size(), MSG_CONFIRM);
            if (n < 0) {
                perror("ERROR sending file over socket");
                exit(1);
            }

            printf("File sent to client: %s\n", get_file.name.c_str());
            n = write(newsockfd,"FILE RECEIVED", 9);
       }
       else{
           exit(1);
       }
   }
   return 0;
}