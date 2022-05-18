#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include <iostream>
#include "ns3/chat-header.h" 
#include <vector> 

int main (int argc, char *argv[])
{
  Packet::EnablePrinting ();
  // you can now remove the header from the packet:
  ChatHeader destinationHeader;
  std::cout << "RemoveHEader\n";
  p->RemoveHeader (destinationHeader);
  std::vector<uint32_t> dv;
  std::vector<uint32_t> dv1;
  dv = destinationHeader.GetData();
  std::cout << dv[0] << "\n";
  uint32_t act = dv[0];
  vector<vector<int> > chatroom(100);
  if(act==0){
      std::cout << "First connect" << "\n";
      dv1.push_back(0);
      dv1.push_back() //user id
  }
  else if(act==1){
      std::cout << "Message send" << "\n";
      dv1.push_back(1);
      dv1.push_back(dv[1]);
      dv1.push_back(dv[2]);
  }
  else{
      std::cout << "Make chat room" << "\n";
      dv1.push_back(2);
      dv1.push_back()
  }
  ChatHeader sourceHeader;
  sourceHeader.SetData (dv1);
  Ptr<Packet> p = Create<Packet> ();
  ChatHeader sourceHeader;
  sourceHeader.SetData (dv1);
  // instantiate a packet
  Ptr<Packet> p = Create<Packet> ();
 
  // and store my header into the packet.
  p->AddHeader (sourceHeader);
  
  // print the content of my packet on the standard output.
  p->Print (std::cout);
  std::cout << std::endl;

  return 0;
}
