/* Copyright 2024 kamilsss655
 * https://github.com/kamilsss655
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include "button.h"
#include "app/uvk5.h"
#include "external/printf/printf.h"

// Button event handler
void BUTTON_Handle(BUTTON_Event_t buttonEvent)
{
    // Send SMS
    char String[30];
    snprintf(String, sizeof(String), "Button %d pressed on ESP.", buttonEvent.pin_number);
    UVK5_SendMessage(String, sizeof(String));
}