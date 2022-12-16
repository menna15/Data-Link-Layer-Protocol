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

typedef std::bitset<8> bits;
#include "MyMessage_m.h"

using namespace omnetpp;

enum event_type
{
  network_layer_ready,
  frame_arrival,
  ack_arrival,
  cksum_err,
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
  bool is_sender = false;
  bool allow_to_send = false;
  int start_transmission = -1; // start transmission time
  std::vector<std::string> errors_arr;
  std::vector<std::string> msg_arr;

  // add timer for each frame
  std::vector<int> timer_arr;
  // add timer_msg for each frame
  std::vector<MyMessage_Base *> timer_msg_arr;

  // Sender variables
  int ack_expected;       // Lower edge of sender's window
  int next_frame_to_send; // Upper edge of sender's window + 1
  int nbuffered;          // number of frames sent till now
  std::vector<std::pair<std::string, std::string>> outbuffer;

  // Receiver variables
  int frame_expected; // Lower edge of receiver's window
  MyMessage_Base *received_frame;

  int WS;             // window size
  int max_seq_number; // MAX_SEQ = 2^m - 1
  int num_frames_scheduled;
  double last_send_time;

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
  template <size_t N1, size_t N2 >
  std::bitset<N1 + N2> concat( const std::bitset <N1> & b1, const std::bitset <N2> & b2 );
};

#endif
