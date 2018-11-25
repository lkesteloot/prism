
Forward ray-tracer to make an image a bit like the cover of
the Pink Floyd album _Dark Side of the Moon_.

![Program output](output.png)

# Build

Build the renderer using CMake:

    % mkdir build
    % cd build
    % cmake ..
    % make

It compiles on MacOS and Linux.

# Running

Run the binary from the `build` directory:

    % build/prism

It will either bring up a UI (if on a Mac and the `UPDATE_DISPLAY`
define is set in the code) or generate PNG file periodically. The
whole program is hacked to generate a single image. Read the comments
to figure out how to modify it.

# License

Copyright 2018 Lawrence Kesteloot

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
