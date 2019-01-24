/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 * Authors: Giovanna Garcia <ggarciabas@gmail.com>
 */

#ifndef SERVER_APPLICATION
#define SERVER_APPLICATION

#include "ns3/core-module.h"
#include "ns3/traced-callback.h"
#include "ns3/wifi-module.h"
#include "ns3/packet-sink.h"

#include "location-model-container.h"
#include "client-model-container.h"
#include "uav-model-container.h"

#include "uav-model.h"
#include "location-model.h"
#include "client-model.h"

#include <string>
#include <vector>
#include <cmath>
#include <iomanip>      // std::setfill, std::setw


namespace ns3
{

/**
 * ServerApplication
 *
 */
class ServerApplication : public Application
{
public:
  void PhyTxTraceAdhoc (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower);
  void PhyRxErrorTraceAdhoc (std::string context, Ptr<const Packet> packet, double snr);
  void PhyRxOkTraceAdhoc (std::string context, Ptr<const Packet> packet, double snr, WifiMode mode, enum WifiPreamble preamble);

  static TypeId GetTypeId(void);
  ServerApplication();
  virtual ~ServerApplication();

  void Start();
  void AddNewUav (uint32_t id, Ipv4Address addrAdhoc, double totalEnergy, double energyCost, double totalBattery, Ptr<MobilityModel> mob);
  void AddSupplyUav (uint32_t id, Ipv4Address addrAdhoc, double totalEnergy, double energyCost, double totalBattery, Ptr<MobilityModel> mob);

  void TracedCallbackRxApp (Ptr<const Packet> packet, const Address & address);

  void AddNewFixedClient(string login, double x, double y);

private:
  void DoDispose();
  virtual void StartApplication(void);
  virtual void StopApplication(void);
  void Run();
  void ValidateUavPosition (); // utilizada para garantir que os UAVs esteja no posicionamento desejado
  void SendUavPacket(Ptr<UavModel> uav);
  void SendCentralPacket(Ptr<UavModel> uav);
  void runDAPython();
  void runDA();
  void runDAPuro();
  void runAgendamento(void);
  void CreateCentralLocation(void);
  double CalculateDistance(const std::vector<double> pos1, const std::vector<double> pos2);
  double CalculateDistanceCentral(const std::vector<double> pos);
  bool ValidateMijConvergency(vector<vector<double>> vec, vector<vector<double>> m_ij, unsigned siz);
  void ReplyUav(Ptr<UavModel> uav);
  void GraficoCenarioDa (double temp, int iter, Ptr<LocationModel> lCentral, double uav_cob, double r_max, double max_antena, double maxDrUav);
  void GraficoCenarioDaPuro (double temp, int iter, Ptr<LocationModel> lCentral, double raio_cob);

  bool MovimentoA();
  bool MovimentoB();
  void CentroDeMassa (Ptr<LocationModel>, Ptr<LocationModel> central, double);

  void AskClientData ();
  void SendAskClientPacket(Ptr<UavModel> uav);
  void ReplyAskCliData(Ptr<UavModel> uav);

  double CalculateCusto (Ptr<UavModel> uav, Ptr<LocationModel> loc, vector<double> central_pos);

  void PrintCusto (vector<vector<double>> custo, int print, bool before, vector<int> uav_ids, vector<int> loc_ids);
  void PrintMij (vector<vector<double>> m_ij, double temp, std::string nameFile, vector<int> uav_ids, vector<int> loc_ids);
  void PrintBij (vector<vector<double>> b_ij, int print, bool before, vector<int> uav_ids, vector<int> loc_ids);


  double WattsToDb (double w) {
    return 10*std::log10(w); // dB
  }

  double WattsToDbm (double w) {
    return 10*std::log10(w)+30; // dB
  }

  double dBmToWatts (double dbm) {
    return std::pow(10, (dbm-30)/10);
  }

  double dbTodBm (double db) {
    return db+30;
  }

  double dbmToDb (double dbm) {
    return dbm-30;
  }

  UavModelContainer   m_uavContainer;
  UavModelContainer   m_uavGoToCentral; // UAVs enviados para central
  LocationModelContainer m_locationContainer;
  ClientModelContainer          m_fixedClientContainer; // utilizando o container de clientes diretamente só mudando o 'login' destes 'nos'
  ClientModelContainer m_clientContainer;
  ClientModelContainer m_clientDaContainer;

  DataRate m_dataRate;
  double m_scheduleServer;
  EventId m_serverEvent;
  double m_rho;
  uint32_t m_environment;
  uint16_t m_serverPort;
  uint16_t m_cliPort;
  uint32_t m_custo;
  double m_maxx, m_maxy;
  double m_rmax;
  double m_locConsTotal; // total de consumo das localizacoes
  Ipv4Address m_address;

  TracedCallback<int, int> m_newUav;
  TracedCallback<int> m_removeUav;
  TracedCallback<int> m_printUavEnergy;
  TracedCallback<string> m_clientPosition;
  TracedCallback<std::string> m_packetTrace;
  std::string m_pathData;
  std::string m_scenarioName;

  int m_supplyPos;
  int m_totalCliGeral;
};

} // namespace ns3

#endif /* SERVER_APPLICATION */
