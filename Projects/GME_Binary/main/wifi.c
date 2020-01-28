/*
 * wifi.c
 *
 *  Created on: Jun 17, 2019
 *      Author: ataayoub
 *
 *      FW Ver 90.00.00
 */


#include"wifi.h"
#include "nvm_CAREL.h"
#include <tcpip_adapter.h>
#include"http_server_IS.h"
#include "utilities_CAREL.h"
#include "MQTT_Interface_CAREL.h"
#include "sys_IS.h"

static const char *TAG = "wifi";

static uint32_t STAConnectionTime = 0;
static connection_status_t STAStatus = DISCONNECTED;

EventGroupHandle_t s_wifi_event_group;
const int CONNECTED_BIT = BIT0;
int i=0;



//Debugging
static int test2=0;
html_config_param_t test_config;


//Variables
//static TaskHandle_t CLI__CmdLine = NULL;				// if we want the console...esp32 library...?!?!

static config_sm_t config_sm = CHECK_FOR_CONFIG;
static httpd_handle_t AP_http_server = NULL;
static uint8_t wifi_conf;

static html_config_param_t WiFiConfig = {
		.gateway_mode = 0,
		.ap_ssid = AP_DEF_SSID,
		.ap_pswd = AP_DEF_PSSWD,
		.ap_ssid_hidden = AP_DEF_SSID_HIDDEN,
		.ap_ip = AP_DEF_IP,
		.ap_dhcp_mode = AP_DEF_DHCP,
		.ap_dhcp_ip = AP_DEF_DHCP_BASE,
		.sta_ssid = "",
		.sta_encryption = "",
		.sta_pswd= "",
		.sta_dhcp_mode = 1,
		.sta_static_ip = "",
		.sta_netmask = "",
		.sta_gateway_ip = "",
		.sta_primary_dns = "",
		.sta_secondary_dns = "",
		.ntp_server_addr = NTP_DEFAULT_SERVER,
		.ntp_server_port = "123",//NTP_DEFAULT_PORT,
		.mqtt_server_addr = MQTT_DEFAULT_BROKER,
		.mqtt_server_port = "8883",//MQTT_DEFAULT_PORT,
};

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {

//AP MODE

    case SYSTEM_EVENT_AP_STAIPASSIGNED:
    	ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STAIPASSIGNED");

        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);


        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
    	ESP_LOGI(TAG, "SYSTEM_EVENT_AP_STADISCONNECTED");
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);


        break;


//STA MODE


    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "Got IP: '%s'",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);

        WIFI__SetSTAStatus(CONNECTED);

        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
    	ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        ESP_ERROR_CHECK(esp_wifi_connect());
        GME__CheckHTMLConfig();

        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
        WIFI__SetSTAStatus(DISCONNECTED);
        break;


    default:
    	ESP_LOGI(TAG, "event; %d\n",event->event_id);
        break;
    }
    return ESP_OK;
}


/**
 * @brief WiFi_GetConfigSM
 *       return state machine config status
 *
 * @param  none
 * @return C_RES
 */
config_sm_t WiFi_GetConfigSM(void){
    return config_sm;
}

/**
 * @brief WiFi_SetConfigSM
 *      set state machine config status
 *
 * @param  none
 * @return C_RES
 */
void WiFi_SetConfigSM(config_sm_t config_state){
    config_sm = config_state;
}

/**
 * @brief WiFi__Config
 *   Configure the WiFi interface:
 * @Param:
 * 		sm: state variable
 * @return:
 * 		gme_sm_t: SM__Start state
 *
 * @Description:
 * 	Situation 1:
 * 	Starts with checking the NVM if exists old configuration. If it finds old config, it will read them from the NVM
 * 	and configure the system directly, starts the wifi then run the whole system in base of the configuration found.
 *
 *	Situation 2:
 * 	If it finds nothing, it will start the http_server and the wifi with the default configuration (check gme_config.h),
 * 	then attends for the html configuration. After receiving them, it writes them in NVM and does a reboot. After the reboot,
 * 	the system will be found in situation 1.
 *
 * 	Note: 	1) The HTTP_Server is launched as Task, so it can be accessed after the configuration, during the system running, in case
 * 				we wanted to re-configure the wifi.
 * 			2) In case of the gateway is configured as AP Mode, the varies functionalities don't start and the system will attend the
 * 				new configuration.
 *
 *
 * */


gme_sm_t WiFi__Config (config_sm_t sm)
{
    config_sm = sm;
    while(1)
    {
        switch(config_sm)
        {
        case CHECK_FOR_CONFIG:
            printf("WiFi__Config .... CHECK_FOR_CONFIG\n");
            /*Check in NVM*/

            if(C_SUCCESS == NVM__ReadU8Value("wifi_conf", &wifi_conf) && CONFIGURED == wifi_conf){
            	WiFi__ReadCustomConfigFromNVM();
                config_sm = CONFIGURE_GME;
            }else{
                config_sm = SET_DEFAULT_CONFIG;
            }
            break;

        case SET_DEFAULT_CONFIG:
            printf("WiFi__Config .... SET_DEFAULT_CONFIG\n");
            if(WiFi__SetDefaultConfig()){
                config_sm = START_WIFI;
            }
            printf("WiFi__Config .... SET_DEFAULT_CONFIG  END\n");
            break;

        case START_WIFI:
            printf("WiFi__Config .... START_WIFI\n");
            if (WiFi__StartWiFi()){
                ESP_ERROR_CHECK(HTTPServer__StartFileServer(&AP_http_server, "/spiffs"));

                if(CONFIGURED == wifi_conf && APSTA_MODE == WiFi__GetCustomConfig().gateway_mode){
        			return GME_WAITING_FOR_INTERNET;
                }else{
                    config_sm = WAITING_FOR_HTML_CONF_PARAMETERS;
                }
            }
            break;

        case WAITING_FOR_HTML_CONF_PARAMETERS:
            if(test2 == 0){
            	printf("\nGateway Mode = %d, Wifi Conf has %d config\n\n",WiFi__GetCustomConfig().gateway_mode,wifi_conf);
            	printf("WiFi__Config .... WAITING_FOR_HTML_CONF_PARAMETERS\n");
            	test2=10;
            }
            if(IsConfigReceived()){
            	printf("Configuration Received");
            	WiFi__WriteCustomConfigInNVM(HTTPServer__GetCustomConfig());

                if(C_SUCCESS == NVM__WriteU8Value("wifi_conf", (uint8_t)CONFIGURED)){
                	config_sm = CONFIGURE_GME;
                }
                if(wifi_conf == CONFIGURED){			//for future implementation
                	wifi_conf = TO_RECONFIGURE;
                }
            }
            break;

        case CONFIGURE_GME:
            printf("WiFi__Config .... CONFIGURE_GME\n");
            if(CONFIGURED == wifi_conf){

            	WiFi__SetCustomConfig(WiFi__GetCustomConfig());
                config_sm = START_WIFI;
            }else{
                return GME_REBOOT;
            }
            break;


        default:
            break;
        }

        //If the factory reset button has been pressed for X time (look gme_config.h)
        if(true == Sys__ResetCheck())
        {
        	printf("RESET CHECK DONE SYS\n");
        	return GME_REBOOT;
        }
    }
}



static void SetAPConfig(const char* ip, const char* gw, const char* netmask){

	tcpip_adapter_ip_info_t ap_ip;
	ap_ip.ip.addr = ipaddr_addr(ip);
	ap_ip.gw.addr = ipaddr_addr(gw);
	ap_ip.netmask.addr = ipaddr_addr(netmask);
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP));
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP,&ap_ip));
	ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP));
}


static void SetSTAStaticIP(const char* ip, const char* gw, const char* netmask, const char* pri_dns, const char* sec_dns)
{
	tcpip_adapter_ip_info_t sta_ip;
	sta_ip.ip.addr = ipaddr_addr(ip);
	tcpip_adapter_dns_info_t sta_primary_dns;
	tcpip_adapter_dns_info_t sta_secondary_dns;

	sta_ip.gw.addr = ipaddr_addr(gw);
	sta_ip.netmask.addr = ipaddr_addr(netmask);
	ESP_ERROR_CHECK(tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_STA));
	ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_STA,&sta_ip));


	sta_primary_dns.ip.u_addr.ip4.addr = ipaddr_addr(pri_dns);
	sta_secondary_dns.ip.u_addr.ip4.addr = ipaddr_addr(sec_dns);
	ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_MAIN, &sta_primary_dns));
	ESP_ERROR_CHECK(tcpip_adapter_set_dns_info(TCPIP_ADAPTER_IF_STA, TCPIP_ADAPTER_DNS_BACKUP, &sta_secondary_dns));
}

int WiFi__SetDefaultConfig(void)
{
	char* ap_ssid_def;
    s_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();

	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    SetAPConfig(AP_DEF_IP, AP_DEF_GW, AP_DEF_NETMASK);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();


    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    Utilities__Init();

    ap_ssid_def = HTTPServer__SetAPDefSSID(AP_DEF_SSID);

    wifi_config_t wifi_config_AP = {
   		.ap = {
   			.password = "",
			.ssid_hidden = 0,
   			.max_connection = AP_DEF_MAX_CONN,
   			.authmode = WIFI_AUTH_OPEN
   		},
       };


    strcpy((char*)wifi_config_AP.ap.ssid, ap_ssid_def);
    wifi_config_AP.ap.ssid_len = strlen(ap_ssid_def);


    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config_AP));

    printf("wifi_init_AP:\nSSID: %s\nPswd: Open Network\n" , ap_ssid_def);

return 1;
}



int WiFi__StartWiFi(void){

    ESP_ERROR_CHECK(esp_wifi_start());



return 1;
}


void WiFi__ReadCustomConfigFromNVM(void){
    

	size_t len = 0;
    NVM__ReadU8Value(HTMLCONF_GATEWAY_MODE,&WiFiConfig.gateway_mode);
    NVM__ReadString(HTMLCONF_AP_SSID, WiFiConfig.ap_ssid, &len);
    NVM__ReadU8Value(HTMLCONF_AP_SSID_HIDDEN, &WiFiConfig.ap_ssid_hidden);
    NVM__ReadString(HTMLCONF_AP_PSWD, WiFiConfig.ap_pswd, &len);
    NVM__ReadString(HTMLCONF_AP_IP, WiFiConfig.ap_ip, &len);
    NVM__ReadU8Value(HTMLCONF_AP_DHCP_MODE, &WiFiConfig.ap_dhcp_mode);
    NVM__ReadString(HTMLCONF_AP_DHCP_IP, WiFiConfig.ap_dhcp_ip, &len);


    NVM__ReadString(HTMLCONF_STA_SSID, WiFiConfig.sta_ssid, &len);
    NVM__ReadString(HTMLCONF_STA_ENCRYP, WiFiConfig.sta_encryption, &len);
    NVM__ReadString(HTMLCONF_STA_PSWD, WiFiConfig.sta_pswd, &len);
    NVM__ReadU8Value(HTMLCONF_STA_DHCP_MODE, &WiFiConfig.sta_dhcp_mode);
    NVM__ReadString(HTMLCONF_STA_STATIC_IP, WiFiConfig.sta_static_ip, &len);
    NVM__ReadString(HTMLCONF_STA_NETMASK, WiFiConfig.sta_netmask, &len);
    NVM__ReadString(HTMLCONF_STA_GATEWAY_IP, WiFiConfig.sta_gateway_ip, &len);
    NVM__ReadString(HTMLCONF_STA_PRI_DNS, WiFiConfig.sta_primary_dns, &len);
    NVM__ReadString(HTMLCONF_STA_SCND_DNS, WiFiConfig.sta_secondary_dns, &len);

    NVM__ReadString(HTMLCONF_NTP_SRVR_ADDR, WiFiConfig.ntp_server_addr, &len);
	NVM__ReadString(HTMLCONF_NTP_SRVR_PORT, WiFiConfig.ntp_server_port, &len);

	NVM__ReadString(HTMLCONF_MQTT_SRVR_ADDR, WiFiConfig.mqtt_server_addr, &len);
	NVM__ReadString(HTMLCONF_MQTT_SRVR_PORT, WiFiConfig.mqtt_server_port, &len);



	PRINTF_DEBUG("gateway_mode: %d\n",WiFiConfig.gateway_mode);
	PRINTF_DEBUG("ap_ssid: %s\n",WiFiConfig.ap_ssid);
    PRINTF_DEBUG("ap_ssid_hidden: %d\n",WiFiConfig.ap_ssid_hidden);
    PRINTF_DEBUG("ap_pswd: %s\n",WiFiConfig.ap_pswd);
    PRINTF_DEBUG("ap_ip: %s\n",WiFiConfig.ap_ip);
    PRINTF_DEBUG("ap_dhcp_mode: %d\n",WiFiConfig.ap_dhcp_mode);
    PRINTF_DEBUG("ap_dhcp_ip: %s\n",WiFiConfig.ap_dhcp_ip);
    PRINTF_DEBUG("sta_ssid: %s\n",WiFiConfig.sta_ssid);
    PRINTF_DEBUG("sta_encryption: %s\n",WiFiConfig.sta_encryption);
    PRINTF_DEBUG("sta_pswd: %s\n",WiFiConfig.sta_pswd);
    PRINTF_DEBUG("sta_dhcp_mode: %d\n",WiFiConfig.sta_dhcp_mode);
    PRINTF_DEBUG("sta_static_ip: %s\n",WiFiConfig.sta_static_ip);
    PRINTF_DEBUG("sta_netmask: %s\n",WiFiConfig.sta_netmask);
    PRINTF_DEBUG("sta_gateway_ip: %s\n",WiFiConfig.sta_gateway_ip);
    PRINTF_DEBUG("sta_primary_dns: %s\n",WiFiConfig.sta_primary_dns);
    PRINTF_DEBUG("sta_secondary_dns: %s\n",WiFiConfig.sta_secondary_dns);

    PRINTF_DEBUG("\nntp_server_addr: %s\n",WiFiConfig.ntp_server_addr);
    PRINTF_DEBUG("ntp_server_port: %s\n",WiFiConfig.ntp_server_port);
    PRINTF_DEBUG("\nmqtt_server_addr: %s\n",WiFiConfig.mqtt_server_addr);
    PRINTF_DEBUG("mqtt_server_port: %s\n",WiFiConfig.mqtt_server_port);

}



void WiFi__WriteCustomConfigInNVM(html_config_param_t config){

    NVM__WriteU8Value(HTMLCONF_GATEWAY_MODE, config.gateway_mode);
    NVM__WriteString(HTMLCONF_AP_SSID, config.ap_ssid);
    NVM__WriteU8Value(HTMLCONF_AP_SSID_HIDDEN, config.ap_ssid_hidden);
    NVM__WriteString(HTMLCONF_AP_PSWD, config.ap_pswd);
    NVM__WriteString(HTMLCONF_AP_IP, config.ap_ip);
    NVM__WriteU8Value(HTMLCONF_AP_DHCP_MODE, config.ap_dhcp_mode);
    NVM__WriteString(HTMLCONF_AP_DHCP_IP, config.ap_dhcp_ip);

    NVM__WriteString(HTMLCONF_STA_SSID, config.sta_ssid);
    NVM__WriteString(HTMLCONF_STA_ENCRYP, config.sta_encryption);
    NVM__WriteString(HTMLCONF_STA_PSWD, config.sta_pswd);

    NVM__WriteU8Value(HTMLCONF_STA_DHCP_MODE, config.sta_dhcp_mode);
    NVM__WriteString(HTMLCONF_STA_STATIC_IP, config.sta_static_ip);
    NVM__WriteString(HTMLCONF_STA_NETMASK, config.sta_netmask);
    NVM__WriteString(HTMLCONF_STA_GATEWAY_IP, config.sta_gateway_ip);
    NVM__WriteString(HTMLCONF_STA_PRI_DNS, config.sta_primary_dns);
    NVM__WriteString(HTMLCONF_STA_SCND_DNS, config.sta_secondary_dns);

    NVM__WriteString(HTMLCONF_NTP_SRVR_ADDR, config.ntp_server_addr);
	NVM__WriteString(HTMLCONF_NTP_SRVR_PORT, config.ntp_server_port);


	if(0 != strlen(config.mqtt_server_addr) && 0 != strlen(config.mqtt_server_port)){
		NVM__WriteString(HTMLCONF_MQTT_SRVR_ADDR, config.mqtt_server_addr);
		NVM__WriteString(HTMLCONF_MQTT_SRVR_PORT, config.mqtt_server_port);
		NVM__WriteU8Value(MQTT_URL, CONFIGURED);
	}else{
		NVM__EraseKey(MQTT_URL);
	}



}

html_config_param_t WiFi__GetCustomConfig (void){
    return WiFiConfig;
}

html_config_param_t* WiFi__GetCustomConfigPTR (void){
    return &WiFiConfig;
}

esp_err_t WiFi__SetCustomConfig(html_config_param_t config){
    
    s_wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();


    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    SetAPConfig(config.ap_ip, config.ap_ip, AP_DEF_NETMASK);

    wifi_config_t wifi_config_AP = {
   		.ap = {
   			.ssid_len = strlen(config.ap_ssid),
			.ssid_hidden = config.ap_ssid_hidden,
   			.max_connection = AP_DEF_MAX_CONN,
   			.authmode = WIFI_AUTH_WPA_WPA2_PSK
   			},
       };
	strcpy((char*)wifi_config_AP.ap.ssid,config.ap_ssid);
	strcpy((char*)wifi_config_AP.ap.password,config.ap_pswd);


	if(1 == config.ap_dhcp_mode)
	{
		u32_t end_ip = 0;
		u32_t temp = 0;
		//Set dhcp server base ip
		tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
		dhcps_lease_t optValue;
		optValue.enable = true;
		optValue.start_ip.addr = ipaddr_addr(config.ap_dhcp_ip);// End IP Address
		end_ip = optValue.start_ip.addr & 0x00FFFFFF;
		temp = optValue.start_ip.addr >> 24;
		temp += AP_DHCP_IP_RANGE;
		if(temp > 0xFA){
			temp = 0xFA000000;
		}else{
			temp = temp << 24;
		}
		end_ip |= temp;
		optValue.end_ip.addr = end_ip; // Start IP Address
		tcpip_adapter_dhcps_option(
			TCPIP_ADAPTER_OP_SET,
			TCPIP_ADAPTER_REQUESTED_IP_ADDRESS,
			&optValue, sizeof(optValue));
		tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);

	}else{
		tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP);
	}

    if (APSTA_MODE == config.gateway_mode)
    {
        wifi_config_t wifi_config_STA = {
			.sta = {
				.ssid = "STAPROBLEM",
				.password = "staproblem",
				},
		   };


        strcpy((char*)wifi_config_STA.sta.ssid,config.sta_ssid);
        strcpy((char*)wifi_config_STA.sta.password,config.sta_pswd);
        printf("\nSTA SSID = %s  and  Password = %s\n",wifi_config_STA.sta.ssid,wifi_config_STA.sta.password);

       if(!config.sta_dhcp_mode)
       {
    	   PRINTF_DEBUG("DHCP STA OFF\n");
           SetSTAStaticIP(config.sta_static_ip,
                            config.sta_netmask,
                            config.sta_gateway_ip,
                            config.sta_primary_dns,
                            config.sta_secondary_dns);
       }
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config_STA));
    }
    else if(AP_MODE == config.gateway_mode){
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    }
        printf("wifi_init_AP\n  SSID: %s\n  Pass: %s\n" , wifi_config_AP.ap.ssid, wifi_config_AP.ap.password);

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config_AP));

   return ESP_OK;
}


void WiFi__WaitConnection(void)
{
    xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

void WIFI__SetSTAStatus(connection_status_t status){
	STAStatus = status;
}
connection_status_t WIFI__GetSTAStatus(void){
	return STAStatus;
}


C_RES WiFi__GetMac(uint8_t* wifi_mac_address_gw){
	C_RES err = C_FAIL;
#ifdef INCLUDE_PLATFORM_DEPENDENT
		err = esp_wifi_get_mac(ESP_IF_WIFI_STA, wifi_mac_address_gw);
#endif
	return err;
}

int8_t WiFi__GetRSSI(void){
	int8_t rssi = 0;
#ifdef INCLUDE_PLATFORM_DEPENDENT
	wifi_ap_record_t wifidata;
	esp_wifi_sta_get_ap_info(&wifidata);
	rssi = wifidata.rssi;
#endif
	return rssi;
}
