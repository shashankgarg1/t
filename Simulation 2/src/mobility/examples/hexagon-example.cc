/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#include <ns3/core-module.h>
#include <ns3/mobility-module.h>
#include <ns3/network-module.h>
#include <ns3/log.h>
#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("HexagonExample");

void 
PrintGnuplottableNodeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  //  uint32_t id = 0;
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
      outFile << pos.x << " " << pos.y << std::endl;
      // outFile << "set label \"" << id
      // 	      << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 0.3 lc rgb \"grey\" offset 0,0"
      // 	      << std::endl;
      // ++id;
    }
}


void 
PrintGnuplottableUhpaToFile (Ptr<UniformHexagonPositionAllocator> uhpa, std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  DoubleValue rho;
  uhpa->GetAttribute ("rho", rho);
  DoubleValue theta;
  uhpa->GetAttribute ("theta", theta);
  DoubleValue X;
  uhpa->GetAttribute ("X", X);
  DoubleValue Y;
  uhpa->GetAttribute ("Y", Y);
  
  outFile << "set object 1 polygon from \\\n";
  
  for (uint32_t vertexId = 0; vertexId < 6; ++vertexId)
    {
      // angle of the vertex w.r.t. y-axis
      double a = vertexId * (M_PI/3.0) + theta.Get ();
      double x =  - rho.Get () * sin (a) + X.Get ();
      double y =  rho.Get () * cos (a) + Y.Get ();
      outFile << x << ", " << y << " to \\\n";
    }
  // repeat vertex 0 to close polygon
  uint32_t vertexId = 0;
  double a = vertexId * (M_PI/3.0) + theta.Get ();
  double x =  - rho.Get () * sin (a) + X.Get ();
  double y =  rho.Get () * cos (a) + Y.Get ();
  outFile << x << ", " << y << std::endl;
}

int
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  // you can change default attributes of UniformHexagonPositionAllocator to get different hexagons
  // for example:
  // ./waf --run hexagon-example --command="%s --ns3::UniformHexagonPositionAllocator::X=2 
  // --ns3::UniformHexagonPositionAllocator::Y=-5 --ns3::UniformHexagonPositionAllocator::rho=0.01 
  // --ns3::UniformHexagonPositionAllocator::theta=-0.1"

  NodeContainer nodes;
  nodes.Create (10000);
  
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");  
  Ptr<UniformHexagonPositionAllocator> positionAlloc = CreateObject<UniformHexagonPositionAllocator> ();




  Ptr<Min2dDistancePositionAllocator> minDistPosAlloc = CreateObject<Min2dDistancePositionAllocator> ();
  minDistPosAlloc->SetPositionAllocator (positionAlloc);

  NodeContainer obstacles1;
  obstacles1.Create (3);
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (-0.5),
                                 "MinY", DoubleValue (-0.5),
                                 "DeltaX", DoubleValue (0.3),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.Install (obstacles1);
  minDistPosAlloc->AddNodesDistance (obstacles1, 0.1);

  NodeContainer obstacles2;
  obstacles2.Create (2);
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (-0.4),
                                 "MinY", DoubleValue (0.7),
                                 "DeltaX", DoubleValue (0.6),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.Install (obstacles2);
  minDistPosAlloc->AddNodesDistance (obstacles2, 0.15);

  minDistPosAlloc->AddPositionDistance (Vector (0.4, -0.3, 0), 0.3);


  mobility.SetPositionAllocator (minDistPosAlloc);
  mobility.Install (nodes);

  PrintGnuplottableNodeListToFile ("nodes.dat");
  positionAlloc->PrintToGnuplotFile ("hexagon.gnuplot");

  // the following gnuplot commands will get you a nice plot:
  //
  // load "hexagon.gnuplot"
  // plot "nodes.dat" with points pt 0 ps 2;

}
