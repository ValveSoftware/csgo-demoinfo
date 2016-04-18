CS:GO Demos and Network Messages
--------------------------------

Demos and network messages in CS:GO use Google's Protocol Buffers (protobuf). Protobuf is a message/object serialization language that generates code to serialize the objects efficiently. For information about protobuf, see https://developers.google.com/protocol-buffers/docs/overview

demoinfogo.exe is a tool that parses CS:GO demo files (ending in .dem) and dumps out every message in the demo. Using this tool, third parties can parse the demo for various game events to generate information and statistics.

The zip file DemoinfoGO contains demoinfogo.exe and the source code required to build this tool. The source includes the proto files containing network message definitions.


Building demoinfogo
------------------

In order to build demoinfogo on Windows, follow these steps

1. Download and extract demoinfogo.zip (XXX). This creates a folder called demoinfogo.
2. Download protobuf-2.5.0.zip (http://code.google.com/p/protobuf/downloads/detail?name=protobuf-2.5.0.zip&can=2&q=) and extract it into the demoinfogo folder. This creates the folder demoinfogp/protobuf-2.5.0.
3. Download the protocol buffer compiler protoc-2.5.0.zip (http://code.google.com/p/protobuf/downloads/detail?name=protoc-2.5.0-win32.zip&can=2&q=) and extract it into the demoinfogo/protoc-2.5.0-win32 folder that was created when you extracted demoinfogo.zip.
4. Open demoinfogo/protobuf-2.5.0/vsprojects/protobuf.sln in Microsoft Visual Studio 2010. Allow Visual Studio to convert projects. Once loaded, build the Release configuration of libprotobuf. Building any other projects is not required.
5. Open the project demoinfogo/demoinfogo.vcxproj in Microsoft Visual Studio 2010. Building the Release configuration creates the binary demoinfogo/demoinfogo.exe


Working with Network Messages
-----------------------------

Building demoinfogo generates C++ classes from the network protobuf files. Follow these steps to generate these files for use in your application without building demoinfogo
1. Follow the instructions above for extracting protobuf 2.5.0 and protoc 2.5.0
2. Run make_cc_files.bat. The generated C++ files are created in subdirectory named generated_proto.


You can now use the generated classes in your C++ code to send user messages. Here is an example of how to send the HintText message

CSingleUserRecipientFilter filter( this );
filter.MakeReliable();
CCSUsrMsg_HintText msg;
msg.set_text( "ExampleHint" );
SendUserMessage( filter, CS_UM_HintText, msg );


====== Copyright (c) 2014, Valve Corporation, All rights reserved. ========

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, 
 this list of conditions and the following disclaimer in the documentation 
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
===========================================================================

