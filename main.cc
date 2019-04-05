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
#define ETAPA 300 // segundos

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

// https://www.wired.com/story/the-physics-of-why-bigger-drones-can-fly-longer/
int main (int argc, char *argv[])
{
	double sim_time=1200.0, cli_pos_update = 5.0, scheduleServer=ETAPA;
	uint32_t scenario = 7, env = 2, protocol = 2, custo=1;
	CommandLine cmd;
	cmd.AddValue ("SimTime", "Simulation time", sim_time);
	cmd.AddValue ("CliUpdate", "Client update position", cli_pos_update);
	cmd.AddValue ("Scenario", "Scenario", scenario);
	cmd.AddValue ("Env", "Environment", env);
	cmd.AddValue ("Protocol", "Routing Protocol", protocol);
	cmd.AddValue ("ScheduleServer", "Tempo minimo etapa", scheduleServer);
	cmd.AddValue ("Custo", "Metrica de análise do DA de Posicionamento", custo);
	cmd.Parse (argc, argv);

	LogComponentEnable("UavNetwork", LOG_DEBUG);
	LogComponentEnable("ServerApplication", LOG_DEBUG);
	// LogComponentEnable("LocationModel", LOG_DEBUG);
	// LogComponentEnable("UavApplication", LOG_FUNCTION);
	LogComponentEnable("UavApplication", LOG_DEBUG);
	// LogComponentEnable("SmartphoneApplication", LOG_DEBUG);
	// LogComponentEnable("WifiPhyStateHelper", LOG_FUNCTION);
	// LogComponentEnable("WifiPhy", LOG_FUNCTION);
	LogComponentEnable ("WifiPhyStateHelper", LOG_DEBUG);
	LogComponentEnable ("WifiRadioEnergyModel", LOG_DEBUG);


	// LogComponentEnable("DhcpClient", LOG_FUNCTION);
	// LogComponentEnable("ClientDeviceEnergyModelHelper", LOG_FUNCTION);
	// LogComponentEnable("ClientDeviceEnergyModel", LOG_FUNCTION);
	// LogComponentEnable("ClientModel", LOG_FUNCTION);
	// LogComponentEnable("ClientModelContainer", LOG_FUNCTION);
	// LogComponentEnable("LocationModelContainer", LOG_FUNCTION);
	// LogComponentEnable("LocationModel", LOG_FUNCTION);
	// LogComponentEnable("ServerApplication", LOG_FUNCTION);
	// LogComponentEnable("SmartphoneApplication", LOG_FUNCTION);
	// LogComponentEnable("SmartphoneApplication", LOG_DEBUG);
	// LogComponentEnable("UavApplicationContainer", LOG_FUNCTION);
	// LogComponentEnable("UavApplication", LOG_FUNCTION);
	// LogComponentEnable("UavDeviceEnergyModelHelper", LOG_FUNCTION);
	LogComponentEnable("UavDeviceEnergyModel", LOG_DEBUG);
	// LogComponentEnable("UavEnergySourceHelper", LOG_FUNCTION);
	LogComponentEnable("UavEnergySource", LOG_DEBUG);
	LogComponentEnable("UavMobilityModel", LOG_DEBUG);
	// LogComponentEnable("UavModelContainer", LOG_FUNCTION);
	// LogComponentEnable("UavModel", LOG_DEBUG);
	// LogComponentEnable("UavNetwork", LOG_FUNCTION);
	// LogComponentEnable("UavNodeContainer", LOG_FUNCTION);

	SeedManager::SetSeed(6112018);

	ObjectFactory obj;
	obj.SetTypeId("ns3::UavNetwork");
	obj.Set("SimulationTime", DoubleValue(sim_time));
	obj.Set("LocationUpdateCli", DoubleValue(cli_pos_update));
	obj.Set("Scenario", UintegerValue(scenario));
	obj.Set("Environment", UintegerValue(env));
	obj.Set("Protocol", UintegerValue(protocol));
	obj.Set("Custo", UintegerValue(custo));
	obj.Set("ScheduleServer", DoubleValue(scheduleServer));
	Ptr<UavNetwork> net = obj.Create()->GetObject<UavNetwork>();
	net->Run();
	net->Dispose();
}
