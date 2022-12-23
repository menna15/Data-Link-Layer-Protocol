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
#include <fstream>
typedef std::bitset<8> bits;
#include "MyMessage_m.h"

using namespace omnetpp;

enum event_type
{
  network_layer_ready,
  frame_arrival,
  ack_arrival,
  nack_arrival,
  slide_back,
  send_ack,
  send_nack,
  timeout

};

/**
 * TODO - Generated class
 */
class Node : public cSimpleModule
{
  
  bool is_sender = false; // true if sender, false if receiver
  int start_transmission = -1; // start transmission time
  /*** parameters from omnetpp.ini ***/
  double PD; // processing delay
  double DD; // duplication delay
  double ED; // error delay
  double TD; // transmission delay
  int WS; // window size
  /*****************************/
  std::vector<std::string> errors_arr; // for storing the error codes for each frame
  std::vector<std::string> msg_arr;   // for storing the messages to be sent
  /****************************/
  int node_id; // to identify the node with 0 or 1 (used for printing the output)
  /*****time out variables*****/
  std::vector<int> timer_arr; // store the timeout interval for each frame
  std::vector<MyMessage_Base *> timer_msg_arr; // add timer_msg for each frame that carries the frame number, each has a message type = 3
  /****************************/
  // Sender variables
  int ack_expected;       // Lower edge of sender's window
  int next_frame_to_send; // Upper edge of sender's window + 1
  int nbuffered;          // number of frames sent till now
  int max_seq_number; // MAX_SEQ = 2^m - 1
  double last_send_time; // to prevent 2 frames from being sent at the same time
  int num_frames_scheduled; // num of frames that wanted to be sent at the same time
  std::vector<std::pair<std::string, std::string>> outbuffer;
  // Receiver variables
  int frame_expected; // Lower edge of receiver's window
  MyMessage_Base *received_frame; // for storing the received frame
  std::string prev_payload; // for storing the previous payload, to check for frames that have been received before
  // for output file
  std::ofstream output_file; 
  std::string output_file_name;

protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);

  bits ParityByteErrorDetection(std::string payload);
  std::string byteStuffing(std::string msg);

  void read_msgs();

  void receiveFrame(MyMessage_Base *msg);
  void sendFrame(int frame_num, int frame_type, int frame_expected);

  std::vector<std::string> splitLine(std::string line);
  bool between(int sf, int si, int sn);
  void inc(int &num);
  void toPhysicalLayer(MyMessage_Base *msg_to_send, std::string error_bits);
  void fromPhysicalLayer(MyMessage_Base *msg_received);
  bool fromNetworkLayer();
  void enableNetworkLayer();
  void disableNetworkLayer();
  void startTimer(int frame_num);
  void stopTimer(int frame_num);
};

#endif
