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
    // prepare timer message
   timer_msg_arr[frame_num]= new MyMessage_Base((char *)"timeout");
   timer_msg_arr[frame_num]->setM_Type(3); // timeout message
   timer_msg_arr[frame_num]->setSeq_Num(frame_num); // frame number for which timer is started
   scheduleAt(simTime()+ timer_arr[frame_num], timer_msg_arr[frame_num]); // schedule timer to trigger after timeout interval
}
//***********************************************************************************************
void Node::stopTimer(int frame_num){
    if (timer_msg_arr[frame_num] != NULL)
        if(timer_msg_arr[frame_num]->isScheduled()) // check if timer is scheduled
        {
            EV<<"Stopped timer for frame "<<frame_num<<endl; // if scheduled, cancel it
            cancelAndDelete(timer_msg_arr[frame_num]); // delete the timer message
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
    return (((sf <= si) && (si < sn)) || ((sn < sf) && (sf <= si)) || ((si < sn) && (sn < sf))|| sf == sn); // check if si is between sf and sn, used for accumulating acks
}

// **********************************************************************************************
void Node::enableNetworkLayer()
{
    EV<<"enableNetworkLayer"<<simTime()<<endl;
    EV<<"last_send_time"<<last_send_time<<endl;
    EV<<"num_frames_scheduled"<<num_frames_scheduled<<endl;
    // in case the network layer is enabled more than once int the same time slot, we need to send only the first frame and schedule the rest of the frames to be sent in the next time slot
    if(last_send_time != NULL && (simTime().dbl()+PD) <= (last_send_time +(num_frames_scheduled)*PD))
    {
        num_frames_scheduled++;

    }
    else
    {
        num_frames_scheduled = 1;
        last_send_time = simTime().dbl();
    }


    MyMessage_Base *msg_to_send = new MyMessage_Base();
    msg_to_send->setSeq_Num(-2); // -2 to enable the physical layer to know that this is a network layer message
    scheduleAt(last_send_time + num_frames_scheduled*PD, msg_to_send); // schedule the message to be sent after PD time
    
}
// **********************************************************************************************
void Node::disableNetworkLayer() {
    // do nothing
}
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
    double time = 0.0; // time to send the frame
    int bit_to_flip=-1; // bit to flip in case of bit error
    if(error_bits[0] == '1') // send corrupted, checked first to print it
        {
            std::string payload = msg_to_send->getM_Payload();

            // generate random bit to flip (starting from 8:(length -8) to avoid flipping the flag)
            bit_to_flip = int(uniform(8, (payload.length()-1)*8));
            std::cout<<"bit to flip: "<<bit_to_flip<<endl; //delete me
            int index_char_to_modify= bit_to_flip / 8; // index of the char to modify, as char is 8 bits
            char char_to_modify= payload[index_char_to_modify];  // char to modify
            std::bitset<8> binary_char(char_to_modify); // convert char to binary
            int bit_to_modify= bit_to_flip % 8; // bit to modify in the char
            binary_char.flip(bit_to_modify); // flip the bit
            payload[index_char_to_modify] = (char)binary_char.to_ulong(); // convert back to char
            msg_to_send->setM_Payload(payload.c_str()); // set the new payload
        }
    if (error_bits[1] != '1') // if not loss
    {
        if (error_bits == "0000") // no error
        {

            if(msg_to_send->getM_Type() == 1 || msg_to_send->getM_Type() == 2) // if control frame
            {
                int rand = uniform(0, 1)*10; // get the probability of sending ack/nack
                std::string ack_type; // ack or nack, used for printing
                if(msg_to_send->getM_Type() == 1) // if ack
                    ack_type = "ACK"; 
                else
                    ack_type = "NACK";
                EV<<"random generated is"<<rand<<endl; // delete me
                // send ack/ nack with probability 1-par("loss_prob").doubleValue()
                if(rand<((1-par("loss_prob").doubleValue())*10))
                {
                    output_file.open(output_file_name, std::ios_base::app);
                    output_file<<"At time["<<simTime()<<"], Node["<<node_id<<"] Sending ["<<ack_type<<"] with number ["<<msg_to_send->getACK_NACK()<<"] , loss [No]"<<endl;
                    output_file.close();
                    sendDelayed(msg_to_send, TD, "to_pair");
                    
                }
                else // don't send ack/ nack (loss)
                {
                    output_file.open(output_file_name, std::ios_base::app);
                    output_file<<"At time["<<simTime()<<"], Node["<<node_id<<"] Sending ["<<ack_type<<"] with number ["<<msg_to_send->getACK_NACK()<<"] , loss [Yes]"<<endl;
                    output_file.close();

                }
            }
            else // not ack/ nack
            {
                sendDelayed(msg_to_send, TD, "to_pair"); // send the frame after TD only

            }   
        }
        else
        {
            
            if (error_bits[3] == '1') // send Delayed
            {
                time += ED; // add ED to the time
            }

            if (error_bits[2] == '1') // send duplicated
            {
                double time_d = time + TD + DD; // time to send the duplicated frame 
                sendDelayed(msg_to_send->dup(), time_d, "to_pair");
            }
            
            time += TD;
            sendDelayed(msg_to_send, time, "to_pair");

        }
    }
    // output to file
    if(is_sender)
    {
        
        int modified_bit = -1;
        if(error_bits[0] == '1')
        {
            modified_bit = bit_to_flip-8;
        }
        std::string lost = "No";
        if(error_bits[1] == '1')
        {
            lost = "Yes";
        }
        bool is_dup = false;
        if(error_bits[2] == '1')
        {
            is_dup = true;
        }
        int delay = 0;
        if(error_bits[3] == '1')
        {
            delay = ED;
        }


        output_file.open(output_file_name, std::ios_base::app);
        output_file<<"At time ["<<simTime()<<
        "], Node["<<node_id<<"] [sent] frame with seq_num=["<<msg_to_send->getSeq_Num()<<"] and payload=["<<msg_to_send->getM_Payload()<<
        "] and trailer=["<<msg_to_send->getTrailer()<<"] , Modified ["<<modified_bit<<"] , Lost ["<<lost<<"], Duplicate ["<<is_dup<<"], Delay ["<<delay<<"]."
        <<endl;
        if(is_dup)
        {
            output_file<<"At time ["<<simTime()+DD<<
            "], Node["<<node_id<<"] [sent] frame with seq_num=["<<msg_to_send->getSeq_Num()<<"] and payload=["<<msg_to_send->getM_Payload()<<
            "] and trailer=["<<msg_to_send->getTrailer()<<"] , Modified ["<<modified_bit<<"] , Lost ["<<lost<<"], Duplicate [2], Delay ["<<delay<<"]."
            <<endl;
        }
        output_file.close();
    }
}


// ********************************** ERROR DETECTION ******************************************
std::bitset<8> Node::ParityByteErrorDetection(std::string payload)
{
    char c; // char to convert to binary
    std::bitset<8> result(0); // result of xor
    for (int i = 0; i < payload.length(); i++)
    {
        c = payload[i]; // get the char
        std::bitset<8> binary_char(c); // convert to binary
        result ^= binary_char; // xor with the result
    }
    return result; // return the result
}
// **************************** Framing *******************************************************
std::string Node::byteStuffing(std::string payload)
{
    std::string payload_after_stuffing = "$"; // start with $ and end with $ 
    for (int i = 0; i < payload.size(); i++) // loop on the payload
    {
        if (payload[i] == '$' || payload[i] == '/') // if $ or / add / before it
        { 
            payload_after_stuffing = payload_after_stuffing + "/"; 
        }
        payload_after_stuffing = payload_after_stuffing + payload[i]; // add the char to the payload
    }

    payload_after_stuffing = payload_after_stuffing + "$"; // add $ at the end

    return payload_after_stuffing; // return the payload
}
// ************************************Read msgs from the input file***********************************
void Node::read_msgs()
{
    std::string file_name = ""; 
    if (strcmp(getName(), "node0") == 0) // check the name of the node
    {
        file_name = "input0.txt"; // get the file name
    }
    else
    {
        file_name = "input1.txt"; // get the file name
    }

    EV << file_name << endl; // delete me

    std::ifstream filestream; 
    std::string line;
    filestream.open(file_name, std::ifstream::in); // open the file
    if (!filestream)
    {
        throw cRuntimeError("Error opening file '%s'?", "coordinator.txt");
    }
    else
    {
        while (getline(filestream, line)) // read the file line by line
        {
            std::string error = line.substr(0, 4); // get the error bits
            errors_arr.push_back(error); // add the error bits to the array
            std::string msg = line.substr(5); // get the message
            msg_arr.push_back(msg); // add the message to the array
        }
    }
}
// ************************************Sending Frame to the physical layer*******************************************
void Node::sendFrame(int frame_num, int frame_kind, int frame_expected)
{

    std::string payload; // the payload of the frame
    bits Trailer; // the trailer of the frame
    MyMessage_Base *msg_to_send = new MyMessage_Base(std::to_string(frame_num).c_str()); // create the frame
    if (frame_kind == 0 && is_sender) // if data, 
    {
        // getting the next message to send from the buffer
        int msg_index; // used to get the index of the message in the buffer
        if(frame_num<ack_expected){ // if the frame number is less than the ack expected, then it is a wrap around
            msg_index = (frame_num + WS - ack_expected); // so we add the window size to the frame number, to avoid the negative index
        }
        else{ // otherwise, the index is the frame number minus the ack expected
            msg_index = frame_num - ack_expected; 
        }
        EV << "index is : " << frame_num << endl; // delete me
        payload = byteStuffing(outbuffer[msg_index % WS].second); // frame the message
        EV << "index is : " << frame_num << " Payload after framing is :" << payload << endl; // delete me
        Trailer = ParityByteErrorDetection(payload); // add the trailer
        EV << "index is : " << frame_num << " Trailer is :" << Trailer << endl; // delete me

        msg_to_send->setM_Type(0); // set the type of the frame to data
        msg_to_send->setM_Payload(payload.c_str()); // set the payload of the frame
        msg_to_send->setTrailer(Trailer); // set the trailer of the frame
        msg_to_send->setSeq_Num(frame_num % WS); // set the sequence number of the frame
        toPhysicalLayer(msg_to_send, outbuffer[msg_index % WS].first); // send the frame to the physical layer
        startTimer(frame_num % WS);// start_timer for the frame
    }
    if (frame_kind == 1 && !is_sender) // if ack,
    {
        EV << "I am: " << getName() << " and currently sending ack of frame num:" << frame_expected << endl; // delete me
        msg_to_send->setM_Type(1); // set the type of the frame to ack
        msg_to_send->setACK_NACK(frame_expected); // set the ack number of the frame to the frame expected
        toPhysicalLayer(msg_to_send, "0000"); // send ack with no error
    }
    if (frame_kind == 2 && !is_sender) // if nack,
    {
        EV << "I am: " << getName() << " and currently sending nack of frame num:" << frame_expected << endl; // delete me
        msg_to_send->setM_Type(2); // set the type of the frame to nack
        msg_to_send->setACK_NACK(frame_expected); // set the nack number of the frame to the frame expected
        toPhysicalLayer(msg_to_send, "0000"); // send nack with no error
    }
    
   
}
// **********************************Fetch from the network layer*******************************************
bool Node::fromNetworkLayer()
{
    if (msg_arr.empty()) // no more messages to send
    {
        return false;
    }
    outbuffer.push_back(std::make_pair(errors_arr.front(), msg_arr.front())); // add the message to the buffer
    errors_arr.erase(errors_arr.begin()); // remove the error bits from the top of the array
    msg_arr.erase(msg_arr.begin()); // remove the message from the top of the array
    return true; // return true if there are more messages to send
}
// ********************************FOR BOTH SENDER AND RECIEVER********************************************
void Node::receiveFrame(MyMessage_Base *msg)
{

    event_type event; // the event that happened
    if (msg->getSeq_Num() == -2) // sender sends to itself to indicate that it is ready to send
    {
        EV<<"I am: "<<getName()<<" and I received network layer ready"<<endl;
        event = network_layer_ready;
    }
    else if(msg->getSeq_Num() == -3 && !is_sender) // reciever sends to it self to indicate that the PD is finished and it's ready to send ack
    {
        EV<<"I am: "<<getName()<<" and I received send_ack"<<endl;
        event= send_ack;
    }
    else if(msg->getSeq_Num() == -4 && !is_sender) // reciever sends to it self to indicate that the PD is finished and it's ready to send nack
    {
        EV<<"I am: "<<getName()<<" and I received send_nack"<<endl;
        event= send_nack;
    }
    else if(msg->getSeq_Num() == -3 && is_sender) // sender sends to itself to indicate that timeout has occurred and it has to slide back
    {
        EV<<"I am: "<<getName()<<" and I received slide_back"<<endl;
        event= slide_back;
    }
    else if (msg->getM_Type() == 0 && msg->getSeq_Num() != -2 && msg->getSeq_Num() != -1) // frame arrival to sender
    {
        EV<<"I am: "<<getName()<<" and I received frame arrival"<<endl;
        event = frame_arrival;
    }
    else if (msg->getM_Type() == 1 && is_sender == true) // ack arrival to sender
    {
        EV<<"I am: "<<getName()<<" and I received ack arrival"<<endl;
        event = ack_arrival;
    }
    
    else if(msg->getM_Type() == 3 && is_sender == true) // timeout to sender
    {   
        EV<<"I am: "<<getName()<<" and I received timeout"<<endl;
        event = timeout;
    }
    else // a request for retransmission for sender
    {
        event = nack_arrival;
    } 
    switch (event)
    {
    case network_layer_ready:
        if (nbuffered < WS && fromNetworkLayer()) // if there's a place available for more messages to be sent
        {
            nbuffered = nbuffered + 1; // increment the number of buffered messages 
            /**** getting the next message to send from the buffer ****/
            int msg_index; // used to get the index of the message in the buffer
            if(next_frame_to_send<ack_expected){ // if the frame number is less than the ack expected, then it is a wrap around
                msg_index = (next_frame_to_send + WS - ack_expected); // so we add the window size to the frame number, to avoid the negative index
            }
            else{ // otherwise, the index is the frame number minus the ack expected
                msg_index = next_frame_to_send - ack_expected; 
            }
            // print the message to the output file
            output_file.open(output_file_name, std::ios_base::app);
            output_file<<"At time ["<<simTime()-PD<<"], Node["<<node_id<<"] , Introducing channel error with code =["<<outbuffer[msg_index%WS].first<<"]"<<endl;
            output_file.close();
            // send the message to the physical layer
            sendFrame(next_frame_to_send, 0, frame_expected);
            inc(next_frame_to_send); // increment the next frame to send
        }
        break;

    case frame_arrival: // at reciever side
        fromPhysicalLayer(msg); // get the frame from the physical layer
        EV << "I am " << getName() << ",received message with sequence number : " << received_frame->getSeq_Num() << ", payload : " << received_frame->getM_Payload() << ", and Trailer: " << received_frame->getTrailer() << endl;
        if (received_frame->getSeq_Num() == frame_expected 
        && (received_frame->getTrailer() == ParityByteErrorDetection(received_frame->getM_Payload()))) // if it's the expected frame and it's error free
        {
            inc(frame_expected); // increment the expected frame
            prev_payload = received_frame->getM_Payload(); // save the payload of the frame
            MyMessage_Base *msg_to_send = new MyMessage_Base(); // create a new message to send to the sender
            msg_to_send->setSeq_Num(-3); // set the sequence number to -3 to indicate that it's a send_ack event
            msg_to_send->setACK_NACK(frame_expected); // set the ack number to the expected frame
            scheduleAt(simTime() + PD, msg_to_send); // schedule the message to be sent after the propagation delay
        }
        else if (received_frame->getSeq_Num() == (frame_expected-1+WS)%WS 
        && received_frame->getM_Payload() == prev_payload
        && (received_frame->getTrailer() == ParityByteErrorDetection(received_frame->getM_Payload()))) // if the previous frame arrived again
        {
            // resend its ack
            MyMessage_Base *msg_to_send = new MyMessage_Base();
            msg_to_send->setSeq_Num(-3); 
            msg_to_send->setACK_NACK(frame_expected);
            scheduleAt(simTime() + PD, msg_to_send);


        }
        else if( received_frame->getSeq_Num() == frame_expected 
        &&received_frame->getTrailer() != ParityByteErrorDetection(received_frame->getM_Payload())) // if the frame is the frame expected but it's corrupted, request retransmission
        {
            MyMessage_Base *msg_to_send = new MyMessage_Base(); 
               msg_to_send->setSeq_Num(-4); // to send nack
               msg_to_send->setACK_NACK(frame_expected); // set the nack number to the expected frame
               scheduleAt(simTime() + PD, msg_to_send); 
        }
        break;
    case send_ack: // at reciever side
        EV<<"sending ack:"<<msg->getACK_NACK()<<endl;
        sendFrame(msg->getSeq_Num(), 1, msg->getACK_NACK()); // send the ack
        break;
    case send_nack: // at reciever side
        sendFrame(msg->getSeq_Num(), 2, msg->getACK_NACK()); // send the nack
        break;
    case ack_arrival: // at sender side
        fromPhysicalLayer(msg); // get the ack from the physical layer
        EV << "I am " << getName() << " I received ack with number: " << msg->getACK_NACK() << endl;
        EV<<"ack_expected: "<<ack_expected<<endl; 
        EV<<"received_frame->getACK_NACK()-1: "<<((received_frame->getACK_NACK()-1)+WS)%WS<<endl;
        EV<<"next_frame_to_send: "<<next_frame_to_send<<endl;
        if (between(ack_expected, ((received_frame->getACK_NACK()-1)+WS)%WS, next_frame_to_send)) // accumaulation of acks
        {
            // stop_timer
            if(ack_expected==((received_frame->getACK_NACK()-1)+WS)%WS) // if it's exactly the ack expected, then stop the timer of the frame and increment the ack expected
                {
                    stopTimer(ack_expected);
                    inc(ack_expected);
                    nbuffered -= 1;
                    outbuffer.erase(outbuffer.begin()); // erase the frame from the buffer
                }
            else // otherwise, stop the timer of the acknowledget frame and all the frames before it in the window
            {
                while(ack_expected!= received_frame->getACK_NACK())
                {
                    stopTimer(ack_expected);
                    inc(ack_expected);
                    nbuffered -= 1;
                    outbuffer.erase(outbuffer.begin());
                }


            }
        }
        break;
    case timeout: // at sender side
        {
            output_file.open(output_file_name, std::ios_base::app);
            output_file<<"Time out event at time ["<<simTime()<<"], at Node["<<node_id<<"] for frame with seq_num=["<<msg->getSeq_Num()<<"]"<<endl;
            output_file.close();
            EV<<"Timeout of message with seq num:"<<msg->getSeq_Num()<<endl;
            next_frame_to_send = ack_expected;
            outbuffer[0].first = "0000"; // make the frame that caused the timeout error free
            int temp = ack_expected;
            // stop timers of timed out frame and all the frames after it in the window, and slide back to the timed out frame
            for (int i = 1; i <= nbuffered; i++)
            {
                stopTimer(temp);
                inc(temp);
                MyMessage_Base *msg_to_send = new MyMessage_Base();
                msg_to_send->setSeq_Num(-3);
                // slide back
                scheduleAt(simTime() + (i)*PD, msg_to_send);
            }
            
            break;
        }
    case slide_back: 
        EV<<"slide back"<<endl;
        sendFrame(next_frame_to_send, 0, frame_expected); // retransmit the frame
        inc(next_frame_to_send);
        break;
    case nack_arrival:
        EV << "I am " << getName() << " I received Nack with number: " << msg->getACK_NACK() << endl;
    default:
        EV << "Default of switch case" << endl;
    }

    if (nbuffered < WS && !msg_arr.empty() && is_sender ) // if there's more space in the window for more frames to be sent
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

    // when it is the first message from coordinator it will has sequence number -1
    if (received_msg->getSeq_Num() == -1)
    {
        std::string line = received_msg->getM_Payload();
        std::vector<std::string> result = splitLine(line);
        std::string node = result[0];
        std::string start_time = result[1];
        
        if ((strcmp(getName(), "node0") == 0 && strcmp(node.c_str(), "0") == 0) || (strcmp(getName(), "node1") == 0 && strcmp(node.c_str(), "1") == 0))// if this node is the sender
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
            scheduleAt(simTime() + start_transmission + PD, smsg); // schedule the message to be sent after time = start_transmission
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

    PD= par("processing_delay").doubleValue(); // processing delay
    WS = par("window_size").intValue(); // window size
    DD = par("duplication_delay").doubleValue(); // duplication delay
    ED = par("error_delay").doubleValue(); // error delay
    TD = par("transmission_delay").doubleValue(); // transmission delay
    max_seq_number = WS -1; // maximum sequence number in the window 
    ack_expected = 0; // the next frame expected at the sender
    next_frame_to_send = 0; // the next frame to be sent from the sender
    frame_expected = 0; // the next frame expected at the receiver
    nbuffered = 0;  // number of frames in the window
    output_file_name = "output.txt"; // output file name

    // initialize timer of each frame in the window
    for (int i = 0; i < WS; i++)
    {
        timer_msg_arr.push_back(new MyMessage_Base((char *)"timeout"));
        timer_arr.push_back(par("time_out").doubleValue());
    }
    
    // clear the output file at the beginning of simulation
    output_file.open(output_file_name);
    output_file<<"";
    output_file.close();
    if(strcmp(getName(),"node1")==0)
            node_id = 1;
        else
            node_id = 0;
    
}
