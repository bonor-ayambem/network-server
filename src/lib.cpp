#include <vector>
#include <stdio.h>
#include<stdlib.h>
#include <iostream>
#include "hashmap.hpp"
#include "pack109.hpp"

HashMap::HashMap() {
  this->size = this->array.size();
}

bool HashMap::insert(string key, File value){
    for(int i = 0; i < this->size; i++){
        if(this->array[i].key == key){
            this->array[i].value = value;
            return true;
        }
    }

    struct Pair new_pair;
    new_pair.key = key;
    new_pair.value = value;

    this->array.push_back(new_pair);
    this->size++;
    return false;
}

File HashMap::get (string key){
    for(int i = 0; i < this->size; i++){
        if(this->array[i].key == key){
            return this->array[i].value;
        }
    }

    throw;
}

using std::begin, std::end;
#define X8  256
#define X16 65536
#define MASK4 0x000000FF
#define MASK8 0x00000000000000FF

vec pack109::slice(vec& bytes, int vbegin, int vend) {
  auto start = bytes.begin() + vbegin;
  auto end = bytes.begin() + vend + 1;
  vec result(vend - vbegin + 1);
  copy(start, end, result.begin());
  return result;
}

vec pack109::serialize(u8 item) {
  vec bytes;
  bytes.push_back(PACK109_U8);
  bytes.push_back(item);
  return bytes;
}

u8 pack109::deserialize_u8(vec bytes) {
  if (bytes.size() < 2) {
    throw;
  }
  if (bytes[0] == PACK109_U8) {
    return bytes[1];
  } else {
    throw;
  }
}

vec pack109::serialize(string item) {
  vec bytes;
  if (item.size() < 256) {
    bytes.push_back(PACK109_S8);
    bytes.push_back((u8)item.size());
    for (int i = 0; i < item.size(); i++) {
      bytes.push_back(item[i]);
    }
  } else {
    throw;
  }
  return bytes;
}

string pack109::deserialize_string(vec bytes) {
  if(bytes.size() < 3) {
    throw;
  }
  string deserialized_string = "";
  if(bytes[0] == PACK109_S8) {
    int string_length = bytes[1];
    for(int i = 2; i < (string_length + 2); i++) {
      deserialized_string += bytes[i];
    }
  }
  return deserialized_string;
}

vec pack109::serialize(std::vector<u8> item) {
  vec bytes;
  if (item.size() < X8) {
    bytes.push_back(PACK109_A8);
    u8 size = (u8)item.size();
    bytes.push_back(size);
    for (int i = 0; i < item.size(); i++) {
      vec temp = serialize(item[i]);
      for (int j = 0; j < temp.size(); j++) {
        bytes.push_back(temp[j]);
      }
    }
  } else {
    throw;
  }
  return bytes;
}

std::vector<u8> pack109::deserialize_vec_u8(vec bytes) {
  if(bytes.size() < 3) {
    throw;
  }
  int el_size = 2;
  std::vector<u8> result;
  if(bytes[0] == PACK109_A8) {
    int size = el_size * bytes[1];
    for (int i = 2; i < (size + 2); i += el_size) {
      vec sub_vec = slice(bytes, i, i + el_size);
      u8 element = deserialize_u8(sub_vec);
      result.push_back(element);
    }
  }
  return result;
}

vec pack109::serialize(struct File item) {
  vec bytes;
  bytes.push_back(PACK109_M8);
  bytes.push_back(0x1); // 1 k/v pair

  // The key is "File"
  vec file = serialize("File");
  bytes.insert(end(bytes), begin(file), end(file));
  // The value is an m8
  bytes.push_back(PACK109_M8);
  bytes.push_back(0x2); // 2 k/v pairs

  // k/v 1 is "name"
  vec f_namek = serialize("name");
  bytes.insert(end(bytes), begin(f_namek), end(f_namek));
  vec f_namev = serialize(item.name);
  bytes.insert(end(bytes), begin(f_namev), end(f_namev));
  // k/v 2 is "bytes"
  vec f_bytesk = serialize("bytes");
  bytes.insert(end(bytes), begin(f_bytesk), end(f_bytesk));
  vec f_bytesv = serialize(item.bytes);
  bytes.insert(end(bytes), begin(f_bytesv), end(f_bytesv));

  // printVec(bytes);
  return bytes;
}

struct File pack109::deserialize_file(vec bytes) {
    // printVec(bytes);
  if (bytes.size() < 8) {
    throw;
  }
  vec file_slice = slice(bytes, 2, 7);
  string file_string = deserialize_string(file_slice);
  if (file_string != "File") {
    throw;
  }

  u8 name_len = bytes[17]; //constant
  vec namev = slice(bytes, 16, 16 + name_len + 1);
  string name = deserialize_string(namev);

  u8 filedata_len = bytes[17 + name_len + 7 + 2];
  u8 filedata_start = 17 + name_len + 7 + 1;
  u8 filedata_end = (17 + name_len + 7 + 1) + filedata_len*2 + 1;
  vec bytesv = slice(bytes, filedata_start, filedata_end);
  vec b = deserialize_vec_u8(bytesv);

  struct File deserialized_file = {name, b};
  return deserialized_file;
}


struct Request pack109::deserialize_request(vec bytes) {
  if (bytes.size() < 11) {
    throw;
  }
  vec request_slice = slice(bytes, 2, 10);
  string request_string = deserialize_string(request_slice);
  if (request_string != "Request") {
    throw;
  }

  u8 name_len = bytes[20]; // Assumes name fewer than 256 chars. Safe assumption I think
  vec namev = slice(bytes, 19, 19 + name_len + 1);
  string name = deserialize_string(namev);
  struct Request deserialized_request = {name};
  return deserialized_request;
}

void pack109::printVec(vec &bytes) {
  printf("[ ");
  for (int i = 0; i < bytes.size(); i++) {
    printf("%x ", bytes[i]);
  }
  printf("]\n");
}