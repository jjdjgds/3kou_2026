/*==========================================================================
âπçƒê∂Ç∆ì«Ç›çûÇ›[Audio.h]



													Author : hidetoshi muramatu

---------------------------------------------------------------------------



==========================================================================*/


#pragma once



void InitAudio();
void UninitAudio();


int LoadAudio(const char* FileName);
void UnloadAudio(int Index);
void PlayAudio(int Index, bool Loop = false);

