/*==========================================================================
UIï\é¶[UI.h]
	


													Author : hidetoshi muramatu
													Date   : 2026/01/16
---------------------------------------------------------------------------



==========================================================================*/

#pragma once
#include <DirectXMath.h>




namespace UI
{
    enum class NotificationType
    {
        NONE,
        STRIKE,
        SPARE,
        GUTTER,
        MAX
    };

    void Initialize();
    void Update(double dt);
    void Draw();
    void Finalize();

    // í ímÇï\é¶Ç∑ÇÈä÷êî
    void ShowNotification(NotificationType type);
}