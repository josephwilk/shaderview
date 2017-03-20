![screenshot 2015-07-28 18 41 49](https://cloud.githubusercontent.com/assets/9792/8938008/2bacab68-355c-11e5-899a-dcd90928be12.png)

Shaderview is an app for creating visuals using OpenGl Shaders and it uses OSC messages to communicate changes in state. For example when live coding music you could send an OSC message when the drum sample played and in turn your shader could do something with that signal, like flashing of white.

```ruby
# We could do this in any language with osc support.
require 'osc-ruby'
@client ||= OSC::Client.new('localhost', 9177)

@client.send(OSC::Message.new("/shader-string", "
uniform float iKick;
void main(void){
  gl_FragColor = vec4(iKick,0.0,0.0, 1.0);
}"))

sample :drum_heavy_kick
@client.send(OSC::Message.new("/decaying-uniform", "iKick", 1.0, 0.01))
```

Since Shaderview does everything through OSC, there is no tie to a specific client language.

Shaderview has been already been used in performances with live coding languages like [Sonic Pi](http://sonic-pi.net/) and [Overtone](https://github.com/overtone/overtone).

![](https://pbs.twimg.com/media/CLVUhFjWwAE7dJP.png:small)
![](https://cloud.githubusercontent.com/assets/9792/10560237/e33c79a6-7504-11e5-9695-2e21c47ebeec.png)

### Install

OS X latest snapshot [https://github.com/josephwilk/shaderview/releases/download/v0.0.1/shaderview.tar.gz](https://github.com/josephwilk/shaderview/releases/download/v0.0.1/shaderview.tar.gz)

Download and Launch Shaderview.

### First simple Example

Shaderview launches with a default opengl shader.

```ruby
uniform float iR;
uniform float iG;
uniform float iB;
void main(void){
  gl_FragColor = vec4(iR,iG,iB, 1.0);
}
```

For more about the shader language: [https://www.opengl.org/documentation/glsl/](https://www.opengl.org/documentation/glsl/):
And for examples see: [https://www.shadertoy.com/](https://www.shadertoy.com/)

From Ruby we change the colors of the visuals. We have a number of ways we can change `uniforms`, each one tries to make it easy for the client to control their light show.

```ruby
require 'osc-ruby'
@client = OSC::Client.new('localhost', 9177)
@client.send(OSC::Message.new("/uniform" , "iR", 0.5))
@client.send(OSC::Message.new("/decaying-uniform" , "iG", 0.1))
@client.send(OSC::Message.new("/smoothed-uniform" , "iB", 0.0))
```

### Shaderview Supported Messages

Shaderview opens an Osc Server listening on port 9177.
The endpoints provided:

```
Message  <Argument> [Optional Argument]


* "/uniform" <UniformName> <FloatValue>
Set uniform to a float value


* "/smoothed-uniform" <UniformName> <FloatValue>
Move a uniform towards a float value (Fixed rate for now).
Execute multiple times to eventually get to FloatValue.


* "/decaying-uniform" <UniformName> <FloatValue> [FloatValue:decay-rate]
Like /uniform but the value will automatically decay to 0.
Decay rate defaults to 0.01 if not specified.

* "/growing-uniform" <UniformName> <FloatValue> [FloatValue:growth-rate]
Like /smoothed-uniform but the value will automatically grow from 0 to target
without having to call the endpoint again.
Growth rate defaults to 0.01 if not specified.

* "/curve-uniform" <UniformName> <FloatValue> [FloatValue:growth-rate] [FloatValue:decay-rate]
Will automatically grow to target value and then once reached will auto decay to 0.
Like growing-uniform & decaying-uniform combined.
Growth+decay rate defaults to 0.01 if not specified.
No fancy curves yet like Sin... todo

* "/shader" <StringValue:full-filename-path>
Load a file which contains an OpenGL shader. Once loaded the
file will be watched for changes and auto-reloaded


* "/shader-string" <StringValue:opengl-frag-shader>
Load a string as a OpenGL shader

* "/vertex <StringValue:full-filename-path> <StringValue:vertex-type> <IntValue:vertex-count>"
Experimental support for just working with vertex part of shaders.


* "/vertex-settings" <StringValue:type> <IntValue:count>
Supports changing parameters of the vertex phase without a pause for reloading.


* "/texture" <StringValue> <IntValue:ichannel-no>
Load an image file as a texture in your shader.
Texture is defined as iChannel1/2/3 based on IntValue.
Add `uniform sampler2D iChannel1` to your shader.
Comes packaged with some useful noise textures, just give the filename:
"tex10.png, tex11.png, tex15.png, tex16.png"
https://github.com/josephwilk/shaderview/tree/master/bin/data/textures/tex10.png
https://github.com/josephwilk/shaderview/tree/master/bin/data/textures/tex11.png
https://github.com/josephwilk/shaderview/tree/master/bin/data/textures/tex15.png
https://github.com/josephwilk/shaderview/tree/master/bin/data/textures/tex16.png

* "/volume" <FloatValue>
Sets iVolume in shader. Useful for say sending the
volume from something like Sonic Pi
```

The uniforms are updated and sent to running shader.

##### Ruby Client Example:
```ruby
require 'osc-ruby'
@client = OSC::Client.new('localhost', 9177)
@client.send(OSC::Message.new("/shader" , "/Users/josephwilk/shaders/wave.glsl"))
```

##### Clojure Client example:
```clojure
(use 'overtone.osc)
(def client (osc-client "localhost" 9177))

(osc-send client "/uniform" "iExample" (float 100.0))
(osc-send client "/shader" "/usr/josephwilk/repl_electric.glsl")
```

### Shortcut keys

```
<cmd>+e     Show error log
<cmd>+a     Show active shader code
<cmd>+f     Full Screen
<ESC>       Quit
<tab>       With editor showing, change buffer
<cmd>+s     With editor showing, save buffer
```

### OpenFramework Plugins

* https://github.com/kylemcdonald/ofxFft
* https://github.com/bakercp/ofxIO
* https://github.com/hideyukisaito/ofxOsc
* https://github.com/darrenmothersele/ofxEditor
* https://github.com/neilmendoza/ofxPostProcessing

### Dev Build instructions

##### Os X

Xcode.

##### Ubuntu (Extremely experimental)

Some notes from @mattrei on the process to get an install on Ubuntu:

* ofxEditor: would not compile because in the src/ClipBoard.cpp the ClipBoard.h is spelled wrongly. Did a pull request already there. for now we have to fix it manually.

* ofxOsc: dont use the addon which ships with oF 8.0.x; use that one which is suggested in the README.md of shaderview

* ofxPostProcessing: the current master is not working with the latest oF 8.0.4: in src/PostProcessing.h in line 52 and 53 remove the 'const' so it machte oF function signatures

* ofxFft: you need this library 'sudo apt-get install libfftw3-dev '; and then in shaderviews 'config.make' add 'USER_LDFLAGS = -lfftw3f'

### Credits

Inspired by Roger Allen's Shadertone: https://github.com/overtone/shadertone

##License

Copyright © 2015-present Joseph Wilk

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

For full license information, see the LICENSE file.
