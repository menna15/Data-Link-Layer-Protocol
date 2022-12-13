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

void Node::handleMessage(MyMessage_Base *msg)
{
    EV << "From Node" << msg->getName() << endl;

}






