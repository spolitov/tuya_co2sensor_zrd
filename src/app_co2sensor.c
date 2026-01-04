#include "app_main.h"

uint8_t remote_cmd_pkt_buff[DATA_MAX_LEN+12];

void local_cmd_co2(void *args) {
    if (args) {
        return; // Always do nothing.
    }
    int32_t *co2 = (int32_t*)args;
    float divisor = 1000000.0f / data_point_model[DP_IDX_CO2].divisor;
    float attrCo2 = *co2 / divisor;

    zcl_setAttrVal(APP_ENDPOINT1, ZCL_CLUSTER_MS_CO2_MEASUREMENT, ZCL_CO2_MEASUREMENT_ATTRID_MEASUREDVALUE, (uint8_t*)&attrCo2);

#if UART_PRINTF_MODE && DEBUG_CMD
            printf("PPM: %d\r\n", *co2);
#endif
}


