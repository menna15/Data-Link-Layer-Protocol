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
#include "MyMessage_m.h"
#include <iostream>
#include <fstream>

Define_Module(Coordinator);
void Coordinator::initialize()
{

    std::ifstream filestream;
    std::string line;

    filestream.open("coordinator.txt", std::ifstream::in);

    if(!filestream)
    {
        throw cRuntimeError("Error opening file '%s'?", "coordinator.txt");

    }
    else
    {
        std::getline(filestream, line);
        EV<<"I am coordinator and I read : "<<line<<endl;
        MyMessage_Base *msg = new MyMessage_Base((char*)"coord");
        msg->setM_Payload(line.c_str());
        msg->setSeq_Num(-1);

        MyMessage_Base *msg2 = new MyMessage_Base((char*)"coord");
        msg2->setM_Payload(line.c_str());
        msg2->setSeq_Num(-1);

        send(msg,"out0");
        send(msg2,"out1");
    }

    //parseInputLine(line);

}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
