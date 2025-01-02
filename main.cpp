#include <bits/stdc++.h>
using namespace std;

int main(int argc,char **argv){
  string input_buffer;
  while(1){
    std::cout << "db > ";
    getline(cin, input_buffer);
    if(input_buffer == ".exit"){
      exit(0);
    }
    else{
      cout << "Unrecognized command " << input_buffer << '\n';
    }
  }
  return 0;
}