# handmadequake_osx
This is (mostly) going to be a follow-along project for the [Handmade Quake series by Philip Buuck](https://www.youtube.com/playlist?list=PLBKDuv-qJpTbCsXHsxcoSSsMarnfyNhHF).

Since the Handmade Quake project seems to be Windows-centric and I have no real urge to install Windows, the main challenge will be getting everything to work on OSX.  This may or may not be possible since the preview video series (episodes 0.1 -> 0.5) tossed out all the OpenGL and hinted on relying on a proprietary win32 library.  We'll see when we get there.

The end product of the Handmade Quake series is also going to [pretty much end up being the same source code that already exists](https://www.reddit.com/r/programming/comments/3xy110/announcing_handmade_quake/cy90vnl).  I intended on modernizing the code as I go using today's best practices and C11 standards, as best I can.  It's been a long while since I've programmed in C regularly so this will make for a good (re)learning experience, too.

So that's the plan.  I may go all the way or I'll hit a snag and the project might not get very far.  No promises.

## Differences
This project is going to diverge from the main Handmade Quake project in several fundamental ways:

#### SDL 2
Graphics, sound, input, and event processing will all be taken care of by SDL 2.  While this project's goal is  to get everything running on OSX, I don't intend on writing any platform-specific code.

#### C11
Quake was written between 1994 and 1996 when C89 would have been the standard.  C's evolved  a lot since then and I'll be using the latest standard throughout the project.

#### Functions, Naming Conventions, and Structure
I intended on following along with the project's overall structure but I have my own preferred naming and formatting style (eg. `com_check_parm` instead of `COM_CheckParm`).  Global variables are a huge pet peeve of mind so some function definitions might change as well.

Functions may also change in behavior.  For example, in the main series and in the original Quake source, the `COM_CheckParm` function simply tested for the existence of an argument.  You would then need to fetch the associated argument value manually.  In my `com_check_parm` function the return value is the parameter's associated value, if one exists.  So if you passed `-width 640` to my Quake executable, passing `"-width"` to `com_check_parm` would return `"640"`.

I won't change every single function but if I see an opportunity to make a bit of code a little more convenient, I'll give it a shot.

In short, don't expect a copy/paste job.

# License
The MIT License (MIT)

Copyright (c) 2016 eggbit

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
