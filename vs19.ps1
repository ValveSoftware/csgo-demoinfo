# Download protobuf and protoc
cmd /c "curl -L https://github.com/google/protobuf/releases/download/v2.5.0/protobuf-2.5.0.zip --output protobuf-2.5.0.zip"
cmd /c "tar -xzf protobuf-2.5.0.zip -C demoinfogo"
cmd /c "del protobuf-2.5.0.zip"
cmd /c "curl -L https://github.com/google/protobuf/releases/download/v2.5.0/protoc-2.5.0-win32.zip --output protoc-2.5.0-win32.zip"
cmd /c "tar -xzf protoc-2.5.0-win32.zip -C demoinfogo/protoc-2.5.0-win32"
cmd /c "del protoc-2.5.0-win32.zip"

# Upgrade protobuf sln
cmd /c "devenv demoinfogo\protobuf-2.5.0\vsprojects\protobuf.sln /Upgrade"

# Include algorithm in zero_copy_stream_impl_lite.cc, min function will be unresolved otherwise
get-content demoinfogo\protobuf-2.5.0\src\google\protobuf\io\zero_copy_stream_impl_lite.cc | %{$_ -replace "#include <google/protobuf/stubs/stl_util.h>", "#include <google/protobuf/stubs/stl_util.h>`n#include <algorithm>"} | out-file temp
cmd /c "move /y temp demoinfogo\protobuf-2.5.0\src\google\protobuf\io\zero_copy_stream_impl_lite.cc"

# Define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS for hash_map deprecation
$projects = "demoinfogo\protobuf-2.5.0\vsprojects\libprotobuf.vcxproj", "demoinfogo\protobuf-2.5.0\vsprojects\libprotobuf-lite.vcxproj", "demoinfogo\protobuf-2.5.0\vsprojects\libprotoc.vcxproj"
foreach ($project in $projects) {
    get-content $project | %{$_ -replace "<PreprocessorDefinitions>", "<PreprocessorDefinitions>_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS;"} | out-file temp
    cmd /c "move /y temp $project"
}

# Build libprotobuf
cmd /c "devenv demoinfogo\protobuf-2.5.0\vsprojects\protobuf.sln /Build `"Release|Win32`" /Project libprotobuf"

# Upgrade demoinfogo sln
cmd /c "devenv demoinfogo\demoinfogo.sln /Upgrade"

# Build demoinfogo
cmd /c "devenv demoinfogo\demoinfogo.sln /Build `"Release|Win32`" /Project demoinfogo"