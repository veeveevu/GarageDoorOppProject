#include "GarageDoorController.h"

/*
**The full journey of one button press, step by step:**

1. User presses SW1 button physically

2. GarageController::checkButtons() runs
   → gpio reads LOW on SW1 pin
   → "oh something happened, I'll label it ButtonPressed"
   → calls m_stateMachine.handleEvent(Event::ButtonPressed)

3. DoorStateMachine::handleEvent() receives it
   → checks current state: Closed
   → checks event: ButtonPressed
   → finds the matching row in the table
   → changes m_state to Opening

4. GarageController::reactToState() runs
   → reads state: Opening
   → calls m_motor.step(Direction::Clockwise)

   */