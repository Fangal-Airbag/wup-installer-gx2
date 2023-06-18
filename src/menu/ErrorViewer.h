#pragma once

#include <nn/erreula.h>
#include <coreinit/filesystem.h>

class ErrorViewer
{
    public:
        ErrorViewer();

        ~ErrorViewer();

        void calc();

        static void drawTV();

        static void drawDRC();

    private:
        FSClient *client;
        nn::erreula::CreateArg createArg;
};