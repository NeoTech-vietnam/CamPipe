/* Real backend + real project task configuration; no native RTOS task headers. */
#include "CamPipe_Backend.h"
#include "CamPipe_TaskConfig.h"
#include "WifiMgr_Cfg.h"
#include "esp_heap_caps.h"
#include <assert.h>
#include <stdio.h>

struct EbOs_Task { uint32 stack_free; };
static struct EbOs_Task worker = {1024U};
static Neo_StatusType create_result = NEO_OK;
static Neo_StatusType join_result = NEO_OK;
static unsigned creates, joins, steps, yields;
static uint32 joined_timeout;
const WifiMgr_ConfigType WifiMgr_Config = {"test", "", 6U, 1U, 20U};

Neo_StatusType EbOs_TaskCreate(const EbOs_TaskConfigType *config, void *argument, EbOs_TaskType *out)
{
    assert(config == &CamPipe_TaskConfig && argument == NULL_PTR);
    assert(config->stack_bytes == 8192U && config->priority == 5U);
    assert(config->core_affinity == -1);
    ++creates;
    *out = create_result == NEO_OK ? &worker : NULL_PTR;
    return create_result;
}

Neo_StatusType EbOs_TaskJoinAndDelete(EbOs_TaskType *task, uint32 timeout_ms)
{
    assert(task != NULL_PTR);
    ++joins;
    joined_timeout = timeout_ms;
    if (join_result == NEO_OK) *task = NULL_PTR;
    return join_result;
}

Neo_StatusType EbOs_TaskGetStackMinFree(EbOs_TaskType task, uint32 *bytes)
{
    assert(task == &worker);
    *bytes = task->stack_free;
    return NEO_OK;
}

boolean CamPipe_ProducerStep(void) { return ++steps <= 3U ? TRUE : FALSE; }
void EbOs_DelayOneTick(void) { ++yields; }
size_t heap_caps_get_free_size(uint32_t caps) { return 100U + caps; }
size_t heap_caps_get_minimum_free_size(uint32_t caps) { return 50U + caps; }
size_t heap_caps_get_largest_free_block(uint32_t caps) { return 75U + caps; }

int main(void)
{
    Neo_PipelineStatsType stats = {0};
    CamPipe_BackendSample(&stats);
    assert(stats.producer_stack_bytes == 0U);
    create_result = NEO_E_MEMORY;
    assert(CamPipe_BackendStart() == NEO_E_MEMORY);
    create_result = NEO_OK;
    assert(CamPipe_BackendStart() == NEO_OK);
    assert(CamPipe_BackendStart() == NEO_E_STATE && creates == 2U);
    CamPipe_TaskConfig.entry(NULL_PTR);
    assert(steps == 4U && yields == 3U);
    CamPipe_BackendSample(&stats);
    assert(stats.producer_stack_bytes == 1024U && stats.free_psram == 101U);
    assert(stats.cpu_mhz == 240U && stats.wifi_channel == 6U);
    join_result = NEO_E_TIMEOUT;
    assert(CamPipe_BackendStop(25U) == NEO_E_TIMEOUT && joined_timeout == 25U);
    assert(CamPipe_BackendStart() == NEO_E_STATE);
    CamPipe_BackendSample(&stats);
    assert(stats.producer_stack_bytes == 1024U);
    join_result = NEO_OK;
    assert(CamPipe_BackendStop(25U) == NEO_OK && joins == 2U);
    CamPipe_BackendSample(&stats);
    assert(stats.producer_stack_bytes == 0U);
    assert(CamPipe_BackendStart() == NEO_OK);
    assert(CamPipe_BackendStop(0U) == NEO_OK);
    puts("PASS camera EbOs lifecycle and project runnable wiring");
    return 0;
}
