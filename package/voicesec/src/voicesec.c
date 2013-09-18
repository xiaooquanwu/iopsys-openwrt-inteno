/*
 * voicesec.c
 * Utility that will encrypt/decrypt a string value using some very specific rules.
 * Undocumented and unhelpful by design.
 *
 * Usage example:
 * ./voicesec -e cleartext > mysecretfile.txt
 * #exec echo "secret = $(voicesec -d $(cat mysecretfile.txt))"
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <string.h>

#define UNDEFINED 0
#define ENCRYPT 1
#define DECRYPT 2

#define KEYFILE "/proc/nvram/DesKey" //encryption key

#define GRANDPARENT "asterisk" //grandparent process for decryption
#define MAXLEN 512 //max length of indata
#define ROUNDS 32 //recommended nr of rounds
#define CHUNK_SIZE 8 //8 byte for each round

/* take 64 bits of data in v[0] and v[1] and 128 bits of key[0] - key[3] */
void encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
	unsigned int i;
	uint32_t v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
	for (i=0; i < num_rounds; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
	}
	v[0]=v0; v[1]=v1;
}

void decipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4]) {
	unsigned int i;
	uint32_t v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta*num_rounds;
	for (i=0; i < num_rounds; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
	}
	v[0]=v0; v[1]=v1;
}

int getParentPid(int pid)
{
	char buf[1024];
	char path[1024];
	char* ppid = NULL;
	FILE *statusfile = NULL;

	sprintf(path, "/proc/%d/status", pid);
	statusfile = fopen(path, "r");
	if (!statusfile) {
		return 0;
	}

	while (fgets(buf, 1024, statusfile)) {
		ppid = strstr(buf, "PPid:");
		if (ppid) {
			ppid = ppid + 6;
			fclose(statusfile);
			return atoi(ppid);
		}
	}
	fclose(statusfile);
	return 0;
}

char* getProcessName(int pid)
{
	char buf[1024];
	char path[1024];
	char* name = NULL;
	FILE *statusfile = NULL;

	sprintf(path, "/proc/%d/status", pid);
	statusfile = fopen(path, "r");
	if (!statusfile) {
		return NULL;
	}

	while (fgets(buf, 1024, statusfile)) {
		name = strstr(buf, "Name:");
		if (name) {
			name = name + 6;
			int len = strlen(name);
			if (name[len-1] == '\n') {
				name[len-1] = '\0';
			}
			fclose(statusfile);
			return name;
		}
	}
	fclose(statusfile);
	return NULL;
}

int main(int argc, char **argv)
{
	int action = UNDEFINED;
	char key[16];
	memcpy(key, "daaurewoifkssfms", 16);

	size_t indata_length;
	char indata[MAXLEN];
	char result[MAXLEN];
	bzero(indata, MAXLEN);
	bzero(result, MAXLEN);

	//Find out what we are supposed to do
	if (argc == 3) {
		if (strcmp(argv[1], "-e") == 0) {
			action = ENCRYPT;

			indata_length = strlen(argv[2]);
			//Copy indata to buffer
			if (indata_length > MAXLEN) {
				indata_length = MAXLEN;
			}
			memcpy(indata, argv[2], indata_length);
		}
		else if (strcmp(argv[1], "-d") == 0) {
			action = DECRYPT;
			indata_length = 0; //Set to zero in case we fail reading file
	
			char input_filename[256];
			FILE *fp = NULL;
			long file_size;
			size_t read_size;

			//Read filename
			strncpy(input_filename, argv[2], sizeof(input_filename));
			input_filename[sizeof(input_filename)-1] = '\0';

			//Read encrypted data
			fp = fopen(input_filename, "r");
			if (fp != NULL) {
				for (;;) {
					if (fseek(fp, 0L, SEEK_END) != 0) {
						break;
					}

					file_size = ftell(fp);
					if (file_size == -1) {
						break;
					}

					if (fseek(fp, 0L, SEEK_SET) != 0) {
						break;
					}

					read_size = fread(&indata, sizeof(char), MAXLEN, fp);
					if (read_size <= 0) {
						break;
					}
					indata_length = read_size-1;
					break;
				}
				fclose(fp);
			}
		}
		else {
			indata_length = 0;
		}
	}
	else if (argc == 2) {
		indata_length = strlen(argv[1]);
		//Copy indata to buffer
		if (indata_length > MAXLEN) {
			indata_length = MAXLEN;
		}
		memcpy(indata, argv[1], indata_length);
	}
	else {
		indata_length = 0;
	}

	//Find out what our parent process is
	pid_t parent_pid = getppid();
	int grandparent_pid = getParentPid(parent_pid);
	char* grandparent_name = getProcessName(grandparent_pid);

	if (action == DECRYPT) {
		if (strcmp(grandparent_name, GRANDPARENT)) {
			//printf("Grandparent:%s[%d]", grandparent_name, grandparent_pid);
			action = UNDEFINED;
		}
	}

	//Find our key
	FILE *key_file = fopen(KEYFILE, "r");
	if (key_file) {
		if(fread(key, sizeof(char), 16, key_file) != 16) {
			//if keyfile does not contain enough data, we use built in key instead
			memcpy(key, "daaurewoifkssfms", 16);
		}
		fclose(key_file);
	}

	//Loop over data and perform requested action on each chunk
	uint32_t k[4]; //key
	uint32_t v[2]; //data
	memcpy(k, key, 16);
	int i = 0;
	while (i < indata_length) {
		memcpy(v, (indata + i), CHUNK_SIZE);
		switch (action) {
			case ENCRYPT:
				encipher(ROUNDS, v, k);
				break;
			case DECRYPT:
				decipher(ROUNDS, v, k);
				break;
			default:
				//Do nothing
				break;
		}
		memcpy((result + i), v, CHUNK_SIZE);
		i = i + CHUNK_SIZE;
	}

	printf("%s\n", result);
	return 0;
}

