#ifndef INC_LIGHT_ANOMALY_H_
#define INC_LIGHT_ANOMALY_H_

#include "stm32l4xx_hal.h"
#include "ai_datatypes_defines.h"
#include "ai_platform.h"
#include "anomalymodel.h"
#include "anomalymodel_data.h"
#include "bh1750.h"

#define WINDOW_SIZE         64
#define SAMPLE_INTERVAL_MS  120
#define LUX_SCALER_MIN      3891.67f
#define LUX_SCALER_MAX      4949.17f
#define ANOMALY_THRESHOLD   0.42898235f

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

static int window_fill_count;
AI_ALIGNED(4) static ai_u8 activations[AI_ANOMALYMODEL_DATA_ACTIVATIONS_SIZE];
AI_ALIGNED(4) static ai_float ai_input_buffer[AI_ANOMALYMODEL_IN_1_SIZE];
AI_ALIGNED(4) static ai_float ai_output_buffer[AI_ANOMALYMODEL_OUT_1_SIZE];
static ai_handle network = AI_HANDLE_NULL;
static ai_buffer *ai_input;
static ai_buffer *ai_output;

static void ScaleWindow(const float *raw_window, ai_float *scaled_out)
{
    float range = LUX_SCALER_MAX - LUX_SCALER_MIN;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        scaled_out[i] = (raw_window[i] - LUX_SCALER_MIN) / range;
    }
}

static float ComputeReconstructionError(const ai_float *input, const ai_float *output)
{
    float sum_sq = 0.0f;
    for (int i = 0; i < WINDOW_SIZE; i++) {
        float diff = input[i] - output[i];
        sum_sq += diff * diff;
    }
    return sum_sq / (float)WINDOW_SIZE;
}

int LedAnomaly_AiInit(void)
{
    ai_error err;
    const ai_handle act_addr[] = { activations };

    err = ai_anomalymodel_create_and_init(&network, act_addr, NULL);
    if (err.type != AI_ERROR_NONE) {
        char msg[64];
        snprintf(msg, sizeof(msg), "AI init failed, type=%d code=%d\r\n", err.type, err.code);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return -1;
    }

    ai_input = ai_anomalymodel_inputs_get(network, NULL);
    ai_output = ai_anomalymodel_outputs_get(network, NULL);

    return 0;
}

static float RunInference(const ai_float *scaled_input)
{
    memcpy(ai_input_buffer, scaled_input, sizeof(ai_input_buffer));

    ai_input[0].data = AI_HANDLE_PTR(ai_input_buffer);
    ai_output[0].data = AI_HANDLE_PTR(ai_output_buffer);

    ai_i32 batches_run = ai_anomalymodel_run(network, ai_input, ai_output);
    if (batches_run != 1) {
        ai_error err = ai_anomalymodel_get_error(network);
        char msg[64];
        snprintf(msg, sizeof(msg), "AI run failed, type=%d code=%d\r\n", err.type, err.code);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return -1.0f;
    }

    return ComputeReconstructionError(ai_input_buffer, ai_output_buffer);
}

int LedAnomaly_Init(void)
{
    window_fill_count = 0;
    return LedAnomaly_AiInit();
}

void LedAnomaly_Update(float *out_error, uint8_t *out_is_anomaly, float buffer[])
{
    ai_float scaled_window[WINDOW_SIZE];
    ScaleWindow(buffer, scaled_window);

    float error = RunInference(scaled_window);

    *out_error = error;
    *out_is_anomaly = (error > ANOMALY_THRESHOLD) ? 1 : 0;

}

#endif /* INC_LIGHT_ANOMALY_H_ */
