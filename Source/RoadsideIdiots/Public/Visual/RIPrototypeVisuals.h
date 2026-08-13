#pragma once

class ARIBikePawn;

namespace RIPrototypeVisuals
{
    void Setup(ARIBikePawn* Bike);
    void Update(ARIBikePawn* Bike);
    void PlaySideAction(ARIBikePawn* Bike, float Side);
    void PlayReaction(ARIBikePawn* Bike, float Side);
}
