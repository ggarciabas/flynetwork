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

#ifndef LOCATION_MODEL
#define LOCATION_MODEL

#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "location-model-container.h"

#include <vector>
#include <map>
#include <cmath>

using namespace std;

namespace ns3
{

class ClientModel;

/**
 * LocationModel
 */
class LocationModel : public Object
{
public:
  static TypeId GetTypeId(void);
  LocationModel();
  virtual ~LocationModel();

  void SetId(uint32_t);
  uint32_t GetId();

  void UpdateDistCli (double);

  void SetPosition(double, double);
  const std::vector<double> GetPosition();

  void SetPosition(double, double, double);
  void SetPositionPuro(double x, double y, double r_max);
  std::vector<double> GetPosition(double);
  std::vector<double> GetPositionA(double);

  double GetXPosition (double);
  double GetYPosition (double);

  std::string toString() {
    std::ostringstream os;
    os << "Id: " << m_id << "\n\tPos: (" << m_position.at(0) << "," << m_position.at(1)
      << ")\n\tConsumption: " << m_totalConsumption
      << "\n\tPunNeigh: " << m_punshNeigh
      << "\n\tWij: " << m_wij
      << "\n\tConnected: " << ((m_connected) ? "true" : "false") << std::endl;
    return os.str();
  }

  bool IsUsed();
  void SetUsed();

  double GetTotalConsumption ();
  void SetTotalConsumption (double);

  void SetTotalCli (int t);
  int GetTotalCli ();

  void IniciarMovimentoA();
  void IniciarMovimentoB();
  bool MovimentoA ();
  bool MovimentoB ();

  void SetPunishNeighboor (double);
  double GetPunishNeighboor ();

  double GetMaxDistClient ();

  void InitializeWij (double);
  void NewClient (double, double, double);
  void RemoveClient (double, double);
  double GetWij ();

  void SetTempPljci (double);

  void AddPljCi (Ptr<ClientModel>, double, double);
  double AddPljCiPuro (Ptr<ClientModel> ci, double Zci, double r_max);


  bool UpdatePunishNeighboor (double);
  void SetFather (Ptr<LocationModel> l, double, double, double);
  Ptr<LocationModel> GetFather ();

  void AddChild (Ptr<LocationModel> l, double r_max);
  void ClearChildList ();
  double GetChildListSize();
  LocationModelContainer GetChildList ();

  void SetConnected (bool);
  bool IsConnected ();

  bool ValidarCapacidade (double);

  void LimparMapaPljci ();

  double GetXPositionA ();
  double GetYPositionA ();
  double GetXPosition ();
  double GetYPosition ();

  void LimparHistorico ();

  void UpdatePosition (double mx, double my);

private:
  void DoDispose ();
  std::vector<double> m_positionA;
  std::vector<double> m_positionB;
  std::vector<std::vector<double> > m_historico; // verificar issue: https://github.com/ggarciabas/client/issues/10
  bool m_changePosition; // variavel para permitir trocar de posicao, verificar issue: https://github.com/ggarciabas/client/issues/10
  double m_tempPljci;
  double m_wij;
  double m_punshNeigh; // punicao de conexao com vizinho
  bool m_used;
  int m_totaCli;
  uint32_t m_id;
  std::vector<double> m_position;
  double m_totalConsumption;

  bool m_connected;
  double m_distFather;
  double m_maxDistCli;

  // l_j = \frac{\sum_{i=1}^{N_{pi}} p(c_i)p(l_j|c_i)c_i+\omega_j(l_n + \sum_{m>j} v_{mj}~l_m)}{\sum_{i=1}^{N_{pi}} p(l_j)+\omega_j+\omega_j\sum_{m>j} v_{mj}}~.
  // \omega_j+\omega_j\sum_{m>j} v_{mj} -> esta parte pode ser calculada multiplicando somente o m_punishNeigh pelo tamanho da lista de filhos + m_punishNeigh, lembrando que este ultimo é referente ao pai
  // double m_xAcum, m_xAcumCli; // posicao em X acumulada, sem normalização, parte inferior da equação acima
  // double m_yAcum, m_yAcumCli; // posicao em Y acumulada, sem normalização, parte inferior da equação acima
  // double m_plj; // acumulado da parte da equação referente a p(l_j)

  std::map<Ptr<ClientModel>, double> m_pljci;
  Ptr<LocationModel> m_father;
  LocationModelContainer m_childList;
};

} // namespace ns3

#endif /* LOCATION_MODEL */
