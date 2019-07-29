
#include <string>

extern double global_ec_persec;
extern int global_nc; // total de cliente no cenario
extern double global_speed;
extern double global_uav_cob;
extern double global_cli_cob;
extern double global_tx_current; // ampere
extern double global_cli_cons_update;
extern int global_ksize;
extern double seed;

#ifndef GLOBAL_PATH
    extern std::string global_path;
    #define GLOBAL_PATH
#endif

#ifndef ETAPA
    extern double etapa;
    #define ETAPA 
#endif

#ifndef TOTAL_BATTERY
    extern double total_battery;
    #define TOTAL_BATTERY
#endif

#ifndef UDP
    #define UDP 
#endif


// #ifndef COM_SERVER
//     #define COM_SERVER
// #endif

// #ifndef DEV_WIFI
//    #define DEV_WIFI
// #endif

#ifndef DEV_CLI
    #define DEV_CLI
#endif

// #ifndef TCP
//     #define TCP 
// #endif

//#ifndef TCP_CLI
//   #define TCP_CLI 
//#endif



#ifndef PACKET_UAV_CLI
    #define PACKET_UAV_CLI
#endif

#ifndef DHCP
    #define DHCP
#endif

// #ifndef COMPARE_COST
//     #define COMPARE_COST
// #endif

// #ifndef LOG_CLIENT
//     #define LOG_CLIENT
// #endif

// #ifndef LOG_ACUM
//     #define LOG_ACUM
// #endif

// #ifndef LOG_ENERGY_ALL
//     #define LOG_ENERGY_ALL
// #endif

// #ifndef LOG_ENERGY
//     #define LOG_ENERGY
// #endif