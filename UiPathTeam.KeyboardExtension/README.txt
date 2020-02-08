UiPathTeam.KeyboardExtension32.dll
UiPathTeam.KeyboardExtension64.dll

This is a helper dynamic linked library for the activity package.

Both 32-bit and 64-bit DLLs need to be built for 64-bit Windows platform.

The main responsibility of this helper library is to inject the code to disable the Input Method Editor into the targeted application processes. 32-bit code can be injected into a 32-bit process only. 64-bit code can be injected into a 64-bit process only. That's the reason why both bitnesses are needed.

This helper library implements 3 main classes; Client, Server, and Agent.

Client class implements the methods to be called from the export functions to provide the functionalities of this library to the activity classes.

Server class implements the code that is hosted by RUNDLL32 to maintain Windows Hooks.

Agent class implements the code that manipulates the Input Method Editor and the keyboard layout by using Text Services Framework.

Agent class code is injected into the targeted application processes by Server class code.

Server class code injects Agent class code into the targeted application processes by using Windows Hook of WH_CALLWNDPROC.

Server class code also installs Windows Hooks of WH_KEYBOARD_LL and WH_MOUSE_LL to be capable of blocking human keyboard and mouse input.

Client, Server, and Agent classes communicate with each other by using a shared memory object.

To support communication with Agent class which is to be injected into an application running at the low integrity level such as Internet Explorer in the protected mode, Client class code temporarily impersonates the low integrity while creating the shared memory object.

That's all. Thank you for reading this.
