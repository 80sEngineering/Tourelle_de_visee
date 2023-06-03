#include <stdio.h>
#include <stdlib.h>


int GetGPSData(char* buffer) {
    FILE *device;
    device = fopen("/dev/cu.usbmodem21402", "r");
    if (device == NULL) {
        printf("Failed to open the device.\n");
        return 1;
    }

    while (fgets(buffer, 256, device) != NULL) {
        printf("%s", buffer);
        break;
    }
    fclose(device);
    return 0;
}
