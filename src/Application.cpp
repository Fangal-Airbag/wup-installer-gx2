/****************************************************************************
 * Copyright (C) 2015 Dimok
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <coreinit/foreground.h>
#include <proc_ui/procui.h>
#include <coreinit/memdefaultheap.h>
#include <nn/erreula.h>
#include "Application.h"
#include "gui/FreeTypeGX.h"
#include "gui/GuiImageAsync.h"
#include "gui/VPadController.h"
#include "gui/WPadController.h"
#include "resources/Resources.h"
#include "sounds/SoundHandler.hpp"
#include "system/exception_handler.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "video/CursorDrawer.h"

Application *Application::applicationInstance = NULL;
bool Application::exitApplication = false;
bool Application::quitRequest = false;

u32 Application::hbmDeniedCallback(void *context)
{
	nn::erreula::HomeNixSignArg homeNixSignArg;
	nn::erreula::AppearHomeNixSign(homeNixSignArg);
	
	return 0;
}

Application::Application()
	: CThread(CThread::eAttributeAffCore0 | CThread::eAttributePinnedAff, 0, 0x20000)
	, bgMusic(NULL)
	, video(NULL)
	, mainWindow(NULL)
    , fontSystem(NULL)
	, exitDisabled(false)
{
	controller[0] = new VPadController(GuiTrigger::CHANNEL_1);
	controller[1] = new WPadController(GuiTrigger::CHANNEL_2);
	controller[2] = new WPadController(GuiTrigger::CHANNEL_3);
	controller[3] = new WPadController(GuiTrigger::CHANNEL_4);
	controller[4] = new WPadController(GuiTrigger::CHANNEL_5);
	
    //! load resources
    Resources::LoadFiles("fs:/vol/content");

	bgMusic = new GuiSound(Resources::GetFile("bgMusic.ogg"), Resources::GetFileSize("bgMusic.ogg"));
	bgMusic->SetLoop(true);
	bgMusic->Play();
	bgMusic->SetVolume(60);

	exitApplication = false;

    ProcUIInit(OSSavesDone_ReadyToRelease);
	ProcUIRegisterCallback(PROCUI_CALLBACK_HOME_BUTTON_DENIED, hbmDeniedCallback, NULL, 1);
}

Application::~Application()
{
	delete bgMusic;
	
	for(int i = 0; i < 5; i++)
		delete controller[i];
	
	AsyncDeleter::destroyInstance();
	GuiImageAsync::threadExit();
	Resources::Clear();
	
	SoundHandler::DestroyInstance();
	
	CursorDrawer::destroyInstance();
	ProcUIShutdown();
}

void Application::exec()
{
	//! start main GX2 thread
	resumeThread();
	//! now wait for thread to finish
	shutdownThread();
}

void Application::quit()
{
	exitApplication = true;
    quitRequest = true;
}

void Application::fadeOut()
{
    GuiImage fadeOut(video->getTvWidth(), video->getTvHeight(), (GX2Color){ 0, 0, 0, 255 });

	for(int i = 0; i < 255; i += 10)
    {
        if(i > 255)
            i = 255;

        fadeOut.setAlpha(i / 255.0f);

        //! start rendering DRC
	    video->prepareDrcRendering();
	    mainWindow->drawDrc(video);

        GX2SetDepthOnlyControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_FUNC_ALWAYS);
        fadeOut.draw(video);
        GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_FUNC_LEQUAL);

	    video->drcDrawDone();

        //! start rendering TV
	    video->prepareTvRendering();

	    mainWindow->drawTv(video);

        GX2SetDepthOnlyControl(GX2_DISABLE, GX2_DISABLE, GX2_COMPARE_FUNC_ALWAYS);
        fadeOut.draw(video);
        GX2SetDepthOnlyControl(GX2_ENABLE, GX2_ENABLE, GX2_COMPARE_FUNC_LEQUAL);

	    video->tvDrawDone();

	    //! as last point update the effects as it can drop elements
	    mainWindow->updateEffects();

	    video->waitForVSync();
    }

    //! one last cleared black screen
    video->prepareDrcRendering();
    video->drcDrawDone();
    video->prepareTvRendering();
    video->tvDrawDone();
    video->waitForVSync();
    video->tvEnable(false);
    video->drcEnable(false);
}

bool Application::procUI(void)
{
    bool executeProcess = false;
	
	switch(ProcUIProcessMessages(true))
    {
		case PROCUI_STATUS_EXITING:
		{
			log_printf("PROCUI_STATUS_EXITING\n");
			exitApplication = true;
			break;
		}
		case PROCUI_STATUS_RELEASE_FOREGROUND:
		{
			log_printf("PROCUI_STATUS_RELEASE_FOREGROUND\n");
			if(video != nullptr)
			{
				// we can turn of the screen but we don't need to and it will display the last image
				video->tvEnable(true);
				video->drcEnable(true);
				
				log_printf("delete fontSystem\n");
				delete fontSystem;
				fontSystem = nullptr;
				
				log_printf("delete video\n");
				delete video;
				video = nullptr;
				
				log_printf("deinitialze memory\n");
				memoryRelease();
				ProcUIDrawDoneRelease();
			}
			else
			{
				ProcUIDrawDoneRelease();
			}
			break;
		}
		case PROCUI_STATUS_IN_FOREGROUND:
		{
			if(!quitRequest)
			{
				if(video == nullptr)
				{
					log_printf("PROCUI_STATUS_IN_FOREGROUND\n");
					log_printf("initialze memory\n");
					memoryInitialize();
					
					log_printf("Initialize video\n");
					video = new CVideo(GX2_TV_SCAN_MODE_720P, GX2_DRC_RENDER_MODE_SINGLE);
					log_printf("Video size %i x %i\n", video->getTvWidth(), video->getTvHeight());
					
					//! setup default Font
					log_printf("Initialize main font system\n");
					auto *fontSystem = new FreeTypeGX(Resources::GetFile("font.ttf"), Resources::GetFileSize("font.ttf"), true);
					GuiText::setPresetFont(fontSystem);

					if (mainWindow == nullptr)
					{
						log_printf("Initialize main window\n");
						mainWindow = new MainWindow(video->getTvWidth(), video->getTvHeight());
					}
				}
				executeProcess = true;
			}
			break;
		}
		case PROCUI_STATUS_IN_BACKGROUND:
		default:
			break;
    }

    return executeProcess;
}

void Application::executeThread(void)
{
	//! setup exceptions on the main GX2 core
	setup_os_exceptions();
	
	log_printf("Entering main loop\n");
	
	//! main GX2 loop (60 Hz cycle with max priority on core 1)
	while(!exitApplication)
	{
	    if(procUI() == false)
			continue;
		
		mainWindow->lockGUI();
		//! Read out inputs
		for(int i = 0; i < 5; i++)
		{
			if(controller[i]->update(video->getTvWidth(), video->getTvHeight()) == false)
				continue;
			
			//! update controller states
			mainWindow->update(controller[i]);
		}
		
		//! start rendering DRC
		video->prepareDrcRendering();
		mainWindow->drawDrc(video);
		video->drcDrawDone();
		
		//! start rendering TV
		video->prepareTvRendering();
		mainWindow->drawTv(video);
		video->tvDrawDone();
		
		//! enable screen after first frame render
		if(video->getFrameCount() == 0) {
			video->tvEnable(true);
			video->drcEnable(true);
		}
		
		//! as last point update the effects as it can drop elements
		mainWindow->updateEffects();
		mainWindow->unlockGUI();
		
		video->waitForVSync();
		
		//! transfer elements to real delete list here after all processes are finished
		//! the elements are transfered to another list to delete the elements in a separate thread
		//! and avoid blocking the GUI thread
		AsyncDeleter::triggerDeleteProcess();
	}
	
	log_printf("Exiting main loop\n");
	
    if(video)
    {
        fadeOut();
    }
}
