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

#include "coordinator.h"
#include <iostream>
#include <fstream>

Define_Module(Coordinator);



void Coordinator::initialize()
{

    std::ifstream filestream;
    std::string line;

    filestream.open("coordinator.txt", std::ifstream::in);

    if(!filestream) {
        throw cRuntimeError("Error opening file '%s'?", "coordinator.txt");
    } else {
        getline(filestream, line);
        EV<<line;
        std::string node=line.substr(0, 1);
        std::string time=line.substr(2, line.length());
        EV<< node<<endl;
        EV<< time<<endl;
//        int n = time.length();
//        char char_array[n + 1];
//        strcpy(char_array, time.c_str());
        cMessage * msg= new cMessage(time.c_str());

        if(strcmp(node.c_str(),"0")==0){
            send(msg,"out0");
        }else{
            send(msg,"out1");
        }

    }

    //parseInputLine(line);

}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
