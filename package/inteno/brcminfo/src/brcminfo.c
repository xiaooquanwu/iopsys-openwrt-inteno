/*
 * brcminfo.c
 * Minor utility that will attempt to read the number of available
 * endpoints from the brcm voice driver
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <endpointdrv.h>

static int endpoint_fd = -1;
static int num_fxs_endpoints = -1;
static int num_fxo_endpoints = -1;
static int num_dect_endpoints = -1;
static int num_endpoints = -1;

int vrgEndptDriverOpen(void)
{
	endpoint_fd = open("/dev/bcmendpoint0", O_RDWR);
	if (endpoint_fd == -1) {
		return -1;
	} else {
		return 0;
	}
}

int vrgEndptDriverClose(void)
{
	int result = close(endpoint_fd);
	endpoint_fd = -1;
	return result;
}

static int brcm_get_endpoints_count(void)
{
	ENDPOINTDRV_ENDPOINTCOUNT_PARM endpointCount;
	endpointCount.size = sizeof(ENDPOINTDRV_ENDPOINTCOUNT_PARM);

	if ( ioctl( endpoint_fd, ENDPOINTIOCTL_FXSENDPOINTCOUNT, &endpointCount ) != IOCTL_STATUS_SUCCESS ) {
		return -1;
	} else {
		num_fxs_endpoints = endpointCount.endpointNum;
	}

	if ( ioctl( endpoint_fd, ENDPOINTIOCTL_FXOENDPOINTCOUNT, &endpointCount ) != IOCTL_STATUS_SUCCESS ) {
		return -1;
	} else {
		num_fxo_endpoints = endpointCount.endpointNum;
	}

	if ( ioctl( endpoint_fd, ENDPOINTIOCTL_DECTENDPOINTCOUNT, &endpointCount ) != IOCTL_STATUS_SUCCESS ) {
		return -1;
	} else {
		num_dect_endpoints = endpointCount.endpointNum;
	}

	num_endpoints = num_fxs_endpoints + num_fxo_endpoints + num_dect_endpoints;
	return 0;
}

int main(int argc, char **argv)
{
	int result;

	result = vrgEndptDriverOpen();
	if (result != 0) {
		printf("Could not open endpoint driver\n");
		return result;
	}

	result = brcm_get_endpoints_count();
	if (result == 0) {
		printf("DECT Endpoints:\t%d\n", num_dect_endpoints);
		printf("FXS Endpoints:\t%d\n", num_fxs_endpoints);
		printf("FXO Endpoints:\t%d\n", num_fxo_endpoints);
		printf("All Endpoints:\t%d\n", num_endpoints);
	}
	else {
		printf("Endpoint counting failed, maybe driver is not initialized?\n");
	}

	result = vrgEndptDriverClose();
	if (result != 0) {
		printf("Could not close endpoint driver\n");
		return result;
	}

	return 0;
}
