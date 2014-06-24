#include <endpointdrv.h>
#include <tpProfiles.h>
#include <stdio.h>
#include <strings.h>

static int endptCfg(VRG_COUNTRY country);

/* List of supported country name, ISO 3166-1 alpha-3 codes */
typedef struct COUNTRY_TABLE
{
	VRG_COUNTRY	vrgCountry;
	char		isoCode[3];
} COUNTRY_MAP;

static COUNTRY_MAP countryMap[] =
{
	{VRG_COUNTRY_AUSTRALIA,				"AUS"},
	{VRG_COUNTRY_BELGIUM,				"BEL"},
	{VRG_COUNTRY_BRAZIL,				"BRA"},
	{VRG_COUNTRY_CHILE,					"CHL"},
	{VRG_COUNTRY_CHINA,	 				"CHN"},
	{VRG_COUNTRY_CZECH, 				"CZE"},
	{VRG_COUNTRY_DENMARK, 				"DNK"},
	{VRG_COUNTRY_ETSI, 					"ETS"}, //Not really an iso code
	{VRG_COUNTRY_FINLAND, 				"FIN"},
	{VRG_COUNTRY_FRANCE, 				"FRA"},
	{VRG_COUNTRY_GERMANY, 				"DEU"},
	{VRG_COUNTRY_HUNGARY,				"HUN"},
	{VRG_COUNTRY_INDIA,					"IND"},
	{VRG_COUNTRY_ITALY, 				"ITA"},
	{VRG_COUNTRY_JAPAN,	 				"JPN"},
	{VRG_COUNTRY_NETHERLANDS, 			"NLD"},
	{VRG_COUNTRY_NEW_ZEALAND, 			"NZL"},
	{VRG_COUNTRY_NORTH_AMERICA, 		"USA"},
	{VRG_COUNTRY_SPAIN, 				"ESP"},
	{VRG_COUNTRY_SWEDEN,				"SWE"},
	{VRG_COUNTRY_SWITZERLAND, 			"CHE"},
	{VRG_COUNTRY_NORWAY, 				"NOR"},
	{VRG_COUNTRY_TAIWAN,	 			"TWN"},
	{VRG_COUNTRY_UK,		 			"GBR"},
	{VRG_COUNTRY_UNITED_ARAB_EMIRATES,	"ARE"},
	{VRG_COUNTRY_CFG_TR57, 				"T57"}, //Not really an iso code
	{VRG_COUNTRY_MAX, 					"-"}
};

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("%s: Missing or incorrect arguments\n", __FUNCTION__);
		return 1;
	}

	VRG_COUNTRY country;
	const COUNTRY_MAP *countryItr;
	for (countryItr = countryMap; countryItr->vrgCountry != VRG_COUNTRY_MAX; countryItr++) {
		if (!strcmp(argv[1], countryItr->isoCode)) {
			country = countryItr->vrgCountry;
			break;
		}
	}

	if (countryItr->vrgCountry == VRG_COUNTRY_MAX) {
		printf("%s: Unknown country '%s'\n", __FUNCTION__, argv[1]);
		return 1;
	}

	return endptCfg(country);
}

int endptCfg(VRG_COUNTRY country)
{
	VRG_ENDPT_INIT_CFG vrgEndptInitCfg;
	int rc, i;

	printf("%s: Configuring endpoint interface\n", __FUNCTION__);

	bosInit();

	if (vrgEndptDriverOpen() != EPSTATUS_SUCCESS) {
		return 1;
	}

	bzero(&vrgEndptInitCfg, sizeof(VRG_ENDPT_INIT_CFG));
	vrgEndptInitCfg.country = country;
	vrgEndptInitCfg.currentPowerSource = 0;
	if (!isEndptInitialized()) {
		if (vrgEndptInit(&vrgEndptInitCfg, NULL, NULL, NULL, NULL, NULL, NULL) != EPSTATUS_SUCCESS) {
			printf("%s: endpoint init failed\n", __FUNCTION__);
			return 1;
		}
	}

	tpUpdateLocaleProfile(country);
	vrgEndptDriverClose();

	printf("%s: done\n", __FUNCTION__);
	return 0;
}
