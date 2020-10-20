#include <stdio.h>
#include <stdlib.h>

#include "../LocalConfig/ConfigManager.h"
#include "../LocalConfig/PipelineManager.h"

int main(int argc, char **argv)
{
	std::string piplineFile;
	ConfigMamager::GetInstance().Load(DEFAULT_CONFIG_FILE, piplineFile);
	LogConfSetting *logset = static_cast<LogConfSetting*>(ConfigMamager::GetInstance().GetLogSetting());
	PipelineManager mana;
	mana.Init(logset->mOutType, logset->mOutLevel, logset->mOutDir.c_str());

	mana.LoadPipeline(piplineFile);
	mana.PreparePipeline();

	mana.RunPipeline();

	if(getchar() == 'q');

	mana.StopPipeline();

	mana.UnInit();

	system("pause");
	return 0;
}