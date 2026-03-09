#include "io.h"
#include "cmsis_os.h"

HAL_StatusTypeDef IO_InitDigitalIO(DigitalIO* dio, const char* mutex_name) {
    if (dio == NULL || mutex_name == NULL) {
        return HAL_ERROR;
    }

    const osMutexAttr_t dio_mutex_attr = {
        .name = mutex_name
    };

    dio->mutex = osMutexNew(&dio_mutex_attr);
    if (dio->mutex == NULL) {
        return HAL_ERROR;
    }

    dio-> value = 0;
    dio->last_updated = 0;

    return HAL_OK;
}

HAL_StatusTypeDef IO_InitAnalogIO(AnalogIO* aio, const char* mutex_name) {
    if (aio == NULL || mutex_name == NULL) {
        return HAL_ERROR;
    }

    const osMutexAttr_t aio_mutex_attr = {
        .name = mutex_name
    };

    aio->mutex = osMutexNew(&aio_mutex_attr);
    if (aio->mutex == NULL) {
        return HAL_ERROR;
    }

    aio-> value = 0;
    aio->last_updated = 0;

    return HAL_OK;
}
HAL_StatusTypeDef IO_InitTemp(Temp* temp, const char* mutex_name) {
    if (temp == NULL || mutex_name == NULL) {
        return HAL_ERROR;
    }

    const osMutexAttr_t temp_mutex_attr = {
        .name = mutex_name
    };

    temp->mutex = osMutexNew(&temp_mutex_attr);
    if (temp->mutex == NULL) {
        return HAL_ERROR;
    }

    temp-> value = 0;
    temp->last_updated = 0;

    return HAL_OK;
}
HAL_StatusTypeDef IO_InitCurrent(Current* current, const char* mutex_name) {
    if (current == NULL || mutex_name == NULL) {
        return HAL_ERROR;
    }

    const osMutexAttr_t current_mutex_attr = {
        .name = mutex_name
    };

    current->mutex = osMutexNew(&current_mutex_attr);
    if (current->mutex == NULL) {
        return HAL_ERROR;
    }

    current-> value = 0;
    current->last_updated = 0;

    return HAL_OK;
}

uint8_t IO_GetDigitalIO(DigitalIO *dio){
    uint8_t val;

    osMutexAcquire(dio->mutex, osWaitForever);
    val = dio->value;
    osMutexRelease(dio->mutex);

    return val;
}

uint16_t IO_GetAnalogIO(AnalogIO *aio) {
    uint16_t val;

    osMutexAcquire(aio->mutex, osWaitForever);
    val = aio->value;
    osMutexRelease(aio->mutex);

    return val;
}

float IO_GetTemp(Temp *t) {
    float val;

    osMutexAcquire(t->mutex, osWaitForever);
    val = t->value;
    osMutexRelease(t->mutex);

    return val;
}

uint32_t IO_GetCurrent(Current *c) {
    uint32_t curr;

    osMutexAcquire(c->mutex, osWaitForever);
    curr = c->value;
    osMutexRelease(c->mutex);

    return curr;
}

void IO_SetDigitalIO(DigitalIO *dio, uint16_t value) {
    uint32_t now = osKernelGetTickCount();

    osMutexAcquire(dio->mutex, osWaitForever);
    dio->value = value;
    dio->last_updated = now;
    osMutexRelease(dio->mutex);
}

void IO_SetAnalogIO(AnalogIO *aio, uint16_t value) {
    uint32_t now = osKernelGetTickCount();

    osMutexAcquire(aio->mutex, osWaitForever);
    aio->value = value;
    aio->last_updated = now;
    osMutexRelease(aio->mutex);
}

void IO_SetTemp(Temp *t, float value) {
    uint32_t now = osKernelGetTickCount();

    osMutexAcquire(t->mutex, osWaitForever);
    t->value = value;
    t->last_updated = now;
    osMutexRelease(t->mutex);
}

void IO_SetCurrent(Current *c, uint32_t value) {
    uint32_t now = osKernelGetTickCount();

    osMutexAcquire(c->mutex, osWaitForever);
    c->value = value;
    c->last_updated = now;
    osMutexRelease(c->mutex);
}