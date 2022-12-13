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
#include <bitset>
#include <iostream>

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

void Node::initialize()
{
    // TODO - Generated method body
}

void Node::handleMessage(cMessage *msg)
{
    EV << "From Node" << msg->getName() << endl;
    std::cout<< ParityByteErrorDetection("hi")<< endl;
}


