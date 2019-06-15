/* Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
#include "global-defines.h"

#include "client-model-container.h"
#include "uav-network.h"

using namespace ns3;

/*
	Scenario
	0- teste somente pontos fixos
	1- Austin City Limits
	2- Lollapalooza
	3- Rock in Rio Brasil
	4- Rock in Rio Lisboa
	5- Rock in Rio USA

	7- teste com clientes ao redor dos pontos fixos

	Environment
	1- High-rise Urban
	2- Dense Urban
	3- Urban
	4- Surburban
*/
std::string global_path;
double etapa;
double total_battery;
int global_nc;
double global_ec_persec;
double global_speed;
double global_uav_cob;
double global_cli_cob;
double global_tx_current;
double global_cli_cons_update;
// https://www.wired.com/story/the-physics-of-why-bigger-drones-can-fly-longer/
int main (int argc, char *argv[])
{
	double sim_time=1200.0;
	double totalCli = 0;
	uint32_t scenario = 7, env = 2, protocol = 1, custo=1, seed=9042019;
	global_cli_cons_update = 1.0;
	global_cli_cob = 115.47; // metros - para clientes utilizando equação de antena direcional com esparramento verificar Klaine2018
	global_uav_cob = 280.5; // metros verificar distancia_sinr.py
	global_tx_current = 0.0174; // ampere, valor padrao classe modulo wifi (antigo 0.0174)
	total_battery = 156960;
	CommandLine cmd;
	cmd.AddValue ("SimTime", "Simulation time", sim_time);
	cmd.AddValue ("Scenario", "Scenario", scenario);
	cmd.AddValue ("Env", "Environment", env);
	cmd.AddValue ("Protocol", "Routing Protocol", protocol);
	cmd.AddValue ("Custo", "Metrica de análise do DA de Posicionamento", custo);
	cmd.AddValue ("Seed", "Seed", seed);
	cmd.AddValue ("GlobalPath", "Global Path", global_path);
	cmd.AddValue ("Etapa", "", etapa);
	cmd.AddValue ("UavCob", "", global_uav_cob);
	cmd.AddValue ("CliCob", "", global_cli_cob);
	cmd.AddValue ("ClientUpdateCons", "", global_cli_cons_update);
	cmd.AddValue ("TotalBattery", "", total_battery);
	cmd.AddValue ("TotalCli", "", totalCli);
	cmd.Parse (argc, argv);

	global_ec_persec = total_battery/27*60; // bt /restime
	global_speed = 5.0; // m/s
	
	// LogComponentEnable("MyOnOffApplication", LOG_DEBUG);
	// LogComponentEnable("ServerApplication", LOG_DEBUG);
	// LogComponentEnable("LocationModel", LOG_DEBUG);
	// LogComponentEnable("UavApplication", LOG_FUNCTION);
	// LogComponentEnable("UavApplication", LOG_DEBUG);
	// LogComponentEnable("SmartphoneApplication", LOG_FUNCTION);
	// LogComponentEnable("WifiPhyStateHelper", LOG_FUNCTION);
	// LogComponentEnable("WifiPhy", LOG_FUNCTION);
	// LogComponentEnable ("WifiPhyStateHelper", LOG_DEBUG);
	// LogComponentEnable ("WifiRadioEnergyModel", LOG_DEBUG);


	// LogComponentEnable("DhcpClient", LOG_FUNCTION);
	// LogComponentEnable("ClientDeviceEnergyModelHelper", LOG_FUNCTION);
	// LogComponentEnable("ClientDeviceEnergyModel", LOG_ALL);
	// LogComponentEnable("ClientModel", LOG_FUNCTION);
	// LogComponentEnable("ClientModelContainer", LOG_FUNCTION);
	// LogComponentEnable("LocationModelContainer", LOG_FUNCTION);
	// LogComponentEnable("LocationModel", LOG_FUNCTION);
	// LogComponentEnable("ServerApplication", LOG_FUNCTION);
	// LogComponentEnable("SmartphoneApplication", LOG_FUNCTION);
	// LogComponentEnable("SmartphoneApplication", LOG_FUNCTION);
	// LogComponentEnable("UavApplicationContainer", LOG_FUNCTION);
	// LogComponentEnable("UavApplication", LOG_FUNCTION);
	// LogComponentEnable("UavDeviceEnergyModelHelper", LOG_FUNCTION);
	// LogComponentEnable("UavDeviceEnergyModel", LOG_DEBUG);
	// LogComponentEnable("UavEnergySourceHelper", LOG_FUNCTION);
	// LogComponentEnable("UavEnergySource", LOG_DEBUG);
	// LogComponentEnable("UavMobilityModel", LOG_DEBUG);
	// LogComponentEnable("UavModelContainer", LOG_FUNCTION);
	// LogComponentEnable("UavModel", LOG_DEBUG);
	// LogComponentEnable("UavNetwork", LOG_DEBUG);
	// LogComponentEnable("UavNodeContainer", LOG_FUNCTION);

	// SeedManager::SetSeed(6112018);
	SeedManager::SetSeed(seed);

	ObjectFactory obj;
	obj.SetTypeId("ns3::UavNetwork");
	obj.Set("SimulationTime", DoubleValue(sim_time));
	obj.Set("Scenario", UintegerValue(scenario));
	obj.Set("Environment", UintegerValue(env));
	obj.Set("Protocol", UintegerValue(protocol));
	obj.Set("Custo", UintegerValue(custo));
	obj.Set("Seed", UintegerValue(seed));
	obj.Set("ScheduleServer", DoubleValue(etapa));
	obj.Set("TotalCli", DoubleValue(totalCli));
	obj.Set("UavTimingNext", DoubleValue((sim_time-0.5)/10.0));
	obj.Set("ClientUpdateCons", DoubleValue(global_cli_cons_update));
	Ptr<UavNetwork> net = obj.Create()->GetObject<UavNetwork>();
	net->Run();
	net->Dispose();
}
