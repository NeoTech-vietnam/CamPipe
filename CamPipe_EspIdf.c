#include "CamPipe_Backend.h"
#include "CamPipe_TaskConfig.h"
#include "WifiMgr_Cfg.h"

#include "esp_heap_caps.h"
#include "sdkconfig.h"

static EbOs_TaskType producer_task;

Neo_StatusType CamPipe_BackendStart(void)
{
    if (producer_task != NULL_PTR) return NEO_E_STATE;
    return EbOs_TaskCreate(&CamPipe_TaskConfig, NULL_PTR, &producer_task);
}

Neo_StatusType CamPipe_BackendStop(uint32 timeout_ms)
{
    return EbOs_TaskJoinAndDelete(&producer_task, timeout_ms);
}

void CamPipe_BackendSample(Neo_PipelineStatsType *stats)
{
    stats->free_psram = (uint32)heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    stats->min_free_psram = (uint32)heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
    stats->largest_psram = (uint32)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    stats->free_internal = (uint32)heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    stats->min_free_internal = (uint32)heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    /* Lifecycle control quiesces status callers before retiring the handle. */
    stats->producer_stack_bytes = 0U;
    if (producer_task != NULL_PTR) {
        (void)EbOs_TaskGetStackMinFree(producer_task, &stats->producer_stack_bytes);
    }
    stats->cpu_mhz = CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
    stats->psram_mhz = CONFIG_SPIRAM_SPEED;
    stats->wifi_channel = WifiMgr_Config.channel;
    stats->wifi_bandwidth_mhz = WifiMgr_Config.bandwidth_mhz;
#if defined(CONFIG_CAMERA_PSRAM_DMA) && CONFIG_CAMERA_PSRAM_DMA
    stats->psram_dma = TRUE;
#else
    stats->psram_dma = FALSE;
#endif
}
