//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "node.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <fstream>

#include "MyMessage_m.h"
#include <bitset>
typedef  std::bitset<8> bits;
#include <iostream>
#include <fstream>
Define_Module(Node);
std::bitset<8> Node::ParityByteErrorDetection(std::string payload)
{

    char c;
    std::bitset<8> result(0);
    for (int i = 0; i < payload.length(); i++)
    {
        c = payload[i];
        std::bitset<8> binary_char(c);
        result ^= binary_char;
    }
    return result;
}

std::string Node::byteStuffing(std::string payload)
{
    std::string payload_after_stuffing = "$";
    for(int i = 0; i < payload.size(); i++)
    {
        if (payload[i] == '$' || payload[i] == '/')
        {
            payload_after_stuffing = payload_after_stuffing + "/";
        }
        payload_after_stuffing = payload_after_stuffing + payload[i];
    }

    payload_after_stuffing = payload_after_stuffing + "$";

    std::cout << " the Message after applying byte stuffing is "<<payload_after_stuffing<<endl;
    return payload_after_stuffing;

}

void Node::initialize()
{
    // for testing bytestuffing and parity byte
    std::cout<< byteStuffing("$/Eng/Menna")<<endl;
    std::cout<< ParityByteErrorDetection("4hi")<< endl;

    // TODO - Generated method body
}

void Node::read_msgs( std::vector<std::string>& errors_arr, std::vector<std::string>& msg_arr  ){
    std::string file_name="";
    if(strcmp(getName(),"node0")==0){
        file_name="input0.txt";
    }else{
        file_name="input1.txt";
    }

    EV<<file_name<<endl;

    std::ifstream filestream;
    std::string line;
    filestream.open(file_name, std::ifstream::in);
    if(!filestream) {
          throw cRuntimeError("Error opening file '%s'?", "coordinator.txt");
     } else {

         while(getline(filestream, line)){
             std::string error = line.substr(0, 4);
             errors_arr.push_back(error);
             std::string msg = line.substr(5, line.length());
             msg_arr.push_back(msg);
            //getline(filestream, line);
         }
     }
}
void Node::send_msg(){
    std::vector<std::string>errors_arr;
    std::vector<std::string>msg_arr;
    read_msgs(errors_arr,msg_arr);
    EV<<errors_arr[0] << " " << msg_arr[0] << endl;

}
void Node::receive_msg(){


}
void Node::handleMessage(cMessage *msg)
{
    std::string line=msg->getName();
    std::string node=line.substr(0, 1);
    std::string start_time=line.substr(2, line.length());

    // wait until start time
    if(msg->isSelfMessage()){
        allow_to_send = true;
    }else
    {
            if(( strcmp(getName(),"node0")==0 && strcmp(node.c_str(),"0")==0 ) || ( strcmp(getName(),"node1")==0 && strcmp(node.c_str(),"1")==0 ) ){
               EV<<getName()<<" Senderr "<<endl;
               scheduleAt(std::stod(start_time), new cMessage(" self messaging .."));
               is_sender = true;
            }else{
                EV<<getName()<<" Reciveer "<<endl;
            }
    }
    if (is_sender)
    {
        if(allow_to_send){
            EV<<line<<endl;
            send_msg();
        }
    }else
    {
        receive_msg();
    }
    //EV<<"From Node"<<msg->getName()<<endl;
}






