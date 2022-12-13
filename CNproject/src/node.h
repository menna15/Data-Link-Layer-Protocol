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

#ifndef __CNPROJECT_NODE_H_
#define __CNPROJECT_NODE_H_

#include <omnetpp.h>
#include <bitset>

typedef  std::bitset<8> bits;
#include "MyMessage_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  bool is_sender=false;
  bool allow_to_send=false;
  protected:
    virtual void initialize();
    virtual void receive_msg();
    virtual void read_msgs( std::vector<std::string>& errors_arr, std::vector<std::string>& msg_arr);
    virtual void send_msg();
    virtual  std::bitset<8> ParityByteErrorDetection(std::string payload);
    virtual void handleMessage(MyMessage_Base *msg);
    virtual std::string byteStuffing(std::string msg);

};

#endif
