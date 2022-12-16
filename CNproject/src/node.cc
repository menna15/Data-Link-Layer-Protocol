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
#include "MyMessage_m.h"
#include <bitset>
typedef std::bitset<8> bits;

Define_Module(Node);

//***********************************************************************************************
void Node::startTimer(int frame_num){
    EV<<"Timer started for frame: "<<frame_num<<endl;
   timer_msg_arr[frame_num]= new MyMessage_Base((char *)"timeout");

   timer_msg_arr[frame_num]->setM_Type(3);
   timer_msg_arr[frame_num]->setSeq_Num(frame_num);
   scheduleAt(simTime()+ timer_arr[frame_num], timer_msg_arr[frame_num]);
    
}
//***********************************************************************************************
void Node::stopTimer(int frame_num){
    // chekc if timer is running
    if (timer_msg_arr[frame_num] != NULL)
        if(timer_msg_arr[frame_num]->isScheduled())
        {
            EV<<"Stopped timer for frame "<<frame_num<<endl;

            cancelAndDelete(timer_msg_arr[frame_num]);
        }
    
}
//***********************************************************************************************
// circular increment
void Node::inc(int &num)
{
    if (num < max_seq_number)
        num = num + 1;
    else
        num = 0;
}
// **********************************************************************************************
//                                        Between
// **********************************************************************************************
bool Node::between(int sf, int si, int sn)
{
    return (((sf <= si) && (si < sn)) || ((sn < sf) && (sf <= si)) || ((si < sn) && (sn < sf)));
}

// **********************************************************************************************
void Node::enableNetworkLayer()
{
    EV<<"enableNetworkLayer"<<simTime()<<endl;
    EV<<"last_send_time"<<last_send_time<<endl;
    EV<<"num_frames_scheduled"<<num_frames_scheduled<<endl;
    if(last_send_time != NULL && (simTime().dbl()+par("processing_delay").doubleValue()) <= (last_send_time +(num_frames_scheduled)*par("processing_delay").doubleValue()))
    {
        num_frames_scheduled++;

    }
    else
    {
        num_frames_scheduled = 1;
        last_send_time = simTime().dbl();
    }
    MyMessage_Base *msg_to_send = new MyMessage_Base();
    msg_to_send->setSeq_Num(-2);
    //    EV<<"enableNetworkLayer"<<simTime()<<endl;
    scheduleAt(last_send_time + num_frames_scheduled*par("processing_delay").doubleValue(), msg_to_send);
}
// **********************************************************************************************
void Node::disableNetworkLayer() {}
// **********************************************************************************************
void Node::fromPhysicalLayer(MyMessage_Base *msg_received)
{
    received_frame = msg_received->dup();
}
// **********************************************************************************************
//                                    To Physical Layer
// **********************************************************************************************
void Node::toPhysicalLayer(MyMessage_Base *msg_to_send, std::string error_bits)
{
    // this happens at both pairs for any frame to be sent
    double PD = par("processing_delay").doubleValue();
    double DD = par("duplication_delay").doubleValue();
    double ED = par("error_delay").doubleValue();
    double TD = par("transmission_delay").doubleValue();
    double time = 0.0;
    if (error_bits[1] != '1') // if not loss
    {
        if (error_bits == "0000") // no error
        {
            sendDelayed(msg_to_send, TD, "to_pair");
        }
        else
        {
            if(error_bits[0] == '1') // send corrupted
            {
                std::string payload = msg_to_send->getM_Payload();

                 std::cout<<"meassage before modification"<<payload<<endl;
                // generate random bit to flip
                int bit_to_flip = int(uniform(8, (payload.length()-2)*8));
                std::cout<<"bit to flip"<<bit_to_flip<<endl;
                int index_char_to_modify= bit_to_flip / 8;
                char char_to_modify= payload[index_char_to_modify];
                std::cout<<"char_to_modify:"<<char_to_modify<<endl;

                std::bitset<8> binary_char(char_to_modify);
                int bit_to_modify= bit_to_flip % 8;
                std::cout<<"binary_char before:"<<binary_char<<endl;

                binary_char.flip(bit_to_modify);
                std::cout<<"binary_char after:"<<binary_char<<endl;
                std::cout<<"binary_char after:"<<(char)binary_char.to_ulong()<<endl;

                payload[index_char_to_modify] = (char)binary_char.to_ulong();
                
                std::cout<<"meassage after modification"<<payload<<endl;

                msg_to_send->setM_Payload(payload.c_str());
                
            }
            if (error_bits[3] == '1') // send Delayed
            {
                time += ED;
            }

            if (error_bits[2] == '1') // send duplicated
            {
                double time_d = time + TD + DD;
                sendDelayed(msg_to_send->dup(), time_d, "to_pair");
            }
            
            time += TD;
            sendDelayed(msg_to_send, time, "to_pair");

        }
    }
}
template <size_t N1, size_t N2 >
std::bitset<N1 + N2> Node::concat( const std::bitset <N1> & b1, const std::bitset <N2> & b2 ) {
    std::string s1 = b1.to_string();
    std::string s2 = b2.to_string();
    return std::bitset <N1 + N2>( s1 + s2 );
}


// **********************************************************************************************
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
// **********************************************************************************************
std::string Node::byteStuffing(std::string payload)
{
    std::string payload_after_stuffing = "$";
    for (int i = 0; i < payload.size(); i++)
    {
        if (payload[i] == '$' || payload[i] == '/')
        {
            payload_after_stuffing = payload_after_stuffing + "/";
        }
        payload_after_stuffing = payload_after_stuffing + payload[i];
    }

    payload_after_stuffing = payload_after_stuffing + "$";

    std::cout << " the Message after applying byte stuffing is " << payload_after_stuffing << endl;
    return payload_after_stuffing;
}
// **********************************************************************************************
void Node::read_msgs()
{
    std::string file_name = "";
    if (strcmp(getName(), "node0") == 0)
    {
        file_name = "input0.txt";
    }
    else
    {
        file_name = "input1.txt";
    }

    EV << file_name << endl;

    std::ifstream filestream;
    std::string line;
    filestream.open(file_name, std::ifstream::in);
    if (!filestream)
    {
        throw cRuntimeError("Error opening file '%s'?", "coordinator.txt");
    }
    else
    {

        while (getline(filestream, line))
        {
            std::string error = line.substr(0, 4);
            errors_arr.push_back(error);
            std::string msg = line.substr(5);
            msg_arr.push_back(msg);
        }
    }
}
// **********************************************************************************************
void Node::sendFrame(int frame_num, int frame_kind, int frame_expected)
{

    std::string payload;
    bits Trailer;
    MyMessage_Base *msg_to_send = new MyMessage_Base(std::to_string(frame_num).c_str());
    if (frame_kind == 0 && is_sender) // if data,
    {
        int msg_index;
        if(frame_num<ack_expected){
            msg_index = (frame_num + WS - ack_expected);
        }
        else{
            msg_index = frame_num - ack_expected;
        }
        EV << "index is : " << frame_num << endl;
        payload = byteStuffing(outbuffer[msg_index % WS].second);
        EV << "index is : " << frame_num << " Payload after framing is :" << payload << endl;
        Trailer = ParityByteErrorDetection(payload);
        EV << "index is : " << frame_num << " Trailer is :" << Trailer << endl;

        msg_to_send->setM_Type(0);
        msg_to_send->setM_Payload(payload.c_str());
        msg_to_send->setTrailer(Trailer);
        msg_to_send->setSeq_Num(frame_num % WS);

        toPhysicalLayer(msg_to_send, outbuffer[msg_index % WS].first);
        // start_timer
        startTimer(frame_num % WS);
    }
    if (frame_kind == 1 && !is_sender) // if ack,
    {

        EV << "I am: " << getName() << " and currently sending ack of frame num:" << frame_expected << endl;

        msg_to_send->setM_Type(1);
        msg_to_send->setACK_NACK(frame_expected);

        toPhysicalLayer(msg_to_send, "0000"); // send ack with no error
    }
    if (frame_kind == 2 && !is_sender) // if nack,
    {

        EV << "I am: " << getName() << " and currently sending nack of frame num:" << frame_expected << endl;

        msg_to_send->setM_Type(2);
        msg_to_send->setACK_NACK(frame_expected);

        toPhysicalLayer(msg_to_send, "0000"); // send nack with no error
    }
    
   
}
// **********************************************************************************************
bool Node::fromNetworkLayer()
{
    if (msg_arr.empty())
    {
        return false;
    }
    //    EV<<"entered"<<endl;
    //    std::cout<<"entered"<<endl;
    //    std::cout<<errors_arr.front()<<endl;
    //    std::cout<<msg_arr.front()<<endl;
    outbuffer.push_back(std::make_pair(errors_arr.front(), msg_arr.front()));
    
    errors_arr.erase(errors_arr.begin());
    msg_arr.erase(msg_arr.begin());
    return true;
}
// **********************************************************************************************
void Node::receiveFrame(MyMessage_Base *msg)
{

    event_type event;
    if (msg->getSeq_Num() == -2)
    {
        EV<<"I am: "<<getName()<<" and I received network layer ready"<<endl;
        event = network_layer_ready;
    }
    else if(msg->getSeq_Num() == -3 && !is_sender)
    {
        EV<<"I am: "<<getName()<<" and I received send_ack"<<endl;
        event= send_ack;
    }
    else if(msg->getSeq_Num() == -4 && !is_sender)
    {
        EV<<"I am: "<<getName()<<" and I received send_nack"<<endl;
        event= send_nack;
    }
    else if(msg->getSeq_Num() == -3 && is_sender)
    {
        EV<<"I am: "<<getName()<<" and I received slide_back"<<endl;
        event= slide_back;
    }
    else if (msg->getM_Type() == 0 && msg->getSeq_Num() != -2 && msg->getSeq_Num() != -1)
    {
        EV<<"I am: "<<getName()<<" and I received frame arrival"<<endl;
        event = frame_arrival;
    }
    else if (msg->getM_Type() == 1 && is_sender == true)
    {
        EV<<"I am: "<<getName()<<" and I received ack arrival"<<endl;
        event = ack_arrival;
    }
    
    else if(msg->getM_Type() == 3 && is_sender == true)
    {   
        EV<<"I am: "<<getName()<<" and I received timeout"<<endl;
        event = timeout;
    }
    else
    {
        EV<<"I am: "<<getName()<<" and I received cksum_err"<<endl;
        event = cksum_err;
    } 
    switch (event)
    {
    case network_layer_ready:
        if (nbuffered < WS && fromNetworkLayer())
        {
            nbuffered = nbuffered + 1;
            sendFrame(next_frame_to_send, 0, frame_expected);
            inc(next_frame_to_send);
        }
        break;

    case frame_arrival:
        fromPhysicalLayer(msg);
        EV << "I am " << getName() << ",received message with sequence number : " << received_frame->getSeq_Num() << ", payload : " << received_frame->getM_Payload() << ", and Trailer: " << received_frame->getTrailer() << endl;
        if (received_frame->getSeq_Num() == frame_expected && (received_frame->getTrailer() == ParityByteErrorDetection(received_frame->getM_Payload())))
        {
            EV<<"matched"<<endl;
            inc(frame_expected);
            MyMessage_Base *msg_to_send = new MyMessage_Base();
            msg_to_send->setSeq_Num(-3);
            msg_to_send->setACK_NACK(frame_expected);
            scheduleAt(simTime() + par("processing_delay").doubleValue(), msg_to_send);

            //sendFrame(received_frame->getSeq_Num(), 1, frame_expected); // send ack
            // still not handle if this message is a duplicated one
        }
        else
        {
           MyMessage_Base *msg_to_send = new MyMessage_Base();
            msg_to_send->setSeq_Num(-4);
            msg_to_send->setACK_NACK(frame_expected);

            scheduleAt(simTime() + par("processing_delay").doubleValue(), msg_to_send);
            //sendFrame(received_frame->getSeq_Num(), 2, frame_expected); // send nack
        }
        break;
    case send_ack:
        EV<<"sending ack:"<<msg->getACK_NACK()<<endl;
        sendFrame(msg->getSeq_Num(), 1, msg->getACK_NACK());
        break;
    case send_nack:
        sendFrame(msg->getSeq_Num(), 2, msg->getACK_NACK());
        break;
    case ack_arrival:
        fromPhysicalLayer(msg);
        EV << "I am " << getName() << " I received ack with number: " << msg->getACK_NACK() << endl;
        if (received_frame->getACK_NACK() == (ack_expected+1)%WS)
        {
            // stop_timer
            stopTimer(ack_expected);
            inc(ack_expected);
            nbuffered -= 1;
            outbuffer.erase(outbuffer.begin());
        }
        break;
    case timeout:
        // stop timers of all frames in window
        EV<<"Timeout of message with seq num:"<<msg->getSeq_Num()<<endl;
        next_frame_to_send = ack_expected;
        outbuffer[next_frame_to_send].first = "0000";

        for (int i = 1; i <= nbuffered; i++)
        {
            stopTimer(i);
            MyMessage_Base *msg_to_send = new MyMessage_Base();
            msg_to_send->setSeq_Num(-3);
            //    EV<<"enableNetworkLayer"<<simTime()<<endl;
            scheduleAt(simTime() + (i)*par("processing_delay").doubleValue(), msg_to_send);
        }
           
        break;
    case slide_back:
        EV<<"slide back"<<endl;
        sendFrame(next_frame_to_send, 0, frame_expected);
        inc(next_frame_to_send);
        break;

    default:
        EV << "Default of switch case" << endl;
    }

    if (nbuffered < WS && !msg_arr.empty() && is_sender )
    {
        enableNetworkLayer();
    }
    else
    {
        disableNetworkLayer();
    }
}
// **********************************************************************************************
// will split the line to vector of strings --> [node number, start time ] and return the vector
// **********************************************************************************************
std::vector<std::string> Node::splitLine(std::string line)
{
    std::vector<std::string> result;
    std::istringstream string_stream(line);
    for (std::string word; string_stream >> word;)
        result.push_back(word);
    return result;
}
// **********************************************************************************************
void Node::handleMessage(cMessage *msg)
{
    MyMessage_Base *received_msg = check_and_cast<MyMessage_Base *>(msg);

    // wait until start time
    //    if(received_msg->getSeq_Num() == -2)
    //    {
    //        // when get self message in Time then allow to send if it is the sender
    //        // frame num = 0, frame type = 0 , frame expected = 0
    //        sendFrame(0,0,0);
    //    }
    // when it is the first message from coordinator it will has sequence number -1
    if (received_msg->getSeq_Num() == -1)
    {
        std::string line = received_msg->getM_Payload();
        std::vector<std::string> result = splitLine(line);
        std::string node = result[0];
        std::string start_time = result[1];

        if ((strcmp(getName(), "node0") == 0 && strcmp(node.c_str(), "0") == 0) || (strcmp(getName(), "node1") == 0 && strcmp(node.c_str(), "1") == 0))
        {
            EV << getName() << " Sender " << endl;
            is_sender = true;

            // set the start_transmission time
            start_transmission = std::stod(start_time.c_str());
            last_send_time = start_transmission;
            num_frames_scheduled = 0;
            // read the message from the sender input file:
            read_msgs();

            // sender node will send message to itself after time = start_transmission to indicate the start of transmission.
            // the message will has sequence number -2 to indicate that this message from sender to itself.
            MyMessage_Base *smsg = new MyMessage_Base((char *)"network layer ready ..");
            smsg->setSeq_Num(-2);
            scheduleAt(simTime() + start_transmission + par("processing_delay").doubleValue(), smsg);
        }
        else
        {
            EV << getName() << " Receiver " << endl;
        }
    }
    else // message is not from coordinator
    {
        receiveFrame(received_msg);
    }
    if (msg->isSelfMessage())
        cancelAndDelete(msg);
    else
        delete msg;
}
// **********************************************************************************************
void Node::initialize()
{
    WS = par("window_size").intValue();
    std::cout<<"WS:"<<WS<<endl;
    max_seq_number = WS -1;
    ack_expected = 0;
    next_frame_to_send = 0;
    frame_expected = 0;
    nbuffered = 0;
    
    // initialize timer of each frame in the window
    for (int i = 0; i < WS; i++)
    {
        timer_msg_arr.push_back(new MyMessage_Base((char *)"timeout"));
        timer_arr.push_back(par("time_out").doubleValue());
    }
    std::cout<<"times length:"<<timer_msg_arr.size()<<endl;

    // TODO - Generated method body
}
