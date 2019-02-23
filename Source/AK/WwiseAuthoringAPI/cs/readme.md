
# C# Samples
## Overview

The WaapiCS project is a Visual Studio 2017 project targeting .NET 4.5, which provides samples for the Wwise Authoring API using WampSharp.

## Requirements

Visual Studio 2017 with C# support and the NuGet package manager extension is required.

As of version 1.2.5.36-beta of WampSharp, you will need to add "ws://127.0.0.1:8080" as an allowed origin if you are running the sample on the same computer as the Wwise Authoring application. To do so, go to Project > User Preferences... and in the Wwise Authoring API section at the bottom of the window, add the address in the Allowed origins text box. Note that values are comma-separated.

## Setup
1. Go to the directory WaapiCS
2. Run `nuget restore` from a command line or open WaapiCS.sln, right-click on the solution and select "Restore NuGet Packages".
This will download the necessary dependencies such as WampSharp.
3. Build the project and observe that there are no errors in the main projects.

## Execution

Two samples can be run. To select the one to run, right-click on the WaapiCS project, select Properties, and in the Application tab select the appropriate sample under Startup object.

### WwiseCall

With Wwise running and a project opened, execute the solution and observe in the output:

	Calling 'ak.wwise.core.getInfo'
	Calling 'ak.wwise.ui.getSelectedObjects'
	ak.wwise.core.getInfo: Hello Wwise v2017.1.0
	ak.wwise.ui.getSelectedObjects: Got selected object data!
	ak.wwise.ui.getSelectedObjects: Got 1 object(s)!
	ak.wwise.ui.getSelectedObjects: The first selected object is 'New Sequence Container' ({03E8DC21-EB2F-4607-BFCC-25BCE69DFB27})
	ak.wwise.ui.getSelectedObjects: Its parent is 'ParentA' ({FBB64B3C-711C-46E6-9BBC-B49511F08244})

The received data from getSelectedObjects will vary based on your opened project. If no selection exists, the following message will be displayed:

	ak.wwise.ui.getSelectedObjects: Select something and try again!

### PubSubWwise

With Wwise running and a project opened, execute the solution and observe in the output:

	Add a child to an entity in the Wwise Authoring application.

Select the Default Work Unit under the Actor-Mixer Hierarchy in the Audio tab of the Project Explorer and add a container. You should see in the output:

	A child was added, ID={EAFD1799-41AA-4A98-B0CA-BDB8CDA0D878}
	Press any key to continue...
