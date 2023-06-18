#include <stdio.h>
#include <malloc.h>
#include <vpad/input.h>
#include "utils/logger.h"
#include "ErrorViewer.h"

ErrorViewer::ErrorViewer() 
{
    client = (FSClient *)malloc(sizeof(FSClient));
    FSAddClient(client, FS_ERROR_FLAG_ALL);
    
    createArg.workMemory = malloc(nn::erreula::GetWorkMemorySize());
    createArg.fsClient = client;
    if (!nn::erreula::Create(createArg)) {
        log_printf("Failed to create error viewer");
        return;
    }
}

ErrorViewer::~ErrorViewer() 
{
    nn::erreula::Destroy();
    free(createArg.workMemory);
    FSDelClient(client, FS_ERROR_FLAG_ALL);
    free(client);
}

void ErrorViewer::calc()
{
    VPADStatus vpadStatus;
    VPADRead(VPAD_CHAN_0, &vpadStatus, 1, nullptr);
    VPADGetTPCalibratedPoint(VPAD_CHAN_0, &vpadStatus.tpNormal, &vpadStatus.tpNormal);

    nn::erreula::ControllerInfo controllerInfo;
    controllerInfo.vpad = &vpadStatus;
    controllerInfo.kpad[0] = nullptr;
    controllerInfo.kpad[1] = nullptr;
    controllerInfo.kpad[2] = nullptr;
    controllerInfo.kpad[3] = nullptr;
    
    nn::erreula::Calc(controllerInfo);
}

void ErrorViewer::drawTV()
{
    nn::erreula::DrawTV();
}

void ErrorViewer::drawDRC()
{
    nn::erreula::DrawDRC();
}