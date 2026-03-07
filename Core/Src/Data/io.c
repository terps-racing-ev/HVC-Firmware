#include "io.h"
#include "cmsis_os.h"

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