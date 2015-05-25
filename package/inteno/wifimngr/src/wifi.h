#ifndef WIFI_H
#define WIFI_H

#include <iostream>
#include <string>
using namespace std;

//class Wireless {
//	public:
//	string ssid, encryption, key;
//	string radio2g, radio5g;
//	int enable, channel2g, channel5g;
//	string getChannels (int);
//	void setChannel (int,int);
//	void setSsid (string);
//	void setKey (string);
//};

//class WirelessRadio {
//	public:
//	string name, country, hwmode;
//	char band;
//	int bwCap, channel, autoChanTimer, dfsChanSelect, obssCoex, txPower, rxChainPwrSave;
//	int frameBursting, fragThresh, rtsThresh, dtimInterval, bcnInterval, wmmMode;
//	void setChannel (int);
//};

//class WirelessInterface {
//	public:
//	string ifname;
//	string radio, mode, ssid, encryption, cipher, key;
//	int enable, gtkRekeyIntv, enableWps, macFilterMode;
//	int hidden, maxAssoc, isolate, enableWmf; 
//	void setFilterList (string);
//	string getMacFilterList (void);
//};

//class WPS {
//	public:
//	int enable;
//	void activate (int);
//	void changeStatus ();
//};

#endif

#define	MAX_WDEV 2
#define	MAX_WIFC 2

#include <uci.h>
struct uci_package *init_package(const char *config);
const char * ugets(struct uci_section *s, const char *opt);
int ugeti(struct uci_section *s, const char *opt);
void uset(struct uci_section *s, const char *opt, const char *value);
void ucommit(struct uci_section *s);
const char* uciGet(const char *p, const char *s, const char *o);
void uciSet(const char *p, const char *s, const char *o, const char *t);
void uciCommit(const char *p);
void runCmd(const char *pFmt, ...);
string strCmd(const char *pFmt, ...);
string toStr(int num);
//void populateWireless(void);
